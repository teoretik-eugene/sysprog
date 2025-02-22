#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 100 // Максимальная длина строк для сайта, логина и пароля

typedef struct Node {
    char site[MAX_LENGTH]; // Сайт
    char login[MAX_LENGTH]; // Логин
    char password[MAX_LENGTH]; // Пароль
    struct Node* next; // Указатель на следующий узел
} Node;

// Создание нового узла
Node* createNode(const char* site, const char* login, const char* password) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Error mem!\n");
        exit(1);
    }
    strncpy(newNode->site, site, MAX_LENGTH);
    strncpy(newNode->login, login, MAX_LENGTH);
    strncpy(newNode->password, password, MAX_LENGTH);
    newNode->next = NULL;
    return newNode;
}

// Добавление записи в стек
void push(Node** top, const char* site, const char* login, const char* password) {
    Node* newNode = createNode(site, login, password);
    newNode->next = *top; // Установить следующий элемент как текущий top
    *top = newNode; // Новый узел становится вершиной стека
    printf("Add node: site = %s, login = %s, pass = %s\n", site, login, password);
}

// Удаление верхней записи из стека (pop)
void pop(Node** top) {
    if (*top == NULL) {
        printf("Stack is empty! No pop.\n");
        return;
    }
    Node* temp = *top; // Сохраняем текущую вершину
    printf("Delete node: site = %s, login = %s, pass = %s\n", temp->site, temp->login, temp->password);
    *top = (*top)->next; // Смещаем вершину вниз
    free(temp); // Освобождаем память
}

// Просмотр верхней записи стека (peek)
void peek(Node* top) {
    if (top == NULL) {
        printf("Stack is empty!\n");
        return;
    }
    printf("Top node: site = %s, login = %s, pass = %s\n", top->site, top->login, top->password);
}

// Просмотр всех записей в стеке
void printStack(Node* top) {
    if (top == NULL) {
        printf("Stack is empty!\n");
        return;
    }
    printf("All nodes:\n");
    Node* current = top;
    while (current != NULL) {
        printf("site: %s, login: %s, pass: %s\n", current->site, current->login, current->password);
        current = current->next;
    }
}

// Поиск записи по сайту
void searchNode(Node* top, const char* searchSite) {
    if (top == NULL) {
        printf("Stack is empty!\n");
        return;
    }
    int found = 0;
    Node* current = top;
    while (current != NULL) {
        if (strncmp(current->site, searchSite, MAX_LENGTH) == 0) {
            // Сайт найден
            printf("Found site: %s, login: %s, pass: %s\n", current->site, current->login, current->password);
            found = 1;
            break;
        }
        current = current->next;
    }
    if (!found) {
        printf("Site %s not found in stack.\n", searchSite);
    }
}

// Меню
void menu() {
    printf("\nCmds:\n");
    printf("push <site> <login> <pass> - Add node\n");
    printf("pop - delete top node\n");
    printf("peek - Check top node\n");
    printf("print - Show all nodes\n");
    printf("search <site> - Search node by site\n");
    printf("exit - Exit program\n");
}

int main() {
    Node* stack = NULL; // Инициализация пустого стека
    char command[MAX_LENGTH];
    char site[MAX_LENGTH], login[MAX_LENGTH], password[MAX_LENGTH];

    printf("Welcome to password manager CLI!\n");

    while (1) {
        menu();
        printf("Enter command: ");
        command[0] = '\0';
        fgets(command, MAX_LENGTH, stdin);
        command[strcspn(command, "\n")] = '\0'; // Удаляем символ новой строки

        if (strncmp(command, "push ", 5) == 0) {
            if (sscanf(command + 5, "%s %s %s", site, login, password) == 3) {
                push(&stack, site, login, password);
            } else {
                printf("Error: Incorrect arguments for push command.\n");
            }
        } else if (strcmp(command, "print") == 0) {
            printStack(stack);
        } else if (strcmp(command, "pop") == 0) {
            pop(&stack);
        } else if (strcmp(command, "peek") == 0) {
            peek(stack);
        } else if (strncmp(command, "search ", 7) == 0) {
            if (sscanf(command + 7, "%s", site) == 1) {
                searchNode(stack, site);
            } else {
                printf("Please enter a site to search.\n");
            }
        } else if (strcmp(command, "exit") == 0) {
            printf("Exiting program.\n");
            break;
        } else {
            printf("Error: Unknown command.\n");
        }
    }

    // Освобождение памяти
    while (stack) {
        pop(&stack);
    }

    return 0;
}
