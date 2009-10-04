#include <stdio.h>
#include <string.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "header.h"
#include "helper.h"

#define STDIN 0


struct list_el {
  int val;
  struct list_el * next;
};

typedef struct list_el Node;

typedef struct P{
  char * name;
  int hp;
  int exp;
  int x;
  int y;
} Player;

int main(int argc, char* argv[]){

  // Model Variables
  Player * self = (Player *) malloc(sizeof(Player)); // Remember to free this
  Node * others;

  bool isLogin = false;
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
  printf("command >> ");

  while(!done){
    
    // Clear the set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(sock, &readfds);
    if (select(sock+1,&readfds,NULL,NULL,NULL) == -1){
      perror("select");
      return 0;
    }
    
    if(FD_ISSET(STDIN, &readfds)){
      // Handle stdin
      
      scanf("%s %s",command, arg);

      printf("command: %s\n arg: %s\n", command,arg);

      if (strcmp(command,"login") == 0){ // LOGIN
	char* name = arg; // TODO: Sanity check the input.'
	
	// Do some checking here
	self->name = arg;
	int status = handlelogin(name,sock);




      } else if(strcmp(command,"move") == 0){ // MOVE
	char* direction = arg;
	int status = handlemove(direction,sock);



      } else if(strcmp(command,"attack") == 0){ // ATTACK
	char* victim = arg;

	// Reprocessing

	if (strcmp(self->name, victim)==0){
	  return 0;
	}

	// Search for the guy in the list

	// Check if he is in sight

	// Handling message deliever

	if(strlen(victim) > 8){
	  // Say that the string is too long
	}

	int status = handleattack(victim,sock);

      } else if(strcmp(command,"speak") == 0){

	char * m = arg;

	// Check the message

	int status = handlespeak(m,sock);
	
	
      } else if(strcmp(command,"logout") == 0){
	done = 1;
      } else {
	printf("WTF?\n");
      }




    } else if (FD_ISSET(sock, &readfds)){ // Receiving from sock
      // Block and wait for response from the server
      unsigned char buffer[4096];
      int expected_data_len = sizeof(buffer);
      int read_bytes = recv(sock,buffer,expected_data_len,0);

      printf("buffer's len: %d\n", sizeof(buffer));

      int j;
      char header_c[4];
      for(j = 0; j < 4; j++){
	header_c[j] = buffer[j];
      }

      for(j = 0; j < 4; j++){
	printf("%02x ", header_c[j]);
      }

      struct header *  m = (struct header *) header_c;

      printf("Received a message.\n");
      printf("msgtype: %x\n", m->msgtype);

      // Check for the msgtype
      if(m->msgtype == LOGIN_REPLY){
	  // LOGIN_REPLY

	  struct login_reply * lreply = (struct login_reply *) m->payload;
	  if(isLogin){
	    // Treat it as a malformed package
	  } else {
	    if(lreply->error_code == 0){
	      // use message.h
	      self->hp = ntohs(lreply->hp);
	      self->exp = ntohs(lreply->exp);
	      self->x = ntohs(lreply->x);
	      self->y = ntohs(lreply->y);

	      isLogin = true;
	    } else if (lreply->error_code == 1){
	      // use message.h
	    }
	  }
      } else if(m->msgtype == MOVE_NOTIFY){
	  struct move_notify * mn = (struct move_notify *) m->payload;
	  // If the player is not insight, and the new location is not visible;
	  
	  Player * p;
	  bool found = false;

	  // Find the dude with the same name

	  if(!found){
	    // Add the dude to the array
	    // Call the dude p

	    //Player * p;
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
	  
      } else if(m->msgtype == ATTACK_NOTIFY){
	  struct attack_notify * an = (struct attack_notify *) m->payload;
	  Player * attacker,victim;
	  //set x,y for attacker and victim
	  /* foreach (@others){ */
/* 		  if (strcmp(an->attacker_name,$_->name) == 0){ */
/* 			  attacker->x = $_->x; */
/* 			  attacker->y = $_->y; */



	  //bool attacker_in_sight = abs((self->x - p->x))<=5 && abs((self->y - p->y))<=5;

	  //bool victim_in_sight = abs((self->x - p->x))<=5 && abs((self->y - p->y))<=5;

	  break;
      } else if(m->msgtype == SPEAK_NOTIFY){ //
      } else if(m->msgtype == LOGOUT_NOTIFY){
      }
    }
  }
}
