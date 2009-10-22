#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>

#include "header.h"
#include "helper.h"
#include "messages.h"
#include "aux.h"

#define STDIN 0
#define HEADER_LENGTH 4

// Flags
#define HEADER 0
#define PAYLOAD 1

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

#include "payloadHelper.h"

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
  int c;
  char* svalue=NULL;
  char* pvalue=NULL;

  while( (c=getopt( argc,argv,"s:p:"))!=-1){
    switch(c){
    case 's':
      svalue=optarg;
      break;
    case 'p':
      pvalue=optarg;
      break;
    default:
      printf("Usage: ./client -s <server IP address> -p <server port>");
      return 0;
    }
  }
  
  serverIP = svalue;
  serverPort = atoi(pvalue);

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

  char * buffer;
  int buffer_size = 0;
  int flag = 0;
  int desire_length = 4;
  unsigned char header_c[HEADER_LENGTH];
  struct header * hdr;

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
	freePlayers(mylist); // Free every player in the list
	free(mylist); // Free the list
	show_prompt();
	on_disconnection_from_server();	
	break;
      } else if(strcmp(command,"whois") == 0){
	printPlayers(mylist);
      } else if(strcmp(command,"whoami")==0){
	if (isLogin)
	  stats(self);
	else printf("Not yet logged in.\n");
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
      unsigned char read_buffer[4096];
      int expected_data_len = sizeof(read_buffer);
      unsigned char *p = (char*) read_buffer; // Introduce a new pointer
      int offset = 0;

      int read_bytes = recv(sock,read_buffer,expected_data_len,0);
      if (read_bytes == 0){ // Disconnected from the server
	// Free memory
	freePlayers(mylist);
	free(self);
	free(mylist);
	on_disconnection_from_server();
	break;
      } else if (read_bytes < 0){
	perror("recv failed");
	return -1;
      } else {
	// Append the read_buffer to the buffer
	buffer = realloc(buffer,buffer_size+read_bytes);
	memcpy(buffer,read_buffer,read_bytes);
	buffer_size += read_bytes;

	while (buffer_size >= desire_length){
	  if(flag == HEADER){
	    // Copy the header
	    int j;
	    for(j = 0; j < HEADER_LENGTH; j++){ header_c[j] = *(buffer+j);}

	    hdr = (struct header *) header_c;
	    check_malformed_header(hdr->version,hdr->len,hdr->msgtype);
	    desire_length = ntohs(hdr->len)-4;
	    flag = PAYLOAD;

	    // Move the pointers
	    char * temp = (char*) malloc(sizeof(char)*(buffer_size-HEADER_LENGTH));
	    memcpy(temp,buffer+4,buffer_size-4);
	    free(buffer);
	    buffer = temp;
	    buffer_size -= 4;

	  } else { // Payload
	    // Move the pointers
	    // Copy the payload
	    char payload_c[desire_length];
	    int j;
	    for(j = 0; j < desire_length; j++){ payload_c[j] = *(buffer+j);}

	    if(hdr->msgtype == LOGIN_REPLY){ // LOGIN REPLY
	      if(isLogin) on_malformed_message_from_server();
	      if(process_login_reply(payload_c,self)==1){
		isLogin = 1;
	      }
	    } else if(hdr->msgtype == MOVE_NOTIFY){
	      process_move_notify(payload_c,self,mylist);
	    } else if(hdr->msgtype == ATTACK_NOTIFY){
	      process_attack_notify(payload_c,self,mylist);
	    } else if(hdr->msgtype == SPEAK_NOTIFY){ // SPEAK_NOTIFY
	      process_speak_notify(payload_c);
	    } else if(hdr->msgtype == LOGOUT_NOTIFY){
	      process_logout_notify(payload_c,mylist);
	    } else if(hdr->msgtype == INVALID_STATE){
	      process_invalid_state(payload_c);
	    } // End of processing reply

	    // Move the pointers
	    char * temp = (char*) malloc(sizeof(char)*(buffer_size-desire_length));
	    memcpy(temp,buffer+desire_length,buffer_size-desire_length);
	    free(buffer);
	    buffer = temp;
	    buffer_size -= desire_length;

	    desire_length = HEADER_LENGTH;
	    flag = HEADER;
	  } // end of handling payload
	} // End of while
      }
      
    }
  }
}

