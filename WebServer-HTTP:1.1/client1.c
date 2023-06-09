
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 12000
#define BUFFER_SIZE 1024

int main(int argc, char const* argv[])
{
	const char* url = argv[1];

	char server_ip[100];
    int port = 80;
    char filename[100];
    sscanf(url, "http://%99[^:]:%99d/%99[^\n]", server_ip, &port, filename);
    printf("ip = \"%s\"\n", server_ip);
    printf("port = \"%d\"\n", port);
    printf("page = \"%s\"\n", filename);

	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	char message[256];

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr)
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
	printf("Connection established with Server \n");


	// printf("Input lowercase sentence:\n");
	// fgets(sentence, sizeof(sentence), stdin);

	strcat(message, "GET /");
	strcat(message, filename);
	strcat(message, " HTTP/1.1\r\n\r\n");
	if(send(sock, message, strlen(message), 0) < 0){
		printf("Send Failed\n");
	}
	
	// char serverreply[BUFFER_SIZE];
	// char *response, *header_response;
	// int header_res = recv(sock, header_response, 4999,0);
	// header_response[header_res] = 0;

	char header[200];
    int content_length;
	int n = 0;
	char response [30000] ={ 0 }, serverreply[30000] = { 0 }; ;

    content_length = recv(sock, header, sizeof(header) - 1, 0);
    header[content_length] = 0;
    printf("%s\n ", header);


	while((n = recv(sock, serverreply, BUFFER_SIZE,0)) > 0){
		serverreply[n] = '\0';
		printf("serverreply: %s\n",serverreply);
		strcat(response,serverreply);
		if(strstr(response,"</html>")!=NULL){
			break;
		}
	}

	printf("Response is:%s\n",response);
	printf("Print something\n");


	
	int o= 0 ;
	char* object[20] = { 0 };

	char* objToken = strtok(response," ");
// 	   while (objToken != NULL) {
//       printf("%s\n", objToken);
//       objToken = strtok(NULL, " ");
//    }
	printf("%s\n", objToken);
	if(strstr(objToken,".jpeg")!=NULL || strstr(objToken,".mp4")!=NULL){
		object[0] = objToken;
		o++;
		printf("%d %s\n", o, objToken);

	}

	while(objToken!=NULL && strstr(objToken,"</html>")==NULL){
		objToken = strtok(NULL," ");
		if(strstr(objToken,".jpeg")!=NULL || strstr(objToken,".mp4")!=NULL) {
			object[o] = objToken;
			printf("%d %s\n", o, object[o]);
			o++;
		}
	}
	int ind;

	for(ind = 0;ind<o;ind++) {
		printf("inside for loop\n");
		char * requesttoken;
		int x = 0;
		char* request[20] ={ 0 };
		requesttoken = strtok(object[ind],"\"");
		while(requesttoken!=NULL) {
			requesttoken = strtok(NULL,"\"");
			request[x] = requesttoken;
			x++;
		}
		char moreMessage[1024] = { 0 };
		strcat(moreMessage,"GET /");
		strcat(moreMessage,request[0]);
		strcat(moreMessage, " HTTP/1.1\r\n");
		//send(sock,moreMessage,strlen(moreMessage),0);
    	if(send(sock, moreMessage, strlen(moreMessage), 0)<0){
        	printf("Send failed\n");
    	}
		printf("%s\n",moreMessage);

		serverreply[0] = '\0';
		response[0] = '\0';
		int counter = 0;

		while((n = recv(sock, serverreply, BUFFER_SIZE,0)) > 0){
		serverreply[n] = '\0';
		printf("Object-Frame:%d", ind);
		printf(" frame- %d\n",++counter);
		strcat(response,serverreply);
		}

		
	}

	sleep(60);
	printf("Closing the connection\n");

	// closing the connected socket
	close(client_fd);
	return 0;
}
