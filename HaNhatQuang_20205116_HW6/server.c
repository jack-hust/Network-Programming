#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sha256.h"

#define BACKLOG 5
#define BUFF_SIZE 1024

// seperate a string to 2 line: numbers and letters
char *tach_xau(char* buff)
{
	// return NULL if input string is empty
	if (strlen(buff) == 0)
		return NULL;
	
	char numbers[BUFF_SIZE], letters[BUFF_SIZE], *result; 
	int i, count_number = 0, count_letter = 0;
	memset(numbers, '\0', BUFF_SIZE);
	memset(letters, '\0', BUFF_SIZE);
	result = malloc(BUFF_SIZE+1);

	// seperate numbers and letters
	for (i = 0; buff[i] != '\0'; i++)
	{
		if (isdigit(buff[i]))
			numbers[count_number++] = buff[i];
		else if (isalpha(buff[i]))
			letters[count_letter++] = buff[i];
		else
			return NULL;
	}
	sprintf(result, "%s\n%s", numbers, letters);
	return result;
}

// Function to receive a file from the client
void receive_file(int conn_sockfd) {
    char buff[BUFF_SIZE];
    FILE *file;
    char filename[BUFF_SIZE];
    
    // Get the filename from the client
    int recvBytes = recv(conn_sockfd, filename, BUFF_SIZE - 1, 0);
    if (recvBytes <= 0) {
        printf("Error receiving filename\n");
        return;
    }
    filename[recvBytes] = '\0';  // Null-terminate the filename
    printf("Receiving file: %s\n", filename);
    
    // Open file for writing
    file = fopen(filename, "wb");
    if (!file) {
        printf("Error opening file %s for writing\n", filename);
        return;
    }

    // Receive the file data
    while ((recvBytes = recv(conn_sockfd, buff, BUFF_SIZE, 0)) > 0) {
        fwrite(buff, 1, recvBytes, file);
    }

    fclose(file);
    printf("File received successfully: %s\n", filename);
}


int main(int argc, char const *argv[])
{
	// valid number of argument
	if (argc != 2)
	{
		printf("Usage: ./server PortNumber\n\n");
		return 0;
	}

	int listen_sockfd, conn_sockfd;
	int recvBytes, sendBytes;

	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	
	char data[BUFF_SIZE];
	
	socklen_t sin_size;
	
	// Construct a TCP socket to listen connection request
	if ((listen_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
		perror("\nError: ");
		return 0;
	}
	
	// Bind address to socket
	memset(&servaddr, '\0', sizeof servaddr);
	servaddr.sin_family = AF_INET;         
	servaddr.sin_port = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listen_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))==-1){
		perror("\nError: ");
		return 0;
	}     
	
	// Listen request from client
	if(listen(listen_sockfd, BACKLOG) == -1){
		perror("\nError: ");
		return 0;
	}
	
	printf("Server started!\n");

	// Communicate with client
	while(1) 
	{
		//accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sockfd = accept(listen_sockfd,( struct sockaddr *)&cliaddr, &sin_size)) == -1) 
			perror("\nError: ");
  
		printf("Connecting Clinet ID: %s\n", inet_ntoa(cliaddr.sin_addr) ); /* prints client's IP */
		
		//start conversation
		while(1)
		{
			//receives message from client
			recvBytes = recv(conn_sockfd, data, BUFF_SIZE-1, 0); //blocking
			if (recvBytes <= 0)
			{
				printf("\nConnection closed");
				break;
			}

			// handle received data
			data[recvBytes] = '\0';
			printf("%s\n", data);

            if (strncmp(data, "FILENAME:", 9) == 0) {
                // Receive the file
                receive_file(conn_sockfd);
            }
            else{
                char sha256_hex_str[SHA256_HEX_SIZE];
                sha256_hex(data, strlen(data), sha256_hex_str);
                printf("SHA256 Encoded: %s\n", sha256_hex_str);

                char *reply = tach_xau(sha256_hex_str);
                
            
                // if string contain symbol return Error
                if (reply == NULL)
                    reply = "Error";

                //echo to client
                sendBytes = send(conn_sockfd, reply, strlen(reply), 0);
                if (sendBytes <= 0)
                {
                    printf("\nConnection closed");
                    break;
                }
                //end conversation
            }
		} 
		close(conn_sockfd);	
	}
	close(listen_sockfd);
	return 0;
}
