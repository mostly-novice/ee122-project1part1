#include <stdio.h>
#include <string.h>
#include "model.c"
#include "messages.h"
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>



int main(int argc, char* argv){
	Player *players;
	Player *self;
	int sockfd;
	sockaddr serv_addr;	//TODO: implement
	int isLogin = 0;
	int serverIP;
	int serverPort;

	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));


	if(argc < 4){
		printf("Usage: ./client -s <server IP address> -p <server port>");
	}

	serverIP = argv[1];
	serverPort = argv[3];

	sockfd = socket(AF_INET, SOCK_STEAM, 0);
	if(sockfd < 0){
		perror("socket() failed");
		abort();
	}else{
		printf("socket established\n");
	}

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(serverIP);
	sin.sin_port = htons(serverPort);

	if(connect(sockfd,(struct sockaddr *) &sin, sizeof(sin)) < 0){
		perror("connection failed");
		abort();
	}else{
		printf("Connection esablished\n");
	}

	

	printf("Hello Welcome to tiny World of Warcraft!\n");

	while(!done){
		printf("command >> ",name);
		scanf("%s",command);

		char* tokens = strtok(command," ");
		if (strcmp(tokens[0],"login") == 0){
			char* name = tokens[1]; // TODO: Sanity check the input.
			login(name);
		} else if(strcmp(command,"move") == 0){
		} else if(strcmp(command,"attack") == 0){
		} else if(strcmp(command,"speak") == 0){
		} else if(strcmp(command,"logout") == 0){
			done = 1;
		} else {
			printf("WTF?\n");
		}
	}

	// socket() - returns a socket descriptor
	sockfd = socket(PF_INET, SOCK_STREAM, 0);	//TODO: implement

	// connect() - connect to a remote host
	connection_status = connect(sockfd, sockaddr *serv_addr, addrlen);	//TODO: implement
}


// login
// @summary: Logs in with the player account. If no account,
//   create one and logs in. Generates message: LOGIN_REQUEST
//   
//   The other commands will be denied by the server with the 
//   INVALID_STATE message with the error code 0. (in other words,
//   the player must login before doing anything else.).	
//
// @arguments: 
// 	char* name - the name of the player. Must be alphanum-
// 		eric and up to 9 characters. 
char* login(char* name,Player * self){
	char * nullTerm = "\0";
	char 


	// Build the message
		// sanity check:
		// The name must be null-terminated. Thus, the effective maximum
	strcat(name,nullTerm);
	
		// name length is 9 bytes. The name must contain only alphanumeric characters.
	

	// Send a login message to the server


	// Block and wait for response from the server

	// Once received the messages, then party!
	// By party, I mean
	// Check for the message type to be correct
	// Initia0lize the player more(etc. hp, exp, location)
	
	// Create the player struct, initialize name
	self->name = name;


}












