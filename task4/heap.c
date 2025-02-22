#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int value;  // Значение элемента (код символа)
} Element;

void swap(Element *a, Element *b) {
    Element temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(Element arr[], int n, int i, int asc) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (asc) {
        if (left < n && arr[left].value > arr[largest].value) {
            largest = left;
        }
        if (right < n && arr[right].value > arr[largest].value) {
            largest = right;
        }
    } else {
        if (left < n && arr[left].value < arr[largest].value) {
            largest = left;
        }
        if (right < n && arr[right].value < arr[largest].value) {
            largest = right;
        }
    }

    if (largest != i) {
        swap(&arr[i], &arr[largest]);
        heapify(arr, n, largest, asc);
    }
}

void heapSort(Element arr[], int n, int asc) {
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, asc);
    }
    for (int i = n - 1; i > 0; i--) {
        swap(&arr[0], &arr[i]);
        heapify(arr, i, 0, asc);
    }
}

int parseInput(const char *input, Element **arr) {
    Element *tempArr = malloc(1024 * sizeof(Element));
    int size = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '\n') {
            continue; // Пропускаем символ новой строки
        }
        tempArr[size].value = (int)input[i];
        size++;
    }
    *arr = realloc(tempArr, size * sizeof(Element));
    return size;
}

int readFile(const char *filename, Element **arr) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Ошибка открытия файла");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(fileSize + 1);
    fread(buffer, 1, fileSize, file);
    fclose(file);
    buffer[fileSize] = '\0';

    // Удаляем последний символ новой строки, если он есть
    if (fileSize > 0 && buffer[fileSize - 1] == '\n') {
        buffer[fileSize - 1] = '\0';
    }

    int size = parseInput(buffer, arr);
    free(buffer);
    return size;
}

void printArray(Element arr[], int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i].value >= 0 && arr[i].value <= 127 && isprint(arr[i].value)) {
            printf("%c", (char)arr[i].value);
        } else {
            printf("\\x%02X", arr[i].value);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    Element *arr = NULL;
    int size = 0;
    int asc = 1;

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--max") == 0) {
                asc = 0; // Максимальные значения в начале
            } else if (strcmp(argv[i], "--min") == 0) {
                asc = 1; // Минимальные значения в начале
            } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
                const char *filename = argv[i + 1];
                size = readFile(filename, &arr);
                if (size == -1) {
                    return 1;
                }
                i++;
            }
        }
    }

    if (size == 0) {
        char input[1024];
        printf("Введите данные (символы): ");
        fgets(input, sizeof(input), stdin);
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        size = parseInput(input, &arr);
    }

    printf("Исходные данные:\n");
    printArray(arr, size);

    heapSort(arr, size, asc);

    printf("Отсортированные данные:\n");
    printArray(arr, size);

    free(arr);
    return 0;
}