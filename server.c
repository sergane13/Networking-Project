#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

#define GET_MENU "GET_MENU"
#define CHANGE_MENU "CHANGE_MENU"
#define AUTH "AUTH admin1234"

bool isManagerLogIn = false;
int socketManager = 0;

char menu[5][100] = {
    "Pizza Vegana, Pizza Pui, Pizza Peperoni", 
    "Paste Carbonara, Paste cu Pesto, Paste alioli", 
    "Burger Vita, Burger Pui, Burger Crevete",
    "Sushi", 
    "Apa, Ceai, Cola, Pepsi"
};

void getMenu(int menuNumber, int client_socket) {
    if (menuNumber >= 1 && menuNumber <= 5) {
        char response[100];
        snprintf(response, sizeof(response), "Menu %d: %s\n", menuNumber, menu[menuNumber - 1]);
        send(client_socket, response, strlen(response), 0);

        fflush(stdout);
        printf("%d - GET_MENU\n", client_socket);
        
        return;
    } 

    char response[] = "Invalid menu number.\n";
    send(client_socket, response, strlen(response), 0);
    
}

void changeMenu(int menuNumber, const char *newMenu, int client_socket) {
    if (menuNumber < 1 || menuNumber > 5) {
        const char *response = "Invalid menu number.\n";
        send(client_socket, response, strlen(response), 0);
        
        return;
    }

    strncpy(menu[menuNumber - 1], newMenu, sizeof(menu[menuNumber - 1]) - 1);
    menu[menuNumber - 1][sizeof(menu[menuNumber - 1]) - 1] = '\0';
    const char *response = "Menu updated successfully.\n";
    send(client_socket, response, strlen(response), 0);

    fflush(stdout);
    printf("%d - CHANGE_MENU\n", client_socket);

}

void handleCommands(char *msg, int client_socket) {
    char command[50];
    int menuNumber;
    char newMenu[100];

    fflush(stdout);
    msg[strcspn(msg, "\r\n")] = '\0'; 

    if (strcmp(msg, AUTH) == 0) {
        const char *response = "Logged as manager.\n";
        send(client_socket, response, strlen(response), 0);
        isManagerLogIn = true;
        socketManager = client_socket;
        return;
    }

    if (sscanf(msg, "%s %d %[^\n]", command, &menuNumber, newMenu) == 3) {
        if (!isManagerLogIn) {
            const char *response = "Please log as manager to change menu.\n";
            send(client_socket, response, strlen(response), 0);
            return;
        }

        if (strcmp(command, CHANGE_MENU) == 0) {
            changeMenu(menuNumber, newMenu, client_socket);
        } else {
            const char *response = "1 - Command does not exist.\n";
            send(client_socket, response, strlen(response), 0);
        }

        return;
    }
    
    if (sscanf(msg, "%s %d", command, &menuNumber) == 2) {
        if (strcmp(command, GET_MENU) == 0) {
            getMenu(menuNumber, client_socket);
        } else if (strcmp(command, CHANGE_MENU) == 0) {
            const char *response = "Invalid command format for CHANGE_MENU.\n";
            send(client_socket, response, strlen(response), 0);
        } else {
            const char *response = "2 - Command does not exist.\n";
            send(client_socket, response, strlen(response), 0);
        }

        return;
    } 

    const char *response = "Invalid command format.\n";
    send(client_socket, response, strlen(response), 0);
}

void *handleClientRequest(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);

    while (1) {
        char msg[100];
        bzero(msg, sizeof(msg));
        fflush(stdout);

        int read_size = read(client_socket, msg, sizeof(msg));

        if (read_size == 0) {
            printf("Client disconnected.\n");
            if (client_socket == socketManager) {
                isManagerLogIn = false;
            }
            close(client_socket);
            pthread_exit(NULL);
            break;
        }

        if (read_size == -1) {
            perror("Error reading from client.\n");
            close(client_socket);
            pthread_exit(NULL);
        }

        handleCommands(msg, client_socket);
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char msg[100];

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }   
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size))) {
        printf("Client connected.\n");

        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handleClientRequest, new_sock) != 0) {
            perror("Thread creation failed");
            free(new_sock);
        }

        pthread_detach(client_thread);
    }

    if (client_socket < 0) {
        perror("Accept failed");
    }

    close(server_socket);

    return 0;
}
