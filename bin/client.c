#include <stdio.h>
#include <string.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

struct Player{
  int hp;
  int exp;
  int x;
  int y;
};

struct header {
  unsigned char version;
  char l;
  char len;
  unsigned char msgtype;
  unsigned int payload[3];
};

struct login_reply {
  unsigned char version;
  unsigned char l;
  unsigned char len;
  unsigned char msgtype;

  uint8_t error_code;
  uint32_t hp;
  uint32_t exp;
  uint8_t x;
  uint8_t y;
  uint8_t padding;
}__attribute__((packed));

int main(int argc, char* argv[]){

  // Model Variables
  struct Player *players;
  struct Player *self;
  int isLogin = 0;
  char command[80];
  char arg[80];

  // Connection variables
  int sock;
  int serverIP;
  int serverPort;
  int done = 0;
  int status;

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));


  if(argc < 4){
    printf("Usage: ./client -s <server IP address> -p <server port>");
  }

  // Initilizations
  
  serverIP = argv[2];
  serverPort = atoi(argv[4]);

  printf("serverIP: %s\n", serverIP);
  printf("serverPort: %d\n", serverPort);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0){
    perror("socket() failed\n");
    abort();
  }else{
    printf("socket established: sock = %d\n", sock);
  }

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(serverIP);
  sin.sin_port = htons(serverPort);

  if(connect(sock,(struct sockaddr *) &sin, sizeof(sin)) < 0){
    close(sock);
    perror("connection failed");
    return;
  }else{
    printf("Connection esablished\n");
  }

  printf("Hello Welcome to tiny World of Warcraft!\n");
  while(!done){
    printf("command >> ");
    scanf("%s %s",command, arg);

    if (strcmp(command,"login") == 0){
      char* name = arg; // TODO: Sanity check the input.
      int status = handlelogin(name,sock);
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
int handlelogin(char* name,int sock){

  // Build the message
  // sanity check:
  // The name must be null-terminated. Thus, the effective maximum
  // name length is 9 bytes. The name must contain only alphanumeric characters.


  int message[4];
  int i = 0;

  printf("client - sock: %d\n", sock);

  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int));

  hdr->version = 0x04;
  hdr->len = 0x10;
  hdr->msgtype = 0x1;
  hdr->payload[0] = 0x0;
  hdr->payload[1] = 0x0;
  hdr->payload[2] = 0x0;

  for(i = 0; i < strlen(name); i++){
    int messageIndex = i/4;
    int shiftAmount = 8*(3- i % 4);

    hdr->payload[messageIndex] += name[i] << shiftAmount;
  }

  unsigned char * msg = (unsigned char *) hdr;

  // Send a login message to the server
  int bytes_sent = send(sock,msg,16,0);
  if (bytes_sent < 0) {
    perror("send failed");
  } else {
    printf("Send: %d bytes\n", bytes_sent);
  }

  // Block and wait for response from the server
  char buffer[4096];
  int expected_data_len = sizeof(buffer);
	  
  int read_bytes = recv(sock,buffer,expected_data_len,0);

  struct login_reply * lreply = (struct login_reply *) buffer;

  // Check if the version is correct

  if (read_bytes == 0){ // Connection is closed
  } else if(read_bytes < 0){
    perror("recv failed");
  } else {
    if(read_bytes != expected_data_len){
	printf("len: %d\n", lreply->len);
      	printf("msg type: %d\n", lreply->msgtype);
      	printf("error code: %d\n", lreply->error_code);
       printf("hp: %d\n", ntohl(lreply->hp));
       printf("exp: %d\n", ntohl(lreply->exp));
       printf("x: %d\n", lreply->x);
       printf("y: %d\n", lreply->y);
    }
  }
  return 0;
}



/* char* logout(){ */
/*   // Generate the LOGOUT_REQUEST */
/*   int message; */
/*   message = LOGOUT_REQUEST; */
/*   message += (0x0004 << 8); */
/*   message += (0x04 << 24); */

/*   // Sending the LOGOUT_REQUEST */
  

/*   // Close the socket */
  
/* } */


char* move(){
}







