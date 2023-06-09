// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ctype.h>
#define PORT 12000

int main(int argc, char const* argv[])
{
    int server_socket, client_socket, valread;
    struct sockaddr_in client_addr, server_addr;
    int opt = 1;
    int addrlen = sizeof(client_addr);
    int server_port = atoi(argv[1]);

    // Creating socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 12000
    if (setsockopt(server_socket, SOL_SOCKET,
                SO_REUSEADDR, &opt,
                sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(server_port);

    // Forcefully attaching socket to the port 12000
    if (bind(server_socket, (struct sockaddr*)&server_addr,
            sizeof(server_addr))
        < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("The server is ready to receive\n\n");

    while (1){
        if ((client_socket
            = accept(server_socket, (struct sockaddr*)&client_addr,
            (socklen_t*)&addrlen)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

		printf("message-from-client ");
		printf(": %s", inet_ntoa(client_addr.sin_addr));
        printf(" , %d\n",  ntohs(client_addr.sin_port));
		printf("\n");

        char sentence[1024] = { 0 };

		//Receive a message from client
		while( (valread = recv(client_socket, sentence, 1024 , 0)) > 0 )
		{
			printf("Message: ");
        	printf("%s\n", sentence);

			char modifiedSentence[1024] = { 0 };
        	int j = 0;
        	char ch;
			while (sentence[j]){
				ch = sentence[j];
				modifiedSentence[j] = toupper(ch);
				j++;
			}
        	printf("\nModified Message:");
        	printf(" %s\n", modifiedSentence);

			//Send the message back to client
       		send(client_socket, modifiedSentence, strlen(modifiedSentence), 0);
			
		}
		
		if(valread == 0)
		{
			printf("\nclose-client ");
			printf(": %s", inet_ntoa(client_addr.sin_addr));
            printf(" , %d\n",ntohs(client_addr.sin_port));
			close(client_socket);
			fflush(stdout);
		}
		else if(valread == -1)
		{
			perror("recv failed");
		}

    }
	// closing the listening socket
	shutdown(server_socket, SHUT_RDWR);
    return 0;
}

