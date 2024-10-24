#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#define BUFF_SIZE 255

//Tach chu cai
void extract_letters(char *input, char *output) {
    int j = 0;
    for (int i = 0; i < strlen(input); i++) {
        if (isalpha(input[i])) {
            output[j++] = input[i];
        } else if (!isalnum(input[i])) {
            strcpy(output, "Error: Invalid input!");
            return;
        }
    }
    output[j] = '\0';
}

//Dao nguoc
void reverse_string(char *str){
    int n = strlen(str);
    for(int i = 0; i < (n/2); i++ ) {
        char temp = str[i];
        str[i] = str[n-i-1];
        str[n-i-1] = temp;
    }
}

int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr1, cliaddr2;
    socklen_t cli_len = sizeof(cliaddr1);
    char buff[BUFF_SIZE], result[BUFF_SIZE];

    if (argc != 2) {
		fprintf(stderr, "Usage: %s <UDP SERVER PORT>\n", argv[0]);
		exit(1);
	}

    short port = atoi(argv[1]);


//Step 1: Construct socke
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error: ");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

// Server running
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("Server is running on port %d...\n", port);

// client 1
    recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr1, &cli_len);
    sendto(sockfd, "user1", 5, 0, (struct sockaddr *)&cliaddr1, cli_len);

// client 2
    recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr2, &cli_len);
    sendto(sockfd, "user2", 5, 0, (struct sockaddr *)&cliaddr2, cli_len);

    while (1) {
// Nhận chuỗi từ client 1
        memset(buff, 0, BUFF_SIZE);
        recvfrom(sockfd, buff, BUFF_SIZE, 0, (struct sockaddr *)&cliaddr1, &cli_len);
// Diều kiện kết thúc
        if (strcmp(buff, "@") == 0 || strcmp(buff, "#") == 0) {
            printf("Client disconnected.\n");
            break;
        }

        printf("Received from Client 1: %s\n", buff);

// Đảo ngược chuỗi
        reverse_string(buff);
        
// Xử lý chuỗi và gửi cho client 2
        memset(result, 0, BUFF_SIZE);
        extract_letters(buff, result);
        sendto(sockfd, result, strlen(result), 0, (struct sockaddr *)&cliaddr2, cli_len);
    }
    close(sockfd);
    return 0;
}

