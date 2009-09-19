#include <stdio.h>
#include "messages.h"
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char* argv){

	int sockfd;
	sockaddr serv_addr;	//TODO: implement

// Do some sanity checking of char* argv here
	if(argc < 4){
		printf("Usage: ./client -s <server IP address> -p <server port>");
	}

// socket() - returns a socket descriptor
	sockfd = socket(PF_INET, SOCK_STREAM, 0);	//TODO: implement

// connect() - connect to a remote host
	connection_status = connect(sockfd, sockaddr *serv_addr, addrlen);	//TODO: implement

	
// TODO: run a while(1) loop after a successful connection
// 	This loop should look like: command>

// dummy code
	char name [80];
	char command [80];
	int done = 0;

	printf("Hello!\n");
	printf("Who are you?");
	scanf("%s",name);
	while(!done){
		printf("command%s >> ",name);
		scanf("%s",command);
		if(strcmp(command,"exit") == 0){
			done = 1;
		}
	}



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
char* login(char* name){

}
