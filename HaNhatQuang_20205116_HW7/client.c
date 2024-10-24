#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/utsname.h>

#define MAX 1024

void send_system_info(int sock) {
    struct utsname sys_info;
    uname(&sys_info);
    char info[MAX];
    snprintf(info, sizeof(info), "OS: %s, Node: %s, Release: %s, Version: %s, Machine: %s",
             sys_info.sysname, sys_info.nodename, sys_info.release, sys_info.version, sys_info.machine);
    send(sock, info, strlen(info), 0);
}

void send_file(int sock, char* filename) {
    FILE* f = fopen(filename, "r");
    char buffer[MAX];
    int bytes_read;

    if (f == NULL) {
        printf("File not found\n");
        return;
    }

    while ((bytes_read = fread(buffer, sizeof(char), MAX, f)) > 0) {
        send(sock, buffer, bytes_read, 0);
    }
    fclose(f);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./client <IPAddress> <PortNumber>\n");
        exit(1);
    }

    int client_sock;
    struct sockaddr_in server_addr;
    char username[MAX], password[MAX], mode[MAX], filename[MAX];
    char response[MAX];

    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);

    if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    // Step 1: Send username
    printf("Enter username: ");
    fgets(username, MAX, stdin);
    username[strcspn(username, "\n")] = '\0'; // Remove newline
    send(client_sock, username, strlen(username), 0);

    // Step 2: Authentication
    recv(client_sock, response, MAX, 0);
    response[1] = '\0';

    if (strcmp(response, "0") == 0) {
        printf("Username not found\n");
        close(client_sock);
        exit(0);
    } else if (strcmp(response, "2") == 0) {
        printf("Account is blocked\n");
        close(client_sock);
        exit(0);
    }

    printf("Enter password: ");
    fgets(password, MAX, stdin);
    password[strcspn(password, "\n")] = '\0'; // Remove newline
    send(client_sock, password, strlen(password), 0);

    recv(client_sock, response, MAX, 0);
    if (strcmp(response, "1") != 0) {
        printf("Login failed\n");
        close(client_sock);
        exit(0);
    }

    // Step 3: Choose mode
    printf("Choose mode (1: System Info, 2: Send File): ");
    fgets(mode, MAX, stdin);
    mode[strcspn(mode, "\n")] = '\0'; // Remove newline
    send(client_sock, mode, strlen(mode), 0);

    if (strcmp(mode, "1") == 0) {
        // Mode 1: Send system info
        send_system_info(client_sock);
    } else if (strcmp(mode, "2") == 0) {
        // Mode 2: Send file
        printf("Enter file name: ");
        fgets(filename, MAX, stdin);
        filename[strcspn(filename, "\n")] = '\0'; // Remove newline
        send(client_sock, filename, strlen(filename), 0);
        send_file(client_sock, filename);
    }

    close(client_sock);
    return 0;
}
