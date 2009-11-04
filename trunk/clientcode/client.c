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
  Node * prev = NULL;
  Node * curr = list->head ;
  Node * temp;
	
  /* check for empty list */
  if(!curr){ printf("The list is empty"); return 0; }

  /* check if datum is in head of list */
  if(strcmp(list->head->datum->name,name)==0){
    if(strcmp(list->tail->datum->name,name)==0){ // if tail == head, this is the only guy
      free(list->head->datum);
      free(list->head);

      list->head = NULL;
      list->tail = NULL;
      return 1;
    }

    curr = list->head;
    free(curr->datum);
    free(curr);
    list->head = list->head->next;
    return 1;
  }

  while( curr->next ){
    prev = curr;
    curr = curr->next;
    if(strcmp( curr->datum->name,name )==0){
      if(strcmp(list->tail->datum->name,name)==0){
	list->tail = prev;}
      prev->next = curr->next;
      free(curr->datum);
      free(curr);
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
  int udpsock;
  int tcpsock;
  int serverIP;
  int serverPort;
  int trackerIP;
  int trackerPort;

  int done = 0;
  int status;
  int udpdone = 0;
  int fdmax = 0;

  // Select
  fd_set readfds;
  fd_set writefds;

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));

  if(argc < 4){ printf("Usage: ./client -s <tracker IP address> -p <tracker port>"); exit(0);}

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
      printf("Usage: ./client -s <tracker IP address> -p <tracker port>");
      return 0;
    }
  }
  
  trackerIP = svalue;
  trackerPort = atoi(pvalue);

  tcpsock = socket(AF_INET, SOCK_STREAM, 0);
  udpsock = socket(AF_INET, SOCK_DGRAM, 0);
  if(udpsock < 0 || tcp < 0){
    perror("socket() failed\n");
    abort();
  }

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(trackerIP);
  sin.sin_port = htons(trackerPort);

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
    FD_SET(udpsock, &readfds);
    FD_SET(tcpsock, &readfds);

    if(udpsock>fdmax) fdmax = udpsock;
    if(tcpsock>fdmax) fdmax = tcpsock;
      
    if (select(fdmax+1,&readfds,NULL,NULL,NULL) == -1){
      perror("select");
      return 0;
    }
    
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
	if(!udpdone){
	  handleudplogin(name,udpsock,sin);
	}
	//int status = handlelogin(name,sock);

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
	free(buffer);
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
       * Handling data from UDP sock
       *
       */


    } else if (FD_ISSET(udpsock, &readfds)){
      unsigned char read_buffer[4096];
      int read_bytes = recvfrom(udpsock,read_buffer,sizeof(read_buffer),0,&sin,sizeof(sin));
      if (read_bytes < 0){
	perror("Handling data from UDP sock: read_bytes == -1");
      }

      char msgtype = read_buffer[0];
      if (msgtype == STORAGE_LOCATION_RESPONSE){
	struct storage_location_response * slr = (struct storage_location_response*) read_buffer;

	// Need to send the server the player_state_request
	struct sockaddr_in sin2;
	sin2.sin_family = AF_INET;
	sin2.sin_addr.s_addr = ntohl(slr->server_ip);
	sin2.sin_port = ntohs(slr->udpport);;
	sendpsrequest(udpsock,&sin2,currentID);



      } else if (msgtype == PLAYER_STATE_RESPONSE){
	struct player_state_response * psr = (struct player_state_response *) read_buffer;
	// Check whether this data is malformed
	
	// Initiate the player states
	initialize(self,psr->name,psr->hp,psr->exp,psr->x,psr->y);

	// Prepare the data to send the server_area_request
	sendsarequest(self->x,self->y,udpsock,&sin,currentID);




      } else if (msgtype == SERVER_AREA_RESPONSE){
	struct server_area_response * sares = (struct server_area_response *) read_buffer;
	
	// Check if the data is malformed

	// Set the value for serverIP and serverPort
	serverIP = ntohl(sares->server_ip);
	serverPort = ntohs(sares->tcpport);

	struct sockaddr_in tcpsin;
	tcpsin.sin_family = AF_INET;
	tcpsin.sin_addr.s_addr = serverIP;
	tcpsin.sin_port = serverPort;

	// Establish connection
	if(connect(tcpsock,(struct sockaddr *) &tcpsin, sizeof(tcpsin)) < 0){
	  perror("client - connect");
	  freePlayers(mylist);
	  free(self);
	  free(mylist);
	  
	  close(sock);
	  on_client_connect_failure();
	  abort();
	}

	// WE ARE GOOD TO GO

	
      } else if (msgtype == SAVE_STATE_RESPONSE){
	struct player_state_response * ssr = (struct player_state_response *) read_buffer;
	
	// Check for duplicates ID

	// Prepare the data to send the server_area_request
	sendsarequest(udpsock,&sin,currentID);
      }



      /*
       * Handling data from TCP sock
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
	    printMessage(header_c,4);

	    hdr = (struct header *) header_c;
	    check_malformed_header(hdr->version,hdr->len,hdr->msgtype);

	    // Move the pointers
	    char * temp = (char*) malloc(sizeof(char)*(buffer_size-HEADER_LENGTH));
	    memcpy(temp,buffer+4,buffer_size-4);
	    free(buffer);

	    buffer = temp;
	    buffer_size -= 4;
	    desire_length = ntohs(hdr->len)-4;
	    flag = PAYLOAD;

	  } else { // Payload
	    // Move the pointers
	    // Copy the payload
	    char payload_c[desire_length];
	    int j;
	    for(j = 0; j < desire_length; j++){ payload_c[j] = *(buffer+j);}
	    printMessage(payload_c,desire_length);

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
	      int i;
	      for(i = 0; i < strlen(payload_c); i++){
		sleep(5);
		process_speak_notify(payload_c[i]);
	      }
	    } else if(hdr->msgtype == LOGOUT_NOTIFY){
	      printf("Got a logout notify.\n");
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

