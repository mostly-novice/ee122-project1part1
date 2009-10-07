#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
  printf("People in the list:\n");
  for(p = list; p != NULL; p = p->next){
    stats(p->datum);
  }
  printf("---------------------\n");
}


Node * findPlayer(char * name, Node * list){
  Node * p;
  for(p = list; p != NULL; p = p->next){
    if (strcmp(p->datum->name,name)==0){
      return p;
    }
  }
  return NULL;
}

unsigned int removePlayer(char * name, Node *list,Node*tail)
{
    Node * cur, *temp, *prev;
    cur = list;
    temp = prev = NULL;

    /* check for empty list */
    if(!list){
        printf("The list is empty");
        return 0;
    }

    /* check head */
    if(strcmp(list->datum->name,name)==0)
    {
        printf("deleting the head!\n");
        cur = list;
        list = list->next;
    }

    /* check the rest */
    while(cur->next)
    {
        prev = cur;
        cur = cur->next;
        if(strcmp(cur->datum->name,name)==0)
        {
            prev->next = cur->next;
            free(cur->datum);
            free(cur);
        }
    }
    return 1; /* success */
}


void freePlayers(Node * list)
{
  // To free all the players after log out
  Node * cur;
  while(list->next)
    {
      cur = list;
      list = list->next;
      free(cur->datum);
      free(cur);
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
    on_client_connect_failure();
    exit(0);
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
	unsigned char d;

	if(strcmp(direction,"north")==0){       d = NORTH;
	}else if(strcmp(direction,"south")==0){ d = SOUTH;
	}else if(strcmp(direction,"east")==0){  d = EAST;
	}else if(strcmp(direction,"west")==0){  d = WEST;
	} else {
	  printf("! Invalid direction: %s", arg);
	  continue;
	}
	int status = handlemove(d,sock);


      } else if(strcmp(command,"attack") == 0){ // ATTACK
	char* victimname = arg;
	if(strcmp(victimname,self->name)== 0){
	  continue;
	}

	if(!check_player_name(victimname)){
	  printf("! Invalid name: %s\n", victimname);
	  continue;
	}

	// Search for the guy in the list
	Node * vic = findPlayer(victimname,others);

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

	if(handlespeak(m,sock)){
	  perror("handlespeak");
	}
	
	
      } else if(strcmp(command,"logout") == 0){
	if (handlelogout(self->name,sock) < 0){
	  perror("handlelogout");
	}

	free(self);
	fc++;
	freePlayers(others);
	
	if(close(sock) < 0){
	  perror("logout - close");
	  break;
	}

	printStat();
	break;

      } else if(strcmp(command,"whois") == 0){
	printPlayers(others);

      } else {
	printf("Unrecognized command.\n");
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
      if (read_bytes == 0){
	on_disconnection_from_server();
	exit(0);
      }
      
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
	      Node * node = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
	      mc++;

	      Player * newplayer = (Player*) malloc(sizeof(Player)); // TODO: remember to free this first
	      mc++;
	      initialize(newplayer,mn->name,ntohl(mn->hp),ntohl(mn->exp),mn->x,mn->y);
	      node->datum = newplayer;
	      node->next = NULL;


	      if(tail == NULL && others == NULL){ // First player
		tail = node;
		others = node;
	      } else {
		tail->next = node;
		tail = node;
	      }

	      // Printing
	      if (isVisible(self->x,self->y,newplayer->x,newplayer->y))
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
	  Player * att;
	  Player * vic;

	  if (strcmp(an->attacker_name,self->name)==0){
	    att = self;
	  } else {
	    Node * att_node = findPlayer(an->attacker_name, others);
	    att = att_node->datum;
	  }

	  if (strcmp(an->victim_name,self->name)==0){
	    vic = self;
	  } else {
	    Node * vic_node = findPlayer(an->victim_name, others);
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
	  printf("others: %x\n.", others);
	  if (!removePlayer(loreply->name, others,tail)){
	    perror("LOGOUT_NOTIFY - remove player");
	    exit(-1);
	  }
	  printf("others: %x\n.", others);
	  
	  printf("Is other NULL? %d.\n", others==NULL);
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
