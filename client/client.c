// client.c (file transfer support)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int sock;
char username[32];

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("%s\n", buffer);
        }
    }
    return NULL;
}

void send_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("File not found!\n");
        return;
    }

    // Inform the server about the file transfer
    char message[BUFFER_SIZE];
    snprintf(message, sizeof(message), "/file %s", filename);
    send(sock, message, strlen(message), 0);

    // Start sending the file in chunks
    char file_buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
        send(sock, file_buffer, bytes_read, 0);
    }

    fclose(file);
    printf("File %s sent!\n", filename);

    // Notify the server that file transfer is done
    strcpy(message, "/filedone");
    send(sock, message, strlen(message), 0);
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t tid;
    char buffer[BUFFER_SIZE];

    // Create client socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        printf("Connection failed.\n");
        return -1;
    }

    printf("Enter your username: ");
    fgets(username, 32, stdin);
    username[strcspn(username, "\n")] = '\0';
    send(sock, username, 32, 0);

    // Create thread to receive messages
    pthread_create(&tid, NULL, &receive_messages, NULL);

    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strncmp(buffer, "/sendfile ", 10) == 0) {
            send_file(buffer + 10);
        } else {
            send(sock, buffer, strlen(buffer), 0);
        }
    }

    close(sock);
    return 0;
}
