#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8081
#define BUFFER_SIZE 1024
#define CONFIG_FILE "config"

char* reverse_string(char *str);
int read_socket_type_config();
char read_key_config();
char* encrypt(char *input);

int main() {
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int server_fd, new_socket;
    char buffer[BUFFER_SIZE] = {0};

    // Чтение типа сокета из конфигурационного файла
    int type = read_socket_type_config();
    if (type == -1) {
        perror("config socket error");
        exit(EXIT_FAILURE);
    }

    if (type == SOCK_STREAM) { // TCP
        printf("Using TCP\n");

        // Создание сокета
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("SOCKET ERROR");
            exit(EXIT_FAILURE);
        }

        // Настройка адреса сервера
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY; // Принимать соединения на все интерфейсы
        address.sin_port = htons(PORT);

        // Привязка сокета к адресу
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Ожидание подключений
        if (listen(server_fd, 3) < 0) {
            perror("listen failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        printf("Server is listening on port %d...\n", PORT);

        while (1) {
            // Принятие подключения
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                continue;
            }

            // Чтение данных
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                perror("read failed");
                close(new_socket);
                continue;
            }

            printf("Input message: %s\n", buffer);

            // Удаление символа новой строки, если он есть
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }

            // Декодируем строку
            char* decrypted = encrypt(buffer);

            // Разворачиваем строку
            char *reversed = reverse_string(decrypted);

            printf("Reversed message: %s\n", reversed);

            // Кодируем обратно
            char *encrypted_response = encrypt(reversed);

            // Отправка ответа
            send(new_socket, encrypted_response, strlen(encrypted_response), 0);

            // Освобождение памяти
            free(encrypted_response);
            free(reversed);
            free(decrypted);

            memset(buffer, 0, BUFFER_SIZE);

            // Закрытие сокета клиента
            close(new_socket);

            printf("Client disconnected.\n");
        }

        // Закрытие серверного сокета
        close(server_fd);

    } else if (type == SOCK_DGRAM) { // UDP
        printf("Using UDP\n");

        // Создание сокета
        if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("SOCKET ERROR");
            exit(EXIT_FAILURE);
        }

        // Настройка адреса сервера
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY; // Принимать соединения на все интерфейсы
        address.sin_port = htons(PORT);

        // Привязка сокета к адресу
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        printf("Server is listening on port %d...\n", PORT);

        while (1) {
            struct sockaddr_in client_addr;
            socklen_t client_addrlen = sizeof(client_addr);

            // Получение данных
            int valread = recvfrom(server_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addrlen);
            if (valread <= 0) {
                perror("recvfrom failed");
                continue;
            }

            printf("Received message: %s\n", buffer);

            // Удаление символа новой строки, если он есть
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }

            // Декодируем строку
            char* decrypted = encrypt(buffer);

            // Разворачиваем строку
            char *reversed = reverse_string(decrypted);

            printf("Reversed message: %s\n", reversed);

            // Кодируем обратно
            char *encrypted_response = encrypt(reversed);

            // Отправка ответа
            sendto(server_fd, encrypted_response, strlen(encrypted_response), 0, (struct sockaddr *)&client_addr, client_addrlen);

            // Освобождение памяти
            free(encrypted_response);
            free(reversed);
            free(decrypted);

            memset(buffer, 0, BUFFER_SIZE);
        }

        // Закрытие серверного сокета
        close(server_fd);
    }

    return 0;
}

char* reverse_string(char *str) {
    int len = strlen(str);
    char *reversed = (char*)malloc(len + 1);
    if (reversed == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < len; i++) {
        reversed[i] = str[len - (i + 1)];
    }
    reversed[len] = '\0';
    return reversed;
}

int read_socket_type_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "SOCKET_TYPE=%s", line) == 1) {
            fclose(file);
            if (strcmp(line, "TCP") == 0) {
                return SOCK_STREAM;
            } else if (strcmp(line, "UDP") == 0) {
                return SOCK_DGRAM;
            }
        }
    }

    fprintf(stderr, "SOCKET_TYPE not found in config file\n");
    fclose(file);
    exit(EXIT_FAILURE);
}

char read_key_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char line[100];
    char key[10] = {0};

    // Чтение строки из файла
    if (fgets(line, sizeof(line), file) == NULL) {
        fprintf(stderr, "Config file is empty or cannot be read\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    // Парсинг строки для извлечения ключа
    if (sscanf(line, "XOR_KEY=%s", key) != 1) {
        fprintf(stderr, "Invalid config file format. Expected 'XOR_KEY=value'\n");
        exit(EXIT_FAILURE);
    }

    // Преобразование строки в число (например, "0xAA" -> 0xAA)
    printf("%s\n", key);
    return (char)strtol(key, NULL, 0);
}

char* encrypt(char *input) {
    char key = read_key_config();
    printf("Using XOR key: 0x%02X\n", (unsigned char)key);

    size_t len = strlen(input);
    char *output = (char*)malloc(len + 1); // +1 для нуль-терминатора
    if (output == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < len; i++) {
        output[i] = input[i] ^ key; // Применяем XOR к каждому символу
    }
    output[len] = '\0'; // Добавляем нуль-терминатор

    return output;
}
