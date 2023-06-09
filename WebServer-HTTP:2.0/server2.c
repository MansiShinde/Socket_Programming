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
#define BUFFER_SIZE 40*1024
#define MAX_CLIENTS 10

//the thread function
void *connection_handler(void *);
void handlingMultipleRequests(int requestNo, int client_socket);

struct ClientArgs {
    int socket;
    struct sockaddr_in clientAddr;
};

struct RemainingMessageSize {
    int datasize;
    int frameNo;
}remainingData[3];


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
	char request[1024] = {0};
    int flagCheck = 0; 
	FILE* file; 
    char buffer[BUFFER_SIZE] = {0};
    int fileLen;
    char *errormsg;
    int requestNo = 0;

	
	//Receive a message from client
	while( (read_size = recv( client_socket , request , 2000 , 0)) > 0 )
	{

        if(strstr(request, "EndOfMessage") != NULL){
            printf("End of file reached\n");

            handlingMultipleRequests(requestNo, client_socket);
            // read_size = 0;
            // break;
        }
        printf("\nmessage-from-client ");
        printf(": %s", client_addr);
        printf(" , %d\n", client_port);


        char *first_line_of_request = strtok(request, "\n"); ; 
        printf(" %s\n", first_line_of_request); 

        // Read the request
        char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
        
        //get Method, version and uri
        sscanf(request, "%s %s %s", method, uri, version);

        if(strcmp(method,"GET")!=0){
            flagCheck = 1;
        }

        if(strstr(version, "HTTP")==NULL){
            flagCheck = 1;
        }

        //Constructing path   
        char path[1024] = { 0 }; 

        
        // get the path from uri
        if(strstr(uri,"www")!=NULL){
            strcat(path,".");
            strcat(path,uri);
        }
        else{
            strcat(path,"./www");
            strcat(path,uri);
        }

        //Open file
        file = fopen(path, "rb");
        
        if(strstr(version,"2.0")==NULL && flagCheck == 0){
            errormsg = "HTTP/2.0 505 HTTP Version Not Supported\r\nConnection: close\r\nContent-Length:30\r\n\r\n505 HTTP Version Not Supported";
            send(client_socket, errormsg, strlen(errormsg), 0);
		    printf("\nmessage-to-client: %s  %d\n", client_addr, client_port);
		    printf("HTTP/2.0 505 HTTP Version Not Supported\n");
            close(client_socket);
        }
         else if(file==NULL && flagCheck == 0){
            errormsg = "HTTP/2.0 404 Not Found\r\nConnection: close\r\nContent-Length:13\r\n\r\n404 Not Found";
            send(client_socket, errormsg, strlen(errormsg), 0);
		    printf("\nmessage-to-client: %s  %d\n", client_addr, client_port);
		    printf("HTTP/2.0 404 Not Found\n");
            close(client_socket);
            fclose(file);
        } else if(file!=NULL & flagCheck == 0){
            //Get file length
            fseek(file, 0, SEEK_END);
            fileLen=ftell(file);
            fseek(file, 0, SEEK_SET);
            char header[102400];

            char *dot = strrchr(uri+1, '.');

            if(strcmp(dot+1, "jpeg") == 0){
            sprintf(header, 
            "HTTP/2.0 200 OK\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %i\r\n"
                    "\r\n", fileLen);
            } else if(strcmp(dot+1, "html") == 0){
            sprintf(header, 
            "HTTP/2.0 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %i\r\n"
                    "\r\n", fileLen);
            }else if(strcmp(dot+1, "mp4") == 0){
            sprintf(header, 
            "HTTP/2.0 200 OK\r\n"
            "Content-Type: video/mp4\r\n"
            "Content-Length: %i\r\n"
                    "\r\n", fileLen); 
            }

            int readret;
            send(client_socket, header, strlen(header), 0);
            printf("\nmessage-to-client: %s  %d\n", client_addr, client_port);
		    printf("HTTP/2.0 200 OK\n");

            if(strcmp(dot+1, "html") == 0) {
            if(file){
            while((readret=fread(buffer,sizeof(unsigned char),BUFFER_SIZE,file))>0){
                if (send(client_socket, buffer, readret, 0) != readret){
                    close(client_socket);
                    fprintf(stderr, "Error sending to client.\n");
                }
            } 
        }
            fclose(file);
        } else{
            printf("requestNo:%d\n",requestNo);
            remainingData[requestNo].datasize = fileLen;
            printf("file len:%d\n", remainingData[requestNo].datasize);
            requestNo++;
        }
        } else{
        errormsg = "HTTP/2.0 400 Bad Request\r\nConnection: close\r\nContent-Length:15\r\n\r\n400 Bad Request";;
		send(client_socket,errormsg,strlen(errormsg),0);
		printf("\nmessage-to-client: %s  %d\n", client_addr, client_port);
		printf("HTTP/2.0 400 Bad Request\n");
		close(client_socket);
     }
	}
	if(read_size == 0)
	{
			printf("\nclose-client ");
			printf(": %s", client_addr);
            printf(" , %d\n\n",client_port);
			close(client_socket);
			fflush(stdout);
	}
	else if(read_size == -1)
	{
		close(client_socket);
	}
		
    
	//Free the socket pointer
	pthread_exit(NULL);
}

void handlingMultipleRequests(int requestNo, int client_socket){
    int totalDataArraySize = 0;
    char frNo[50] ; 
    char response[BUFFER_SIZE]; 
    int modified_buffer_size = BUFFER_SIZE-50 ;  //to store certain words 

    for(int i=0;i<requestNo;i++){
        totalDataArraySize += remainingData[i].datasize ; 
        remainingData[i].frameNo =1;
    }

    printf("initial total data size:%d\n",totalDataArraySize);
	if(totalDataArraySize < modified_buffer_size){

        frNo[0] = '\0' ; 
        response[0]='\0';

        if(requestNo==0){
                sprintf(response,"Object-Frame:[Object %d]", 1);
                sprintf(frNo," Frame %d", remainingData[0].frameNo);
                strcat(response, frNo);
                strcat(response, "\n");
                printf("sending :%s\n", response);

                send(client_socket, response, strlen(response), 0) ; 
                remainingData[0].frameNo++;
                remainingData[0].datasize =0;
                totalDataArraySize =0;  
        }else{
               for(int ind=0;ind<requestNo;ind++){
                frNo[0] = '\0' ; 
                response[0]='\0';
                sprintf(response,"Object-Frame:[Object %d]", ind+1);
                sprintf(frNo," Frame %d", remainingData[ind].frameNo);
                strcat(response, frNo);
                strcat(response, "\n");
                printf("sending :%s", response);

                send(client_socket, response, strlen(response), 0) ;
            
                remainingData[ind].frameNo++;
                remainingData[ind].datasize = 0;
                totalDataArraySize -= modified_buffer_size;
               } 
        }

        send(client_socket, "Send-All-Frames", 15, 0) ;

    }else{
        while(totalDataArraySize > modified_buffer_size){

        for(int ind=0;ind<requestNo;ind++){

            frNo[0] = '\0' ; 
            response[0]='\0';
    
            if(remainingData[ind].datasize > modified_buffer_size) {
                
                sprintf(response,"Object-Frame:[Object %d]", ind+1);
                sprintf(frNo," Frame %d", remainingData[ind].frameNo);
                strcat(response, frNo);
                strcat(response, "\n");
                printf("sending :%s", response);

                send(client_socket, response, strlen(response), 0) ; 

                remainingData[ind].frameNo++;
                remainingData[ind].datasize -= modified_buffer_size;
                totalDataArraySize -= modified_buffer_size;
            } else if(remainingData[ind].datasize > 0){
                sprintf(response,"Object-Frame:[Object %d]", ind+1);
                sprintf(frNo," Frame %d", remainingData[ind].frameNo);
                strcat(response, frNo);
                strcat(response, "\n");
                printf("sending :%s", response);

                send(client_socket, response, strlen(response), 0) ; 
     
            
                remainingData[ind].frameNo++;
                remainingData[ind].datasize = 0;
                totalDataArraySize -= modified_buffer_size;
            }
            }
        }
        int bytes_send = send(client_socket, "Send-All-Frames", BUFFER_SIZE, 0) ;
    }
}