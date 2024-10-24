#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFF_SIZE 1024

void send_string(int client_sockfd) {
    char buff[BUFF_SIZE];
    
    while (1) {
        // Send message
        printf("\nInsert string to send (ENTER to quit): ");
        fgets(buff, BUFF_SIZE, stdin);

        // Exit if user only input ENTER
        if (buff[0] == '\n') break;

        // Replace \n with \0
        buff[strcspn(buff, "\n")] = '\0';

        ssize_t sendBytes = send(client_sockfd, buff, strlen(buff), 0);
        if (sendBytes <= 0) {
            printf("\nConnection closed!\n");
            break;
        }

        // Receive echo reply
        char response[BUFF_SIZE];
        ssize_t recvBytes = recv(client_sockfd, response, BUFF_SIZE - 1, 0);
        if (recvBytes <= 0) {
            printf("\nError! Cannot receive data from server!\n");
            break;
        }

        // Print reply
        response[recvBytes] = '\0';
        puts("Reply from server:");
        puts(response);
    }
}

void send_file(int client_sockfd) {
    char filename[BUFF_SIZE];
    char buff[BUFF_SIZE];
    FILE *file;

    printf("\nEnter the file path to send: ");
    fgets(filename, BUFF_SIZE, stdin);
    filename[strcspn(filename, "\n")] = '\0';  // Remove newline character

    file = fopen(filename, "rb");
    if (!file) {
        printf("Error opening file %s\n", filename);
        return;
    }

    // Read file and send it to the server
    size_t bytesRead;
    while ((bytesRead = fread(buff, 1, BUFF_SIZE, file)) > 0) {
        ssize_t sendBytes = send(client_sockfd, buff, bytesRead, 0);
        if (sendBytes <= 0) {
            printf("\nConnection closed!\n");
            break;
        }
    }

    fclose(file);
    printf("File sent successfully.\n");
}

int main (int argc, char const *argv[])
{
	// valid number of argument
	if (argc != 3)
	{
		printf("Usage: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	int client_sockfd;
	struct sockaddr_in serv_addr;
	

	// Construct socket
	client_sockfd = socket(AF_INET,SOCK_STREAM,0);
	
	// Specify server address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	// Request to connect server
	if(connect(client_sockfd, (struct sockaddr*)&serv_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	int choice;
    while (1) {
        // Display menu
        printf("\nMENU -----------------------------------\n");
        printf("1. Gửi xâu bất kỳ\n");
        printf("2. Gửi nội dung một file\n");
        printf("0. Thoát\n");
        printf("Chọn chế độ: ");
        scanf("%d", &choice);
        getchar(); // Consume newline character left by scanf

        switch (choice) {
            case 1:
                send_string(client_sockfd);
                break;
            case 2:
                send_file(client_sockfd);
                break;
            case 0:
                close(client_sockfd);
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
	
	// Close socket
	close(client_sockfd);
	return 0;
}
