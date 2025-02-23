#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024
#define CONFIG_FILE "config"

char read_key_config();
int read_socket_type_config();
char* encrypt(char *input);

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    //int type = read_socket_type_config();
    int type = read_socket_type_config();

    if (type == SOCK_STREAM) { // TCP
        printf("Using TCP\n");

        // Создание сокета
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\nSocket creation ERROR\n");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Преобразование IP-адреса из текстового в бинарный формат
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported\n");
            return -1;
        }

        // Подключение к серверу
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\nConnection Failed\n");
            return -1;
        }

        // Чтение данных из консоли
        printf("Enter the input:\n");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Удаление символа новой строки, если он есть
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Шифрование данных
        char *encrypted = encrypt(buffer);
        printf("Encrypted: %s\n", encrypted);

        // Отправка данных на сервер
        send(sock, encrypted, strlen(encrypted), 0);
        printf("Message sent: %s\n", buffer);

        // Очистка буфера перед чтением ответа
        memset(buffer, 0, BUFFER_SIZE);
        free(encrypted);

        // Чтение ответа от сервера
        int valread = recv(sock, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) {
            perror("recv failed");
            close(sock);
            return -1;
        }

        // Дешифрование ответа
        char *decrypted_response = encrypt(buffer);
        printf("Server response: %s\n", decrypted_response);

        // Освобождение памяти
        free(decrypted_response);

        // Закрытие сокета
        close(sock);

    } else if (type == SOCK_DGRAM) { // UDP
        printf("Using UDP\n");

        // Создание сокета
        if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("\nSocket creation ERROR\n");
            return -1;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        // Преобразование IP-адреса из текстового в бинарный формат
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported\n");
            return -1;
        }

        // Чтение данных из консоли
        printf("Enter the input:\n");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Удаление символа новой строки, если он есть
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Шифрование данных
        char *encrypted = encrypt(buffer);
        printf("Encrypted: %s\n", encrypted);

        // Отправка данных на сервер
        sendto(sock, encrypted, strlen(encrypted), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        printf("Message sent: %s\n", buffer);

        // Очистка буфера перед чтением ответа
        memset(buffer, 0, BUFFER_SIZE);
        free(encrypted);

        // Чтение ответа от сервера
        socklen_t addr_len = sizeof(serv_addr);
        int valread = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&serv_addr, &addr_len);
        if (valread <= 0) {
            perror("recvfrom failed");
            close(sock);
            return -1;
        }

        // Дешифрование ответа
        char *decrypted_response = encrypt(buffer);
        printf("Server response: %s\n", decrypted_response);

        // Освобождение памяти
        free(decrypted_response);

        // Закрытие сокета
        close(sock);

    } else {
        fprintf(stderr, "Invalid socket type in config file\n");
        return -1;
    }

    return 0;
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
    printf("$s\n", key);
    return (char)strtol(key, NULL, 0);
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

// Функция для шифрования строки
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
