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

	// socket() - returns a socket descriptor
	sockfd = socket(PF_INET, SOCK_STREAM, 0);	//TODO: implement

	// connect() - connect to a remote host
	connection_status = connect(sockfd, sockaddr *serv_addr, addrlen);	//TODO: implement

	if(sockfd < 0){
		perror("socket() failed");
		abort();
	}else{
		printf("socket established\n");
	}

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(serverIP);
	sin.sin_port = htons(serverPort);

	if(connect(sockfd,(sockaddr *serv_addr) &sin, sizeof(sin)) < 0){
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
			login(name,self,sockfd);
		} else if(strcmp(command,"move") == 0){
		} else if(strcmp(command,"attack") == 0){
		} else if(strcmp(command,"speak") == 0){
		} else if(strcmp(command,"logout") == 0){
			done = 1;
		} else {
			printf("WTF?\n");
		}
	}

}

// -------------------------------------------------------------
// login
//
// args: 
// 	char* name 	- the name of the player. Must be alphanum-
//	             	  eric and up to 9 characters. 
//	Player* self 	- the player running on the local client.
//	int sock	- the server socket.
//
// @returns:
// 	???
// -------------------------------------------------------------

char* login(char* name,Player * self,int sock){
<<<<<<< .mine
=======
	char * nullTerm = "\0";
	char version = 0x4;
	char* length = 0x0010;
	char* packet;
	char* response;
>>>>>>> .r8

<<<<<<< .mine
=======
	if(!check_player_name(name){
		printf("player name %s is not valid",name);
	}	

>>>>>>> .r8
<<<<<<< .mine
	// Build the message
		// sanity check:
		// The name must be null-terminated. Thus, the effective maximum
		// name length is 9 bytes. The name must contain only alphanumeric characters.
=======
>>>>>>> .r8

<<<<<<< .mine

	  int message[4];
	  int i = 0;
	  char * name = "kevintony";
	  printf("The size of name is: %d\n", strlen(name));

	  // Header
	  message[0] = LOGIN_REQUEST;
	  message[0] += (0x10 << 8);
	  message[0] += (0x04 << 24);

	  message[1] = 0;
	  message[2] = 0;
	  message[3] = 0;

	  for(i = 0; i < strlen(name); i++){
	    int messageIndex = i/4 + 1;
	    int shiftAmount = 8*(3- i % 4);

	    message[messageIndex] += name[i] << shiftAmount;
	  }

=======
	packet = build_message(version,length,LOGIN_REQUEST,name);

>>>>>>> .r8
	// Send a login message to the server
<<<<<<< .mine
	  int bytes_sent = send(sockfd,message,16,0)
	  if (sent_bytes < 0) {
	    perror("send failed");
	  }
=======
	int sent_bytes = send(sock, packet, sizeof(packet), 0);
	if(sent_bytes < 0){
		perror("send failed");
	}
>>>>>>> .r8

	// Block and wait for response from the server
<<<<<<< .mine
	  char buffer[4096];
	  int expected_data_len = sizeof(buffer);
	  
	  int read_bytes = recv(sock,buffer,expected_data_len,0);
	  if (read_bytes == 0){ // Connection is closed
	    printf("Connection closed.\n");
	  } else if(read_bytes < 0){
	    perror("recv failed");
	  } else { // OK. But no guarantee read_byes == expected_data_len
	    // Check if the receiving data is actual the correct packet
	    if(read_bytes != expected_data_len){
	      perror("Data malformed");
	    }
	  }     
=======
	while(1){
		printf("...\n");
	}
>>>>>>> .r8

	// Once received the messages, then party!
	// By party, I mean
	// Check for the message type to be correct
	// Initia0lize the player more(etc. hp, exp, location)
	self->HP = getHP(response);
	self->EXP = getEXP(response);
	self->location = getLocation(response);
	  int result = extractAndSet(buffer,self)
	  self->name = name;
}



char* logout(){
  // Generate the LOGOUT_REQUEST
  int message;
  message = LOGOUT_REQUEST;
  message += (0x0004 << 8);
  message += (0x04 << 24);

<<<<<<< .mine
  // Sending the LOGOUT_REQUEST
  
=======
// -------------------------------------------------------------
// build_message
//
// args: 
// 	version - 0x4 for login request.
// 	length - 0x0010 for login request.
// 	msg_type - LOGIN_REQUEST for ... yea.
// 	payload - the message being sent.
//
// returns:
// 	???
// -------------------------------------------------------------
>>>>>>> .r8

<<<<<<< .mine
  // Close the socket
  
}
=======
char * build_message(char version, char* length, char msg_type, char* payload){
	int message[4];
	int i;
	message[0] = version;
	message[0] += (length << 4);
	message[0] += (msg_type << 12);
	
	printf("version: %x\n",version);
	printf("length: %x\n",length);
	printf("msg_type: %x\n",msg_type);
	printf("----------------\n");
	printf("version:length:msg_type = %x\n",message[0]);
>>>>>>> .r8

	// fill out the payload in rows 1 - 3, leaving 1 byte for
	//   '\0' null character and 2 bytes of padding space.

<<<<<<< .mine
char* move(){
}
=======
	for(i = 0; i < strlen(payload); i++){
		int messageIndex = i/4 + 1;
		int shiftAmount = 8*(3- i % 4);
>>>>>>> .r8

		message[messageIndex] += payload[i] << shiftAmount;
	}

	printf("Message[1]: %x\n", message[1]);
	printf("Message[2]: %x\n", message[2]);
	printf("Message[3]: %x\n", message[3]);

	return message;
}










