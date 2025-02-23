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
char read_key_config();
char* encrypt(char *input);

int main () {

    char server_message[256] = "You have reached the server!";
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int server_fd, new_socket;
    char buffer[BUFFER_SIZE] = {0};

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
        perror("listen failed\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);
    while (1) {
        // Принятие подключения
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // Чтение данных
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        printf("Input message: %s\n", buffer);
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        // Декодируем строку
        char* decrypted = encrypt(buffer);
        // Разворачиваем
        char *reversed = reverse_string(decrypted);
        printf("response: %s\n", reversed);
        // Кодируем обратно
        char *encrypted_response = encrypt(reversed);
        send(new_socket, encrypted_response, strlen(encrypted_response), 0);

        // Освобождение памяти и очистка буфера
        free(encrypted_response);
        free(reversed);
        free(decrypted);
        memset(buffer, 0, BUFFER_SIZE);
    }

    // Закрытие сокетов
    close(new_socket);
    close(server_fd);

    return 0;
}

char* reverse_string(char *str) {
    int len = strlen(str);
    char *reversed = (char*)malloc(strlen(str) + 1);
    if (reversed == NULL) {
        perror("mem err");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < len; i++)
        reversed[i] = str[len - (i + 1)];
    reversed[len] = '\0';
    return reversed;
}

char read_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char key[10];
    if (fscanf(file, "XOR_KEY=%s", key) != 1) {
        printf(stderr, "Invalid config file format\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    return (char)strtol(key, NULL, 0);
}

char read_key_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
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
    //fprintf("key: %s\n", key);

    // Преобразование строки в число (например, "0xAA" -> 0xAA)
    return (char)strtol(key, NULL, 0);
}


// Функция для шифрования строки
char* encrypt(char *input) {
    char key = read_key_config();
    printf("Using XOR key: \n", (unsigned char)key);

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
