#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "header.h"
#include "helper.h"
#include "messages.h"

#define STDIN 0
#define HEADER_LENGTH 4

typedef struct P{
  char * name;
  int hp;
  int exp;
  int x;
  int y;
} Player;

struct list_el {
  Player * datum;
  struct list_el * next;
};

typedef struct list_el Node;

Node * findPlayer(char * name, Node * list){
  Node * p;
  for(p = list; p != NULL; p = p->next){
    if (strcmp(p->datum->name,name)==0){
      return p;
    }
  }
  return NULL;
}

void initialize(Player * object,char * name, int hp, int exp, int x, int y){
  object->name = name;
  object->hp = hp;
  object->exp = exp;
  object->x = x;
  object->y = y;
}

int main(int argc, char* argv[]){

  // Model Variables
  Player * self = (Player *) malloc(sizeof(Player)); // Remember to free this
  Node * others;
  Node * tail;

  Node * p;

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

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0){
    perror("socket() failed\n");
    abort();
  }

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(serverIP);
  sin.sin_port = htons(serverPort);

  if(connect(sock,(struct sockaddr *) &sin, sizeof(sin)) < 0){
    close(sock);
    perror("connection failed");
    return;
  }

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
	      handlelogout(self->name,sock);
	done = 1;
      } else {
	printf("Unrecognized command.\n");
      }

    } else if (FD_ISSET(sock, &readfds)){ // Receiving from sock
      // Block and wait for response from the server
      unsigned char buffer[4096];
      int expected_data_len = sizeof(buffer);
      unsigned char *p = (char*) buffer; // Introduce a new pointer
      int offset = 0;

      int read_bytes = recv(sock,buffer,expected_data_len,0);
      
      int j;

      for(j = 0; j < read_bytes; j++){
	printf("%02x ", buffer[j]);
      }
      
      unsigned char header_c[HEADER_LENGTH];
      while(offset < read_bytes){
	for(j = 0; j < HEADER_LENGTH; j++){
	  header_c[j] = *(p+offset+j);
	}
	// Copy the headers
	offset += HEADER_LENGTH;

	/* for(j = 0; j < HEADER_LENGTH; j++){ */
/* 	  printf("%02x ", header_c[j]); */
/* 	} */

	// Cast it to struct header
	struct header * hdr = (struct header *) header_c;

	int payload_len = hdr->len-HEADER_LENGTH;

	char payload_c[payload_len];
	for(j = 0; j < payload_len; j++){
	  payload_c[j] = *(p+offset+j);
	}

	offset += payload_len;
      
	// @TODO: Check for the version

	// Check for the msgtype
	if(hdr->msgtype == LOGIN_REPLY){
	  // LOGIN_REPLY

	  struct login_reply * lreply = (struct login_reply *) payload_c;
	  if(isLogin){
	    // Treat it as a malformed package
	    on_malformed_message_from_server();

	  } else {
	    on_login_reply(lreply->error_code);

	    if(lreply->error_code == 0){
	      self->hp = ntohs(lreply->hp);
	      self->exp = ntohs(lreply->exp);
	      self->x = lreply->x;
	      self->y = lreply->y;

	      isLogin = 1;
	    } else if (lreply->error_code == 1){
	      
	    }
	  }
	} else if(hdr->msgtype == MOVE_NOTIFY){
	  printf("I got a move notify message.\n");
	  for(j = 0; j < 20; j++){
	    printf("%02x ", payload_c[j]);
	  }
	  struct move_notify * mn = (struct move_notify *) payload_c;
	  // If the player is not insight, and the new location is not visible;
	  Node *p;

	  /* printf("name: %s.\n", mn->name); */
/* 	  printf("hp: %x.\n", ntohl(mn->hp)); */
/* 	  printf("exp: %x.\n", ntohl(mn->exp)); */
/* 	  printf("x: %02x.\n", mn->x); */
/* 	  printf("y: %02x.\n", mn->y); */

/* 	  self->hp = ntohs(mn->hp); */
/* 	  self->exp = ntohs(mn->exp); */
/* 	  self->x = mn->x; */
/* 	  self->y = mn->y; */

	  if (strcmp(mn->name,self->name)==0){ // If the guy is self
	    printf("This message is for me.\n");
	    self->hp = ntohl(mn->hp);
	    self->exp = ntohl(mn->exp);
	    self->x = mn->x;
	    self->y = mn->y;

	    on_move_notify(self->name, self->x, self->y, self->hp,self->exp);

	  } else {
	    p = findPlayer(mn->name,others);
	    printf("1\n");
	    if(p == NULL){ // Not in the list
	      // Make a new node
	      Node * newnode = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
	      Player * newplayer = (Player*) malloc(sizeof(Player)); // TODO: remember to free this first
	      printf("3\n");
	      initialize(newplayer, mn->name, ntohl(mn->hp), ntohl(mn->exp), ntohs(mn->x), ntohs(mn->y));
	      printf("4\n");

	      newnode->datum = newplayer;
	      newnode->next = NULL;

	      if(tail == NULL){
		others = newnode;
		tail = newnode;
		printf("5\n");
	      } else {
		tail->next = newnode;
		tail = newnode;
	      }
      	      printf("6\n");
	      int oldp = abs((self->x - p->datum->x))<=5 
		&& abs((self->y - p->datum->y))<=5; // if the old position is in sight
	      int newp = abs((self->x - p->datum->x))<5 
		&& abs((self->y - p->datum->y))<5; // if the new position is in sight
	      printf("6\n");
	      if (oldp || newp){
		if (newp){
		  on_move_notify(p->datum->name, p->datum->x, p->datum->y, p->datum->hp,p->datum->exp);
		  printf("6\n");
		}
	      }
	    }
	  }
	  
	} else if(hdr->msgtype == ATTACK_NOTIFY){
	  struct attack_notify * an = (struct attack_notify *)buffer;
	  Player * attacker,victim;
	  //set x,y for attacker and victim
	  /* foreach (@others){ */
	  /* 		  if (strcmp(an->attacker_name,$_->name) == 0){ */
	  /* 			  attacker->x = $_->x; */
	  /* 			  attacker->y = $_->y; */



	  //bool attacker_in_sight = abs((self->x - p->x))<=5 && abs((self->y - p->y))<=5;

	  //bool victim_in_sight = abs((self->x - p->x))<=5 && abs((self->y - p->y))<=5;

	  break;
	} else if(hdr->msgtype == SPEAK_NOTIFY){ //
	} else if(hdr->msgtype == LOGOUT_NOTIFY){
//		struct logout_reply * loreply = (struct logout_reply *) payload_c;
//		printf("Player %s has left the tiny world of warcraft.\n",loreply->name);	  	
	}
      }
    }
  }
}
