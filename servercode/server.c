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

#include "model.h"

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
  // Keep track of the list of sockets
  int sock;
  int myport;
  int done = 0;
  int status;

  // Select
  fd_set readfds;
  fd_set writefds;

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));


  if(argc != 2){ printf("Usage: ./server -p <server port>");}

  // Initilizations
  int c;
  char* pvalue=NULL;

  while( (c=getopt( argc,argv,"p:"))!=-1){
    switch(c){
    case 'p':
      pvalue=optarg;
      break;
    default:
      printf("Usage: ./server -p <server port>");
      return 0;
    }
  }
  myort = atoi(pvalue);

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
    
    if (FD_ISSET(sock, &readfds)){ // Receiving from sock
      // Block and wait for response from the server
      unsigned char read_buffer[4096];
      int expected_data_len = sizeof(read_buffer);
      unsigned char *p = (char*) read_buffer; // Introduce a new pointer
      int offset = 0;

      int read_bytes = recv(sock,read_buffer,expected_data_len,0);
      if (read_bytes == 0){ // Disconnected from the server
	// Free memory
	freePlayers(mylist);
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

	    // Processing the payload
	    if(hdr->msgtype == LOGIN_REQUEST){ // LOGIN REQUEST
	    } else if(hdr->msgtype == MOVE){ // MOVE
	    } else if(hdr->msgtype == ATTACK){ // ATTACK
	    } else if(hdr->msgtype == SPEAK){ // SPEAK
	    } else if(hdr->msgtype == LOGOUT){ 
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
