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

char read_key_config();
char* encrypt(char *input);

int main() {

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char* hello = "Hello from client";
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation ERROR\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Преобразование IP-адреса из текстового в бинарный формат
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Подключение к серверу
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    // чтение данных из консоли
    printf("Enter the input\n");
    fgets(buffer, BUFFER_SIZE, stdin);

    // шифрование данных
    char *encrypted = encrypt(buffer);
    printf("Encrypted: %s\n", encrypted);
    
    send(sock, encrypted, strlen(encrypted), 0);
    printf("Message sent: %s\n", buffer);

    // Очистка буфера перед чтением ответа
    memset(buffer, 0, BUFFER_SIZE);
    free(encrypted);
    // Чтение ответа от сервера
    int valread = recv(sock, buffer, BUFFER_SIZE, 0);
    char *decrypted_response = encrypt(buffer);
    printf("Server response: %s\n", decrypted_response);

    // Закрытие сокета
    close(sock);
    free(decrypted_response);

    return 0;
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
