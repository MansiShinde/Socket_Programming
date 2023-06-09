// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#define PORT 12000

int main(int argc, char const* argv[])
{
	const char* host = argv[1];
	int server_port = atoi(argv[2]);
	const char* sentence = argv[3];
	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	
	char modifiedSentence[1024] = { 0 };

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, host, &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}

	if ((client_fd
		= connect(sock, (struct sockaddr*)&serv_addr,
				sizeof(serv_addr)))
		< 0) {
		printf("\nConnection Failed \n");
		return -1;
	}
	printf("Connection established with Server \n\n");

	printf("Sending the message to the server.\n");

	
	send(sock, sentence, strlen(sentence), 0);

	printf("\nModified message received from server:\n");
	valread = recv(sock, modifiedSentence, 1024,0);
	printf("%s\n", modifiedSentence);

	sleep(60);
	printf("Closing the connection\n");

	// closing the connected socket
	close(client_fd);
	return 0;
}
