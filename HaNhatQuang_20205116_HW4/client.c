#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define BUFF_SIZE 255

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t sin_size;
    char buff[BUFF_SIZE];
    char result[BUFF_SIZE];

    if (argc != 3) {
		fprintf(stderr, "Usage: %s <ServerIP> [<EchoPort>]\n", argv[0]);
		exit(1);
	}

    char *server_IP = argv[1];
    short server_PORT = atoi(argv[2]);

//Step 1: Construct socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Cannot create socket");
        exit(1);
    }

//Step 2: Define the address of the server
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(server_IP);
    servaddr.sin_port = htons(server_PORT);

    sin_size = sizeof(servaddr);

    sendto(sockfd, "user1", 5, 0, (struct sockaddr *)&servaddr, sin_size);
    recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&servaddr, &sin_size);

    printf("Connected as %s\n", buff);

    //Step 3: Communicate with server
    while (1) {
        printf("Enter: ");
        fgets(buff, BUFF_SIZE, stdin);
        buff[strcspn(buff, "\n")] = 0; 

        if (strcmp(buff, "@") == 0 || strcmp(buff, "#") == 0) {
            sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&servaddr, sin_size);
            break;
        }

        // Gửi chuỗi đến server
        sendto(sockfd, buff, strlen(buff), 0, (struct sockaddr *)&servaddr, sin_size);

        // Nhận kết quả từ server
        memset(result, 0, BUFF_SIZE);
        recvfrom(sockfd, result, BUFF_SIZE, 0, (struct sockaddr *)&servaddr, &sin_size);
        printf("Received from Server: %s\n", result);
    }

    close(sockfd);
    return 0;
}
    
