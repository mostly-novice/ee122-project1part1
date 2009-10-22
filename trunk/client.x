#include <stdio.h>
#include <string.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "header.h"
#include "helper.h"

struct Player{
  int hp;
  int exp;
  int x;
  int y;
};

int main(int argc, char* argv[]){

  // Model Variables
  struct Player * others;
  struct Player self;
  int isLogin = 0;
  char command[80];
  char arg[80];

  // Connection variables
  int sock;
  int serverIP;
  int serverPort;
  int done = 0;
  int status;

  // Select
  fd_set readfds;

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


  FD_ZERO(&readfds);
  FD_SET(STDIN, &readfds);
  FD_SET(sock, &readfds);

  while(!done){
    if (select(sock+1,&readfds,&writefds,NULL,NULL) == -1){
      perror("select");
      return 0;
    }
    
    if(FD_ISSET(STDIN, &readfds)){
      // Handle stdin
      
      printf("command >> ");
      scanf("%s %s",command, arg);

      if (strcmp(command,"login") == 0){
	char* name = arg; // TODO: Sanity check the input.
	int status = handlelogin(name,sock);
      } else if(strcmp(command,"move") == 0){

	char* direction = arg;
	int status = handlemove(direction,sock);

      } else if(strcmp(command,"attack") == 0){
      } else if(strcmp(command,"speak") == 0){
      } else if(strcmp(command,"logout") == 0){
	done = 1;
      } else {
	printf("WTF?\n");
      }




    } else if (FD_ISSET(sock, &readfds)){
      // Block and wait for response from the server
      char buffer[4096];
      int expected_data_len = sizeof(buffer);
      int read_bytes = recv(sock,buffer,expected_data_len,0);

      struct message * m = (struct message *) buffer;

      // Check for the msgtype
      switch m->msgtype {
	case messages.LOGIN_REPLY:
	  struct login_reply * lreply = (struct login_reply *) m->payload;
	  if(login){
	    // Treat it as a malformed package
	  } else {
	    if(lreply->error_code == 0){
	      // use message.h
	      self->hp = ntohs(lreply->hp);
	      self->exp = ntohs(lreply->exp);
	      self->x = ntohs(lreply->x);
	      self->y = ntohs(lreply->y);

	      login = true;
	    } else if (lreply->error_code == 1){
	      // use message.h
	    }
	  }
	  break;

	case messages.MOVE_NOTIFY:
	  struct move_notify * mn = (struct move_notify *) m->payload;
	  // If the player is not insight, and the new location is not visible;
	  
	  Player * p;
	  bool found = false;

	  for(p = others; p != NULL, p++){
	    if (strcmp(p->name,mn->name)==0){
	      found = true;
	      break;
	    }
	  }

	  if(!found){
	    // Add the dude to the array
	    // Call the dude p

	    struct Player * p;
	  }
	  
	  bool oldp = abs((self->x - p->x))<=5 && abs((self->y - p->y))<=5; // if the old position is in sight
	  bool newp = abs((self->x - p->x))<5 && abs((self->y - p->y))<5; // if the new position is in sight
	  if (oldp || newp){
	    // update his stat
	    if (newp){
	      // send out a message
	    }
	    // in either case, update the position for player p in database.
	  } else {
	    // Ignore
	  }
	  break;

	case messages.ATTACK_NOTIFY:
	  struct attack_notify * an = (struct attack_notify *) m->payload;
	  Player * attacker,victim;
	  //set x,y for attacker and victim
	  foreach (@others){
		  if (strcmp(an->attacker_name,$_->name) == 0){
			  attacker->x = $_->x;
			  attacker->y = $_->y;



	  bool attacker_in_sight = abs((self->x - p->x))<=5 && abs((self->y - p->y))<=5; // if the old position is in sight

	  bool victim_in_sight = abs((self->x - p->x))<=5 && abs((self->y - p->y))<=5; // if the old position is in sight

	  break;
        case messages.SPEAK_NOTIFY:
	  break;
	case LOGOUT_NOTIFY:
	  break;
      }

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







