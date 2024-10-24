#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <netinet/in.h>

#define MAX 1024
#define BACKLOG 5

typedef struct node {
    char username[MAX];
    char password[MAX];
    int status; // 1: active, 0: blocked
    struct node* next;
} node_t;

node_t* load_data(char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Cannot open file");
        exit(1);
    }
    node_t* head = NULL;
    node_t* current = NULL;
    char username[MAX], password[MAX];
    int status;

    while (fscanf(f, "%s %s %d", username, password, &status) != EOF) {
        node_t* new_node = (node_t*)malloc(sizeof(node_t));
        strcpy(new_node->username, username);
        strcpy(new_node->password, password);
        new_node->status = status;
        new_node->next = NULL;

        if (!head) head = new_node;
        else current->next = new_node;
        current = new_node;
    }
    fclose(f);
    return head;
}

void save_data(node_t* head, char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("Cannot open file");
        exit(1);
    }
    node_t* current = head;
    while (current) {
        fprintf(f, "%s %s %d\n", current->username, current->password, current->status);
        current = current->next;
    }
    fclose(f);
}

node_t* find_node(node_t* head, char* username) {
    while (head) {
        if (strcmp(head->username, username) == 0)
            return head;
        head = head->next;
    }
    return NULL;
}

void log_client_info(struct sockaddr_in client, char* client_info) {
    char filename[MAX];
    sprintf(filename, "client_%s_%d_log.txt", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    FILE* f = fopen(filename, "a");
    if (!f) {
        perror("Error opening log file");
        return;
    }
    fprintf(f, "Client IP: %s, Port: %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
    fprintf(f, "Client Info: %s\n", client_info);
    fclose(f);
}

void save_csv_file(int conn_sock, char* filename) {
    FILE* f = fopen(filename, "w");
    char buffer[MAX];
    int bytes_received;

    while ((bytes_received = recv(conn_sock, buffer, MAX, 0)) > 0) {
        fwrite(buffer, sizeof(char), bytes_received, f);
    }
    fclose(f);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./server <PortNumber>\n");
        exit(1);
    }

    int listen_sock, conn_sock;
    struct sockaddr_in server, client;
    socklen_t client_size = sizeof(client);
    char username[MAX], password[MAX], client_info[MAX];
    int bytes_received;
    char* filename = "account.txt";
    node_t* accounts = load_data(filename);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = INADDR_ANY;
    bind(listen_sock, (struct sockaddr*)&server, sizeof(server));
    listen(listen_sock, BACKLOG);

    printf("Server is running on port %s...\n", argv[1]);

    while (1) {
        conn_sock = accept(listen_sock, (struct sockaddr*)&client, &client_size);
        printf("Connection from %s\n", inet_ntoa(client.sin_addr));

        if (fork() == 0) { // Child process
            close(listen_sock);

            // Step 1: Authentication
            bytes_received = recv(conn_sock, username, MAX, 0);
            username[bytes_received] = '\0';
            node_t* user = find_node(accounts, username);
            char reply[MAX];
            int attempts = 0;

            if (user && user->status == 1) {
                strcpy(reply, "1"); // User found and active
            } else {
                strcpy(reply, user ? "2" : "0"); // Blocked or not found
            }
            send(conn_sock, reply, strlen(reply), 0);

            if (strcmp(reply, "1") != 0) {
                close(conn_sock);
                exit(0);
            }

            while (attempts < 3) {
                bytes_received = recv(conn_sock, password, MAX, 0);
                password[bytes_received] = '\0';

                if (strcmp(user->password, password) == 0) {
                    strcpy(reply, "1");
                    send(conn_sock, reply, strlen(reply), 0);
                    break;
                } else {
                    attempts++;
                    strcpy(reply, (attempts == 3) ? "2" : "0");
                    send(conn_sock, reply, strlen(reply), 0);
                    if (attempts == 3) {
                        user->status = 0; // Block user after 3 attempts
                        save_data(accounts, filename);
                    }
                }
            }

            // Step 2: Process client mode
            bytes_received = recv(conn_sock, reply, MAX, 0);
            if (strcmp(reply, "1") == 0) { // Mode 1: Receive client info
                bytes_received = recv(conn_sock, client_info, MAX, 0);
                client_info[bytes_received] = '\0';
                printf("Client Info: %s\n", client_info);
                log_client_info(client, client_info);
            } else if (strcmp(reply, "2") == 0) { // Mode 2: Receive file
                bytes_received = recv(conn_sock, reply, MAX, 0); // Receive file name
                reply[bytes_received] = '\0';
                printf("Receiving file: %s\n", reply);
                save_csv_file(conn_sock, reply);
            }

            close(conn_sock);
            exit(0);
        } else {
            close(conn_sock);
        }
    }

    close(listen_sock);
    return 0;
}
