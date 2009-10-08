#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "header.h"
#include "helper.h"
#include "messages.h"
#include "aux.h"

#define STDIN 0
#define HEADER_LENGTH 4

// Debugging variables
int mc; // malloc counter
int fc; // free counter

typedef struct P{
  char name[10];
  int hp;
  int exp;
  int x;
  int y;
  int visible;
} Player;

struct list_el {
  Player * datum;
  struct list_el * next;
};

typedef struct list_el Node;

typedef struct ll{
  Node * head;
  Node * tail;
} LinkedList;

void stats(Player * p){
  fprintf(stdout, "%s: location=(%u,%u), HP=%u, EXP=%u\n",
          p->name, p->x, p->y, p->hp, p->exp);
}

void printPlayers(LinkedList * list){
  Node * n;
  printf("People in the list:\n");
  for(n = list->head; n != NULL; n = n->next){
    stats(n->datum);
  }
  printf("---------------------\n");
}

void updateSelfVision(int x, int y, LinkedList * list){
  Node * p;
  for(p = list->head; p != NULL; p = p->next){
    if (isVisible(x,y,p->datum->x,p->datum->y)){
      p->datum->visible = 1;
    }
  }
}

void checkSelfVision(int x, int y, LinkedList * list){
  Node * p;
  for(p = list->head; p != NULL; p = p->next){
    if (isVisible(x,y,p->datum->x,p->datum->y)){
      if (p->datum->visible==0){
	p->datum->visible = 1;
	on_move_notify(p->datum->name,
		       p->datum->x,
		       p->datum->y,
		       p->datum->hp,
		       p->datum->exp);
      }
    } else {
      p->datum->visible = 0;
    }
  }
}

Node * findPlayer(char * name, LinkedList * list){
  Node * p;
  for(p = list->head; p != NULL; p = p->next){
    if (strcmp(p->datum->name,name)==0){
      return p;
    }
  }
  return NULL;
}

unsigned int removePlayer(char * name, LinkedList * list)
{
  Node * prev = list->head;
  Node * curr = list->head ;
  Node * temp;
	
  /* check for empty list */
  if(!prev){ printf("The list is empty"); return 0; }

  /* check if datum is in head of list */
  if(strcmp(prev->datum->name,name)==0){
    if(strcmp(list->tail->datum->name,name)==0){ // if tail == head, this is the only guy
      free(prev->datum);
      fc++;
      free(prev);
      fc++;

      list->head = NULL;
      list->tail = NULL;
      return 1;
    }
    list->head = list->head->next;

    free(prev->datum);
    free(prev);
    return 1;
  }

  while( curr->next ){
    if(strcmp( curr->next->datum->name,name )==0){
      if(strcmp(list->tail->datum->name
		,name)==0){ // if item is the tail
	list->tail = curr;
      }

      temp = curr->next;
      curr->next = curr->next->next;

      free(temp->datum);
      free(temp);
    }
  }
  return 1;
}

Node * freePlayers(LinkedList * list)
{
  // To free all the players after log out
  Node * curr;
  while(list->head)
    {
      curr = list->head;
      list->head = curr->next;
      free(curr->datum);
      fc++;
      free(curr);
      fc++;
    }

}

void initialize(Player * object,char * name, int hp, int exp, int x, int y){
  strcpy(object->name,name);
  object->hp = hp;
  object->exp = exp;
  object->x = x;
  object->y = y;
}

// Printing out the relevant statistics
void printStat(){
  printf("\n");
  printf("Number of mallocs:%d\n",mc);
  printf("Number of frees:%d\n",fc);
  printf("\n");
}

int main(int argc, char* argv[]){

  // Model Variables
  Player * self = (Player *) malloc(sizeof(Player)); // Remember to free this
  mc++; // Debugging memory leak
  LinkedList * mylist = (LinkedList *) malloc (sizeof(LinkedList));
  mc++;
  mylist->head = NULL;
  mylist->tail = NULL;
  Node * p;
  int isLogin = 0;
  char command[80];
  char arg[4000];
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


  if(argc < 4){ printf("Usage: ./client -s <server IP address> -p <server port>");}

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
    perror("client - connect");
    freePlayers(mylist);
    free(self);
    free(mylist);

    close(sock);
    on_client_connect_failure();
    abort();
  }
  show_prompt();

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
      if(!readstdin(command,arg)){
	continue;
      }

      if (strcmp(command,"login") == 0){ // LOGIN
	char* name = arg; // TODO: Sanity check the input.'

	if(!check_player_name(name)){
	  printf("! Invalid syntax.\n");
	  show_prompt();
	  continue;
	}
	
	strcpy(self->name,arg);

	int status = handlelogin(name,sock);


      } else if(strcmp(command,"move") == 0){ // MOVE
	char* direction = arg;
	if (strcmp(arg,"")==0x40){
	  printf("! Invalid syntax\n");
	  show_prompt();
	  continue;
	}
	unsigned char d;

	if(strcmp(direction,"north")==0){       d = NORTH;
	}else if(strcmp(direction,"south")==0){ d = SOUTH;
	}else if(strcmp(direction,"east")==0){  d = EAST;
	}else if(strcmp(direction,"west")==0){  d = WEST;
	} else {
	  printf("! Invalid direction: %s\n", arg);
	  continue;
	}
	int status = handlemove(d,sock);
      } else if(strcmp(command,"attack") == 0){ // ATTACK
	char* victimname = arg;
	if(strcmp(victimname,self->name)== 0){
	  show_prompt();
	  continue;
	}

	if(!check_player_name(victimname)){
	  printf("! Invalid name: %s\n", victimname);
	  continue;
	}

	// Search for the guy in the list
	Node * vic = findPlayer(victimname,mylist);

	if (!vic){ // If it is NULL
	  on_not_visible();
	  continue;
	}

	Player * victim = vic->datum;

	int isvisible = isVisible(self->x,self->y,victim->x,victim->y);

	if(!isvisible){ on_not_visible(); continue;}

	int status = handleattack(victimname,sock);

      } else if(strcmp(command,"speak") == 0){

	char * m = arg;

	// Check the message
	if(!check_player_message(m)){ printf("! Invalid text message.\n"); show_prompt(); continue;}
	if(handlespeak(m,sock)){ perror("handlespeak"); }
	
	
      } else if(strcmp(command,"logout") == 0){
	if(!isLogin){
	  show_prompt();
	  printf("You must login first.\n");
	  show_prompt();
	  continue;
	}
	if (handlelogout(self->name,sock) < 0){ perror("handlelogout");}
	free(self);
	fc++;
	freePlayers(mylist); // Free every player in the list
	free(mylist); // Free the list
	fc++;
	
	show_prompt();
	on_disconnection_from_server();	
	break;
      } else if(strcmp(command,"whois") == 0){
	printPlayers(mylist);

      } else if(strcmp(command,"whoami")==0){
	stats(self);
      } else {
	printf("Available command: login, move, attack, speak, logout.\n");
      }

      show_prompt();






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
      if (read_bytes == 0){ // Disconnected from the server
	// Free memory
	freePlayers(mylist);
	free(self);
	free(mylist);
	on_disconnection_from_server();
	break;
      }
      
      if(read_bytes % 4 != 0){ // 32-bit aligned\
	// Free memory
	printf("! Message is not 32-bit aligned\n");
	freePlayers(mylist);
	free(self);
	free(mylist);
	on_malformed_message_from_server();
      }

      int j;
      
      unsigned char header_c[HEADER_LENGTH];
      while(offset < read_bytes){
	for(j = 0; j < HEADER_LENGTH; j++){header_c[j] = *(p+offset+j);}
	offset += HEADER_LENGTH;

	// Cast it to struct header
	struct header * hdr = (struct header *) header_c;

	if (hdr->version != 0x4){ // Check for version number
	  // Free memory
	  freePlayers(mylist);
	  free(self);
	  free(mylist);
	  on_malformed_message_from_server();
	}

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
	    } else { //TODO
	    }
	  }
	} else if(hdr->msgtype == MOVE_NOTIFY){
	  struct move_notify * mn = (struct move_notify *) payload_c;
	  Node *p;

	  check_malformed_stats(mn->x,mn->y,ntohl(mn->hp),ntohl(mn->exp));

	  if (strcmp(mn->name,self->name)==0){ // If the guy is self
	    self->hp = ntohl(mn->hp);
	    self->exp = ntohl(mn->exp);
	    self->x = mn->x;
	    self->y = mn->y;
	    on_move_notify(self->name, self->x, self->y, self->hp,self->exp);
	  } else { // The guy is someone else
	    p = findPlayer(mn->name,mylist);
	    if(p == NULL){ // Not in the list
	      Node * node = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
	      Player * newplayer = (Player*) malloc(sizeof(Player)); // TODO: remember to free this first
	      initialize(newplayer,mn->name,ntohl(mn->hp),ntohl(mn->exp),mn->x,mn->y);
	      node->datum = newplayer;
	      node->next = NULL;
	      if(mylist->tail == NULL && mylist->head == NULL){ // First player
		mylist->tail = node;
		mylist->head = node;
	      } else {
		mylist->tail->next = node;
		mylist->tail = node;
	      }
	    } else { // The guy is in the list
	      int oldv = isVisible(self->x,self->y,p->datum->x,p->datum->y);
	      int newv = isVisible(self->x,self->y,mn->x,mn->y);
	      p->datum->x = mn->x;
	      p->datum->y = mn->y;
	      if(oldv || newv){initialize(p->datum,mn->name,ntohl(mn->hp),ntohl(mn->exp),mn->x,mn->y);}
	    } // End inner if
	  }

	  checkSelfVision(self->x,self->y,mylist);

	  	  
	} else if(hdr->msgtype == ATTACK_NOTIFY){
	  struct attack_notify * an = (struct attack_notify *)payload_c;
	  Player * att;
	  Player * vic;

	  if (strcmp(an->attacker_name,self->name)==0){
	    att = self;
	  } else {
	    Node * att_node = findPlayer(an->attacker_name,mylist);
	    att = att_node->datum;
	  }

	  if (strcmp(an->victim_name,self->name)==0){
	    vic = self;
	  } else {
	    Node * vic_node = findPlayer(an->victim_name,mylist);
	    vic = vic_node->datum;
	  }

	  int updated_hp = ntohl(an->hp);
	  char damage = an->damage;

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
	  if(!check_player_message(start)){ printf("! Invalid format\n"); continue;}		
	  printf("%s: %s\n",broadcaster,start);
	  show_prompt();


	} else if(hdr->msgtype == LOGOUT_NOTIFY){
	  // Take the the player out of the database

	  struct logout_reply * loreply = (struct logout_reply *) payload_c;
	  if (!removePlayer(loreply->name, mylist)){
	    perror("LOGOUT_NOTIFY - remove player");
	    exit(-1);
	  }
	  
	  printf("Player %s has left the tiny world of warcraft.\n",loreply->name);
	  show_prompt();


	} else if(hdr->msgtype == INVALID_STATE){

	  struct invalid_state * is = (struct invalid_state *) payload_c;
	  on_invalid_state(is->error_code);

	} // end of else if for hdr->msgtype

      }
    }
  }
}
