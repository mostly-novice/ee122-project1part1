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
#include "aux.h"

#define STDIN 0
#define HEADER_LENGTH 4

typedef struct P{
  char name[10];
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

void stats(Player * p){
  fprintf(stdout, "%s: location=(%u,%u), HP=%u, EXP=%u\n",
          p->name, p->x, p->y, p->hp, p->exp);
}

void printPlayers(Node * list){
  Node * p;
  for(p = list; p != NULL; p = p->next){
    stats(p->datum);
  }
}


Node * findPlayer(char * name, Node * list){
  Node * p;
  for(p = list; p != NULL; p = p->next){
    if (strcmp(p->datum->name,name)==0){
      printf("Found the guy: %s.\n", name);
      return p;
    }
  }
  printf("Didn't find the guys: %s\n", name);
  return NULL;
}

unsigned int removePlayer(char * name, Node *list)
{
  Node * prev = list;
  Node * curr;
  Node * temp;
	
  /* check for empty list */
  if(!list){
    return 0;
  }

  /* check if datum is in head of list */
  if(strcmp(prev->datum->name,name)==0){
    list = list->next;
    free(prev->datum);
    free(prev);
    return 1;
  }

  while( curr->next ){
    if( strcmp( curr->next->datum->name,name )==0 ){
      temp = curr->next;
      curr->next = curr->next->next;
      free(temp->datum);
      free(temp);
    }
  }
}



Node * freePlayers(Node * list)
{
  // To free all the players after log out
  Node * head = list;
  Node * curr;
  while(head)
    {
      curr = head;
      head = curr->next;
      free(curr->datum);
      free(curr);
    }

}

Node * addPlayer(Node * node, Node * list, Node * tail){
  if(tail == NULL && list == NULL){ // First player
    printf("%s is the first player.\n", node->datum->name);
    tail = node;
    list = node;
  } else {
    printf("%s is not the first player.\n", node->datum->name);
    tail->next = node;
    tail = node;
  }
}

void initialize(Player * object,char * name, int hp, int exp, int x, int y){
  strcpy(object->name,name);
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
  fd_set writefds;

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

  

  while(1){
    // Clear the set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(sock, &readfds);
      
    if (select(sock+1,&readfds,NULL,NULL,NULL) == -1){
      perror("select");
      return 0;
    }

    /*
     * Handling stdin
     *
     */
    
    if(FD_ISSET(STDIN, &readfds)){
      show_prompt();
      readstdin(command,arg);

      if (strcmp(command,"login") == 0){ // LOGIN
	char* name = arg; // TODO: Sanity check the input.'
	
	strcpy(self->name,arg);
	int status = handlelogin(name,sock);


      } else if(strcmp(command,"move") == 0){ // MOVE
	char* direction = arg;
	int status = handlemove(direction,sock);



      } else if(strcmp(command,"attack") == 0){ // ATTACK
	char* victimname = arg;

	if(!check_player_name(victimname)){
	  printf("! Invalid name: %s\n", victimname);
	  continue;
	}

	// Search for the guy in the list
	Player * victim = findPlayer(victimname,others);

	if (!victim){ // If it is NULL
	  on_not_visible();
	  continue;
	}

	int isvisible = isVisible(self->x,self->y,victim->x,victim->y);

	if(!isvisible){
	  on_not_visible();
	  continue;
	}

	int status = handleattack(victim,sock);

      } else if(strcmp(command,"speak") == 0){

	char * m = arg;

	// Check the message

	if(handlespeak(m,sock)){
	  perror("handlespeak");
	}
	
	
      } else if(strcmp(command,"logout") == 0){
	if (handlelogout(self->name,sock) < 0){
	  perror("handlelogout");
	}
	freePlayers(others);
	exit(1);

      } else if(strcmp(command,"whois") == 0){
	printPlayers(others);

      } else {
	printf("Unrecognized command.\n");
      }












      /*
       * Handling data from sock
       *
       */





    } else if (FD_ISSET(sock, &readfds)){ // Receiving from sock
      // Block and wait for response from the server
      unsigned char buffer[4096];
      int expected_data_len = sizeof(buffer);
      unsigned char *p = (char*) buffer; // Introduce a new pointer
      int offset = 0;

      int read_bytes = recv(sock,buffer,expected_data_len,0);
      
      int j;

      /* for(j = 0; j < read_bytes; j++){ */
      /* 	printf("%02x ", buffer[j]); */
      /*       } */
      
      unsigned char header_c[HEADER_LENGTH];
      while(offset < read_bytes){
	for(j = 0; j < HEADER_LENGTH; j++){
	  header_c[j] = *(p+offset+j);
	}
	// Copy the headers
	offset += HEADER_LENGTH;

	// Cast it to struct header
	struct header * hdr = (struct header *) header_c;

	int payload_len = ntohs(hdr->len)-HEADER_LENGTH;

	char payload_c[payload_len];

	for(j = 0; j < payload_len; j++){
	  payload_c[j] = *(p+offset+j);
	}

	offset += payload_len; // Increment the offset
      
	// @TODO: Check for the version

	// Check for the msgtype
	if(hdr->msgtype == LOGIN_REPLY){ // LOGIN REPLY

	  struct login_reply * lreply = (struct login_reply *) payload_c;
	  if(isLogin){

	    // Treat it as a malformed package
	    on_malformed_message_from_server();

	  } else {

	    on_login_reply(lreply->error_code);

	    if(lreply->error_code == 0){
	      self->hp = ntohl(lreply->hp);
	      self->exp = ntohl(lreply->exp);
	      self->x = lreply->x;
	      self->y = lreply->y;

	      isLogin = 1;
	    }
	  }
	} else if(hdr->msgtype == MOVE_NOTIFY){

	  struct move_notify * mn = (struct move_notify *) payload_c;
	  Node *p;

	  if (strcmp(mn->name,self->name)==0){ // If the guy is self
	    self->hp = ntohl(mn->hp);
	    self->exp = ntohl(mn->exp);
	    self->x = mn->x;
	    self->y = mn->y;

	    on_move_notify(self->name, self->x, self->y, self->hp,self->exp);

	  } else { // The guy is someone else

	    p = findPlayer(mn->name,others);

	    if(p == NULL){ // Not in the list
	      
	      // Adding the player
	      printf("Adding the player.\n");
	      Node * newnode = (Node*) malloc(sizeof(Node)); // TODO: remember to free this

	      Player * newplayer = (Player*) malloc(sizeof(Player)); // TODO: remember to free this first
	      initialize(newplayer,mn->name,ntohl(mn->hp),ntohl(mn->exp),mn->x,mn->y);
	      newnode->datum = newplayer;
	      newnode->next = NULL;

	      addPlayer(newnode,others,tail);
	      printf("Adding the player.\n");

	      // Printing
	      on_move_notify(newplayer->name, newplayer->x, newplayer->y, newplayer->hp,newplayer->exp);

	    } else { // The guy is in the list
	      int oldv = isVisible(self->x,self->y,p->datum->x,p->datum->y);
	      int newv = isVisible(self->x,self->y,mn->x,mn->y);

	      if(oldv || newv){
		initialize(p->datum,mn->name,ntohl(mn->hp),ntohl(mn->exp),mn->x,mn->y);
		on_move_notify(p->datum->name, p->datum->x, p->datum->y, p->datum->hp,p->datum->exp);
	      }

	    } // End inner if
	  }
	  
	} else if(hdr->msgtype == ATTACK_NOTIFY){
	  struct attack_notify * an = (struct attack_notify *)payload_c;
	  Node * att_node = findPlayer(an->attacker_name, others);
	  Node * vic_node = findPlayer(an->victim_name, others);

	  Player * att = att_node->datum;
	  Player * vic = vic_node->datum;

	  int updated_hp = ntohl(an->hp);
	  char damage = an->damage;
	  // Check in the case where they are null

	  // Check the visibility
	  int attVisible = isVisible(self->x,self->y,att->x,att->y);
	  int vicVisible = isVisible(self->x,self->y,vic->x,vic->y);

	  if (attVisible && vicVisible){
	    on_attack_notify(att->name,vic->name,damage,updated_hp);
	  }
	  
	  

	} else if(hdr->msgtype == SPEAK_NOTIFY){ // SPEAK_NOTIFY
	  struct speak_notify* sreply = (struct speak_notify*) payload_c;
	  unsigned char * broadcaster = sreply->broadcaster;

	  char * start = ((char*)payload_c)+10;

	  // null terminated & no longer than 255
	  if(!check_player_message(start)){ 		//TODO: need to check this in the send too!!
	    printf("! Invalid format\n");
	    continue;
	  }
		
	  printf("%s: %s\n",broadcaster,start);
	} else if(hdr->msgtype == LOGOUT_NOTIFY){
	  // Take the the player out of the database

	  struct logout_reply * loreply = (struct logout_reply *) payload_c;
	  removePlayer(loreply->name, others);
	  printf("Player %s has left the tiny world of warcraft.\n",loreply->name);


	} else if(hdr->msgtype == INVALID_STATE){

	  struct invalid_state * is = (struct invalid_state *) payload_c;
	  on_invalid_state(is->error_code);

	} // end of else if for hdr->msgtype

      }
    }
  }
}
