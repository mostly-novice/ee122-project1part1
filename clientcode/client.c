#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#include "header.h"
#include "messages.h"

#define MAX_MESSAGE_RECORD 50

#define STDIN 0
#define HEADER_LENGTH 4

// Flags
#define HEADER 0
#define PAYLOAD 1

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
  Node * curr;
  while(list->head)
    {
      curr = list->head;
      list->head = curr->next;
      free(curr->datum);
      free(curr);
    }

}

void initialize(Player * object,char * name, int hp, int exp, int x, int y){
  strcpy(object->name,name);
  object->hp = hp;
  object->exp = exp;
  object->x = x;
  object->y = y;
}

#include "aux.h"
#include "helper.h"
#include "payloadHelper.h"

int main(int argc, char* argv[]){

  // Model Variables
  Player * self = (Player *) malloc(sizeof(Player)); // Remember to free this
  LinkedList * mylist = (LinkedList *) malloc (sizeof(LinkedList));
  message_record ** mr_array = malloc(sizeof(*mr_array)*MAX_MESSAGE_RECORD);

  message_record * tobeack = (message_record *) malloc(sizeof(message_record));

  mylist->head = NULL;
  mylist->tail = NULL;

  Node * p;
  int isLogin = 0;
  char command[80];
  char arg[4000];

  struct timeval * tv;
  unsigned int attempt = 1;

  // Connection variables
  int udpsock;
  int tcpsock;

  int serverIP;
  int serverPort;   // This is a TCP Port

  int dbserverIP;   // Server to store the state
  int dbserverport; // This is a UDP Port

  int trackerIP;
  int trackerPort;  // This is a UDP Port

  int done = 0;
  int status;
  int udpdone = 0;
  int fdmax = 0;

  // Playable area
  int min_x;
  int max_x;
  int min_y;
  int max_y;

  // Select
  fd_set readfds;
  fd_set writefds;

  struct sockaddr_in trackersin,serversin,dbserversin,sendersin;

  sendersin.sin_family = AF_INET;

  message_record ** message_list = malloc(sizeof(*message_list)*50);

  srand(time(NULL));
  int currentID = rand()%100;

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
  if(udpsock < 0 || tcpsock < 0){
    perror("socket() failed\n");
    abort();
  }

  trackersin.sin_family = AF_INET;
  trackersin.sin_addr.s_addr = inet_addr(trackerIP);
  trackersin.sin_port = htons(trackerPort);

  show_prompt();

  char * buffer;
  int buffer_size = 0;
  int flag = 0;
  int desire_length = 4;
  unsigned char header_c[HEADER_LENGTH];
  struct header * hdr;

  tv = NULL;

  while(1){
    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    FD_SET(udpsock, &readfds);
    FD_SET(tcpsock, &readfds);
    if(udpsock>fdmax) fdmax = udpsock;
    if(tcpsock>fdmax) fdmax = tcpsock;
    if (select(fdmax+1,&readfds,NULL,NULL,tv) == -1){
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
	if(!udpdone){ // If we don't have a connection yet
	  tobeack->id = currentID;
	  tobeack->ip = trackersin.sin_addr.s_addr;

	  // Send a storage_location_request
	  attempt = 1;
	  on_udp_attempt(attempt);

	  // send_to
	  sendslrequest(self->name,udpsock,&trackersin,currentID,tobeack);
	  currentID++;

	  // Set time out fo this attempt
	  tv = (struct timeval*) malloc(sizeof(struct timeval));
	  tv->tv_sec = 0;
	  tv->tv_usec = pow(2,attempt-1)*100000;

	} else { // If no longer need UDP
	  int status = handlelogin(name,udpsock);
	}

      } else if(strcmp(command,"move") == 0){ // MOVE
	char* direction = arg;
	if (strcmp(arg,"")==0x40){
	  printf("! Invalid syntax\n");
	  show_prompt();
	  continue;
	}
	unsigned char d;

	if(strcmp(direction,"north")==0){       d = NORTH; self->y -= 3; self->y = (100+self->y) % 100;
	}else if(strcmp(direction,"south")==0){ d = SOUTH; self->y += 3; self->y = (100+self->y) % 100;;
	}else if(strcmp(direction,"east")==0){  d = EAST;  self->x += 3; self->x = (100+self->x) % 100;;
	}else if(strcmp(direction,"west")==0){  d = WEST;  self->x -= 3; self->x = (100+self->x) % 100;;
	} else {
	  printf("! Invalid direction: %s\n", arg);
	  continue;
	}

	if(self->x > max_x || self->x < min_x || self->y > max_y || self->y < min_y){
	  if (handlelogout(self->name,tcpsock) < 0){ perror("handlelogout");}
	  freePlayers(mylist); // Free every player in the list
	  free(mylist); // Free the list
	  mylist = (LinkedList *) malloc (sizeof(LinkedList));
	  show_prompt();

	  // Contact the tracker
	  // Send SERVER_AREA_REQUEST
	  attempt = 1;
	  on_udp_attempt(attempt);

	  sendsarequest(self->x,self->y,udpsock,&trackersin,currentID,tobeack);
	  currentID++;

	  // Set the timeout
	  tv->tv_usec = pow(2,attempt-1)*100000;

	  close(tcpsock);
	  isLogin = 0; // FALSE
	  tcpsock = socket(AF_INET, SOCK_STREAM, 0);
	} else {
	  int status = handlemove(d,tcpsock);
	}
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
	int status = handleattack(victimname,tcpsock);
      } else if(strcmp(command,"speak") == 0){

	char * m = arg;
	// Check the message
	if(!check_player_message(m)){ printf("! Invalid text message.\n"); show_prompt(); continue;}
	if(handlespeak(m,tcpsock)){ perror("handlespeak"); }
	
	
      } else if(strcmp(command,"logout") == 0){
	if(!isLogin){
	  show_prompt();
	  printf("You must login first.\n");
	  show_prompt();
	  continue;
	}

	// Sending the UDP Save State Request
	attempt = 1;
	on_udp_attempt(attempt);

	sendssrequest(self,udpsock,&dbserversin,currentID,tobeack);
	currentID++;

	// Reset time out
	tv->tv_usec = pow(2,attempt-1)*100000;

	// Stop listening from the user
	FD_CLR(STDIN,&readfds);

	// Free the data
	free(buffer);
	free(self);
	freePlayers(mylist); // Free every player in the list
	free(mylist); // Free the list

      } else if(strcmp(command,"whois") == 0){
	printPlayers(mylist);
      } else if(strcmp(command,"whoami")==0){
	if (isLogin)
	  stats(self);
	else printf("Not yet logged in.\n");
      } else {
	printf("Available command: login, move, attack, speak, logout.\n");
      }



      /*
       * Handling data from UDP sock
       *
       */


    } else if (FD_ISSET(udpsock, &readfds)){
      unsigned char read_buffer[4096];
      socklen_t addrLen = sizeof(sendersin);
      int read_bytes = recvfrom(udpsock,read_buffer,sizeof(read_buffer),0,(struct sockaddr *) &sendersin,&addrLen);
      if (read_bytes < 0){
	perror("Handling data from UDP sock: read_bytes == -1");
      }

      int ip = sendersin.sin_addr.s_addr;
      char msgtype = read_buffer[0];
      if(tobeack->ip!=ip || tobeack->id!=currentID-1){
	printf("This is not good.\n");
	//on_udp_malformed();
      } else {
	// Free the last tobeack
	if(tobeack->message) free(tobeack->message);
	tobeack->id = currentID;
	tobeack->ip = sendersin.sin_addr.s_addr;

	if (msgtype == STORAGE_LOCATION_RESPONSE){
	  struct storage_location_response * slr = (struct storage_location_response*) read_buffer;
	  on_loc_resp(msgtype,slr->server_ip,slr->udpport);

	  // TODO: Check to see if the source address is correct
	  if(NULL) on_invalid_udp_source();

	  // Need to send the server the player_state_request
	  dbserversin.sin_family = AF_INET;
	  dbserversin.sin_addr.s_addr = slr->server_ip;
	  dbserversin.sin_port = slr->udpport;

	  attempt = 1; // Reset the attempt
	  on_udp_attempt(attempt);
	  
	  sendpsrequest(self->name,udpsock,&dbserversin,currentID,tobeack);
	  currentID++;

	  tv->tv_usec = pow(2,attempt-1)*100000;

	} else if (msgtype == PLAYER_STATE_RESPONSE){
	  struct player_state_response * psr = (struct player_state_response *) read_buffer;
	  initialize(self,psr->name,ntohl(psr->hp),ntohl(psr->exp),psr->x,psr->y);
	  
	  // Reset the attempt
	  attempt = 1;
	  on_udp_attempt(attempt);

	  sendsarequest(self->x,self->y,udpsock,&trackersin,currentID,tobeack);
	  currentID++;

	  tv->tv_usec = pow(2,attempt-1)*100000;
	  
	} else if (msgtype == SERVER_AREA_RESPONSE){
	  struct server_area_response * sares = (struct server_area_response *) read_buffer;

	  // Set tv to be null so that we don't have to wait anymore
	  tv = NULL;
	  
	  // Check if the data is malformed
	  min_x = sares->min_x; max_x = sares->max_x; min_y = sares->min_y; max_y = sares->max_y;
	  
	  //on_receive_server_area_response(min_x,max_x,min_y,max_y);
	  
	  struct sockaddr_in tcpsin;
	  tcpsin.sin_family = AF_INET;
	  tcpsin.sin_addr.s_addr = sares->server_ip;
	  tcpsin.sin_port = sares->tcpport;

	  // Establish connection
	  if(connect(tcpsock,(struct sockaddr *) &tcpsin, sizeof(tcpsin)) < 0){
	    perror("client - connect returns -1");
	    freePlayers(mylist);
	    free(self);
	    free(mylist);
	  }
	  udpdone = 1;
	  int status = handlelogin(self->name,tcpsock);
	  
	} else if (msgtype == SAVE_STATE_RESPONSE){
	  // TODO: clean up the message history
	  // TODO: Clean up the player list

	  // Now official logging out
	  if (handlelogout(self->name,tcpsock) < 0){ perror("handlelogout");}
	  
	  break;
	} else {
	  //on_udp_malformed();
	  printf("UDP MALFORMED\n");
	}
      }


      /*
       * Handling data from TCP sock
       *
       */
    } else if (FD_ISSET(tcpsock, &readfds)){ // Receiving from sock
      // Block and wait for response from the server
      unsigned char read_buffer[4096];
      int expected_data_len = sizeof(read_buffer);
      unsigned char *p = (char*) read_buffer; // Introduce a new pointer
      int offset = 0;

      int read_bytes = recv(tcpsock,read_buffer,expected_data_len,0);
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
	    //check_malformed_header(hdr->version,hdr->len,hdr->msgtype);

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

	    if(hdr->msgtype == LOGIN_REPLY){ // LOGIN REPLY
	      if(isLogin) on_malformed_message_from_server();
	      if(process_login_reply(payload_c,self)==1){
		isLogin = 1;
	      }
	    } else if(hdr->msgtype == MOVE_NOTIFY){   process_move_notify(payload_c,self,mylist);
	    } else if(hdr->msgtype == ATTACK_NOTIFY){ process_attack_notify(payload_c,self,mylist);
	    } else if(hdr->msgtype == SPEAK_NOTIFY){  process_speak_notify(payload_c);
	    } else if(hdr->msgtype == LOGOUT_NOTIFY){ process_logout_notify(payload_c,mylist);
	    } else if(hdr->msgtype == INVALID_STATE){
	      process_invalid_state(payload_c);
	    } else {
	      on_malformed_message_from_server();
	    }

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
    } else {

      // Handling timeout event
      if (attempt == 4){
	// clean up and exit
	// Print out on_udp_fail
	
	printf("UDP FAILED. IF A NEW VERSION COMES OUT, REMEMBER TO FIX THIS!\n");
	exit(0);
      } else {
	attempt++;
	on_udp_attempt(attempt); // print out UDP attempt
	
	// Resend the attempt
	char * tosent = tobeack->message;
	if(tosent[0] == STORAGE_LOCATION_REQUEST){
	  sendto(udpsock,tosent,STORAGE_LOCATION_REQUEST_SIZE,0,(struct sockaddr*)&trackersin,sizeof(trackersin));
	  
	} else if(tosent[0] == PLAYER_STATE_REQUEST){
	  sendto(udpsock,tosent,PLAYER_STATE_REQUEST_SIZE,0,(struct sockaddr*)&dbserversin,sizeof(dbserversin));
	  
	} else if(tosent[0] == SERVER_AREA_REQUEST){
	  sendto(udpsock,tosent,SERVER_AREA_REQUEST_SIZE,0,(struct sockaddr*)&serversin,sizeof(serversin));
	  
	} else if(tosent[0] == SAVE_STATE_REQUEST){
	  sendto(udpsock,tosent,SAVE_STATE_REQUEST_SIZE,0,(struct sockaddr*)&dbserversin,sizeof(dbserversin));
	  
	} else {
	  fprintf(stderr,"THIS SHOULD NEVER HAPPEN.\n");
	}

	// Set the timeout
	tv->tv_usec = pow(2,attempt-1)*100000; // 100ms = 100,000usec
      }
    }
  } // End of while(1)
}
