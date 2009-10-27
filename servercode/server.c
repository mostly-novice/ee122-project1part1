// Server code
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
#include "messages.h"

#define STDIN 0
#define HEADER_LENGTH 4
#define DIR "users"

// Flags
#define HEADER 0
#define PAYLOAD 1

#define MAX_CONNECTION 20

// Debugging variables
int mc; // malloc counter
int fc; // free counter

void printMessage(char * message, int len){
  int i;
  for(i = 0; i < len; i++){
    printf("%02x ", *(message+i));
  }
  printf("\n");
}
#include "model.h"
#include "processHelper.h"
#include "aux.h"
int main(int argc, char* argv[]){

  // Model Variables
  LinkedList * mylist = (LinkedList *) malloc (sizeof(LinkedList));
  mylist->head = NULL;
  mylist->tail = NULL;
  struct sockaddr_in client_sin;
  Node * p;
  int isLogin = 0;
  char command[80];
  char arg[4000];

  // Connection variables
  // Keep track of the list of sockets
  int listener;
  int myport;
  int done = 0;
  int status;

  // Select
  fd_set readfds;
  fd_set master; // master fd
  int fdmax;
  
  char * fdnamemap[20];

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));

  printf("%d\n",argc);
  if(argc != 3){ printf("Usage: ./server -p <server port>");  exit(0);}

  // Initilizations
  int c;
  char* pvalue=NULL;

  mkdir(DIR);
  chdir(DIR);

  myport = atoi(argv[2]);

  if(setvbuf(stdout,NULL,_IONBF,NULL) != 0){
      perror('setvbuf');
  }

  listener = socket(AF_INET, SOCK_STREAM, 0);
  if(listener < 0){
    perror("socket() failed\n");
    abort();
  } else {
    printf("Listenning sock is ready. Sock: %d\n",listener);
  }

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(myport);


  struct timeval tv;

  int optval = 1;
  if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) < 0){
    perror("Reuse failed");
    abort(); // TODO: check if this is the appropriate behavior.
  }

  if (bind(listener,(struct sockaddr *) &sin, sizeof(sin)) < 0){
    perror("Bind failed");
    abort();
  }

  if (listen(listener,MAX_CONNECTION)){
    perror("listen");
    abort();
  }

  FD_ZERO(&master);
  FD_ZERO(&readfds);

  FD_SET(listener,&master);

  fdmax = listener;

  printf("server: waiting for connections...\n");
  while(1){ // main accept() log
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    readfds = master; // copy it
    if (select(fdmax+1,&readfds,NULL,NULL,NULL) == -1){
      perror("select");
      exit(-1);
    }

    // run through the existing connections looking for data to read
    int i;
    for(i=0; i<= fdmax; i++){
      if (FD_ISSET(i,&readfds)){
	if (i==listener){ // NEW CONNECTION COMING IN
	  printf("Received a new connection\n");
	  // handle new connection
	  int addr_len = sizeof(client_sin);
	  int newfd = accept(listener,(struct sockaddr*) &client_sin,&addr_len);
	  if (newfd < 0){
	    perror("accept failed");
	  } else {
	    FD_SET(newfd,&master);
	    if (newfd > fdmax) fdmax = newfd;
	    printf("New connection in socket %d\n", newfd);
	  }
	} else { // If someone has data
	  printf("Handle data from socket %d\n", i);

	  unsigned char read_buffer[4096];
	  int expected_data_len = sizeof(read_buffer);
	  unsigned char *p = (char*) read_buffer; // Introduce a new pointer
	  int offset = 0;

	  char * buffer;
	  int buffer_size = 0;
	  int flag = 0;
	  int desire_length = 4;
	  unsigned char header_c[HEADER_LENGTH];
	  struct header * hdr;

	  int read_bytes = recv(i,read_buffer,expected_data_len, 0);
	  if (read_bytes <= 0){
	    // got error or connection closed by client
	    if(read_bytes == 0){
	      // Connection closed
	      printf("Socket %d hung up\n",i);
	    } else {
	      perror("recv");
	    }
	    close(i); // bye!
	    FD_CLR(i,&master); // remove from the master set
	  } else {
	    // we got some data from a client
	    // Handling incoming data
	    buffer = realloc(buffer,buffer_size+read_bytes);
	    memcpy(buffer,read_buffer,read_bytes);
	    buffer_size += read_bytes;
	    
	    while (buffer_size >= desire_length){
	      if(flag == HEADER){
		printf("Processing header\n");
		// Copy the header
		int j;
		for(j = 0; j < HEADER_LENGTH; j++){ header_c[j] = *(buffer+j);}

		hdr = (struct header *) header_c;
		printf("Got the header\n");
		printMessage(read_buffer,16);

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
		char payload_c[desire_length];
		int j;
		for(j = 0; j < desire_length; j++){ payload_c[j] = *(buffer+j);}

		if(hdr->msgtype == LOGIN_REQUEST){ // LOGIN REQUEST
		  struct login_request * lr = (struct login_request *) payload_c;
		  int retval = process_login_request(listener,i,fdmax,master,lr->name,mylist);
		  char tostore[strlen(lr->name)];
		  strcpy(tostore,lr->name);
		  fdnamemap[i] = tostore;
		  printf("Map at i:%s\n",fdnamemap[i]);


		} else if(hdr->msgtype == MOVE){ // MOVE
		  printf("Got a move message\n");
		  struct move * m = (struct move *) payload_c;
		  char * name = getName(i,fdnamemap);
		  printf("Sender:%s\n", name);
		  process_move(listener,i,fdmax,master,name,m->direction,mylist);


		} else if(hdr->msgtype == ATTACK){ // ATTACK
		  printf("We got an attack");
		  struct attack * attackPayload = (struct attack *) payload_c;
		  char * attacker = getName(i,fdnamemap);
		  char * victim = attackPayload->victimname;
		  process_attack(listener,
				 i,
				 fdmax,
				 master,
				 attacker,
				 victim,
				 mylist);
		} else if(hdr->msgtype == SPEAK){ // SPEAK
		  printf("We got a speak");
		} else if(hdr->msgtype == LOGOUT){ 
		  printf("We got a logout");
		  char * name = getName(i,fdnamemap);
		  process_logout(listener,
				 i,
				 fdmax,
				 master,
				 name,
				 mylist);
		} else {
		  printf("We got nothing");
		}
		//else if(hdr->msgtype == INVALID_STATE){
		//} // End of processing reply

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
    printf("5 seconds... look ma' no blocking\n");
  }
}
