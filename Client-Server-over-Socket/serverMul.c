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
#include<pthread.h> //for threading 
#define PORT 12000
#define MAX_CLIENTS 10


struct ClientArgs {
    int socket;
    struct sockaddr_in clientAddr;
};


//the thread function
void *connection_handler(void *);

int main(int argc, char const* argv[])
{
    int server_socket, client_socket, valread, *new_sock;
    struct sockaddr_in client_addr, server_addr;
    int opt = 1;
    int server_port = atoi(argv[1]);
    int addrlen = sizeof(client_addr);
    int clientcnt = 0;
    pthread_t threads[MAX_CLIENTS];
    struct ClientArgs args;

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

        while( (client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&addrlen)) )
        {
            printf("\nmessage-from-client ");
            printf(": %s", inet_ntoa(client_addr.sin_addr));
            printf(" , %d\n", ntohs(client_addr.sin_port));

            
            args.socket = client_socket;
            args.clientAddr = client_addr;
            
            if (pthread_create(&threads[clientcnt], NULL, connection_handler, &args) != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
            clientcnt++;
        }
	
        if (client_socket < 0)
        {
            perror("accept failed");
            return 1;
        }


    }
	// closing the listening socket
	shutdown(server_socket, SHUT_RDWR);
    return 0;
}

/*
This will handle connection for each client
 * */
void *connection_handler(void *arg)
{

    struct ClientArgs *args = arg;
	//Get the socket descriptor
    int client_socket = args->socket;
    char *client_addr = inet_ntoa(args->clientAddr.sin_addr);
  	int client_port = ntohs(args->clientAddr.sin_port);

	int read_size;
	char client_message[2000];
	
	
	//Receive a message from client
	while( (read_size = recv( client_socket , client_message , 2000 , 0)) > 0 )
	{
			printf("Message: ");
        	printf("%s\n", client_message);

			char modifiedSentence[1024] = { 0 };
        	int j = 0;
        	char ch;
			while (client_message[j]){
				ch = client_message[j];
				modifiedSentence[j] = toupper(ch);
				j++;
			}
        	printf("Modified Message:");
        	printf(" %s\n", modifiedSentence);

			//Send the message back to client
       		send(client_socket, modifiedSentence, strlen(modifiedSentence), 0);
	}
	
	if(read_size == 0)
	{
			printf("\nclose-client ");
			printf(": %s", client_addr);
            printf(" , %d\n",client_port);
			close(client_socket);
			fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}
	
	pthread_exit(NULL);
}
