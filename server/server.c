// server.c (file transfer support)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define PORT 8080

typedef struct {
    struct sockaddr_in address;
    int sock;
    int id;
    char username[32];
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_message(char *message, int sender_id) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] && clients[i]->id != sender_id) {
            write(clients[i]->sock, message, strlen(message));
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    char buffer[BUFFER_SIZE];
    client_t *cli = (client_t *)arg;
    FILE *file = NULL;

    while (1) {
        int bytes_received = recv(cli->sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("%s disconnected.\n", cli->username);
            close(cli->sock);
            pthread_mutex_lock(&clients_mutex);
            clients[cli->id] = NULL;
            pthread_mutex_unlock(&clients_mutex);
            break;
        }

        buffer[bytes_received] = '\0';

        // Handle file transfer start
        if (strncmp(buffer, "/file ", 6) == 0) {
            char *filename = buffer + 6;
            file = fopen(filename, "wb");
            if (file == NULL) {
                printf("Error opening file for writing.\n");
                continue;
            }
            printf("%s started sending file: %s\n", cli->username, filename);
            continue;
        }

        // Handle file transfer completion
        if (strcmp(buffer, "/filedone") == 0) {
            if (file) {
                fclose(file);
                file = NULL;
                printf("File transfer complete.\n");
            }
            continue;
        }

        // If file transfer is ongoing, write to file
        if (file) {
            fwrite(buffer, 1, bytes_received, file);
        } else {
            // Normal message broadcasting
            send_message(buffer, cli->id);
        }
    }

    free(cli);
    return NULL;
}

int main() {
    int server_sock, new_sock;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;

    // Create server socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    // Start listening for incoming connections
    listen(server_sock, 10);
    printf("Server started on port %d\n", PORT);

    while (1) {
        socklen_t client_len = sizeof(client_addr);
        new_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_len);

        // Add new client
        client_t *cli = (client_t *)malloc(sizeof(client_t));
        cli->sock = new_sock;
        cli->id = new_sock; // Assign socket ID as unique identifier for simplicity
        recv(cli->sock, cli->username, 32, 0); // Receive username from client
        printf("%s connected.\n", cli->username);

        pthread_mutex_lock(&clients_mutex);
        clients[new_sock] = cli;
        pthread_mutex_unlock(&clients_mutex);

        // Create a thread to handle the client
        pthread_create(&tid, NULL, &handle_client, (void*)cli);
    }

    close(server_sock);
    return 0;
}
