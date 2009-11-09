// Tracker code
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

#define MAX_MESSAGE_RECORD 50

typedef struct sr{
  unsigned int ip;
  unsigned short tcp_port;
  unsigned short udp_port;
  unsigned char min_x;
  unsigned char max_x;
  unsigned char min_y;
  unsigned char max_y;
} server_record;

typedef struct mr{
  unsigned int ip;
  unsigned int id;
  unsigned char * message;
} message_record;

void printMessage(char * message, int len){
  int i;
  for(i = 0; i < len; i++){
    printf("%02x ", *(message+i));
  }
  printf("\n");
}

#include "sendhelper.h"
#include "aux.h"

// Hashing function
unsigned int hash(char*s){
  unsigned int hashval;
  for (hashval=0;*s!=0;s++){
    hashval += *s+31*hashval;
  }
  return hashval;
}

int findServer(char x, int server_count, server_record ** sr_array){
  int i;
  for(i=0;i<server_count;i++){
    if (sr_array[i]->max_x >= x && sr_array[i]->min_x <= x){
      return i;
    }
  }
  return -1;
}

int findDup(message_record ** mr_array,int id, int ip){
  int i;
  for(i = 0; i <MAX_MESSAGE_RECORD;i++){
    message_record * mr = mr_array[i];
    if(mr->ip == ip || mr->id == id){
      return i;
    }
  }
  return -1;
}

int main(int argc, char* argv[]){
  int listener;
  int myport;
  int done = 0;
  int status;
  char * configpath;
  server_record ** sr_array = malloc(sizeof(*sr_array)*100);
  int server_count;

  message_record ** mr_array = malloc(sizeof(*mr_array)*MAX_MESSAGE_RECORD);
  int oldest = 0;

  fd_set master;
  fd_set readfds;
  int fdmax;

  struct sockaddr_in sin;
  struct sockaddr_in clientaddr;
  int sin_len;
  memset(&sin, 0, sizeof(sin));
  

  if(argc != 5){ printf("Usage: ./tracker -f <configuration file> -p port");  exit(0);}

  // Initilizations
  int c;
  char* fvalue=NULL;
  char* pvalue=NULL;

  while( (c=getopt( argc,argv,"f:p:"))!=-1){
    switch(c){
    case 'f':
      fvalue=optarg;
      break;
    case 'p':
      pvalue=optarg;
      break;
    default:
      printf("Usage: ./client -s <tracker IP address> -p <tracker port>");
      return 0;
    }
  }
  
  configpath = fvalue;
  myport = atoi(pvalue);

  // Initialize the server record data structure
  server_count = initsr(sr_array,configpath);
  
  if(setvbuf(stdout,NULL,_IONBF,NULL) != 0){ perror('setvbuf');}

  listener = socket(AF_INET, SOCK_DGRAM, 0);
  if(listener < 0){
    perror("socket() failed\n"); abort();
  } else { printf("Listenning sock is ready. Sock: %d\n",listener);}
  
  sin.sin_family = AF_INET;
  sin.sin_port = htons(myport);

  int optval = 1;
  if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) < 0){
    perror("Reuse failed");
    abort(); // TODO: check if this is the appropriate behavior.
  }

  if (bind(listener,(struct sockaddr *) &sin, sizeof(sin)) < 0){
    perror("Bind failed");
    abort();
  }
  FD_ZERO(&readfds);
  FD_SET(listener,&readfds);
  fdmax = listener;
  
  int size = sizeof(clientaddr);

  while(1){ // main accept() lo
    int client_sin_len;
    int selVal = (select(fdmax+1,&readfds,NULL,NULL,NULL));
    printf("selval:%d",selVal);
    int i;
    for(i=0; i<= fdmax; i++){
      if (FD_ISSET(i,&readfds)){
	if (i!=listener){ // NEW CONNECTION COMING IN
	} else { // If someone has data
	  printf("We got a connection\n");
	  unsigned char read_buffer[4096];
	  int read_bytes = recvfrom(i,
				    read_buffer,
				    sizeof(read_buffer),
				    0,
				    (struct sockaddr *) &clientaddr,
				    &sin_len);

	  if (read_bytes < 0){
	    perror("Tracker - Recvfrom Failed - read_bytes return -1\n");
	    close(i); // bye!
	  } else {
	    // Recording the message for duplicate checking
	    fprintf(stdout,"RECEIVED: ");
	    printMessage(read_buffer,read_bytes);
	    char msgtype = read_buffer[0];
	    int ip = clientaddr.sin_addr.s_addr;

	    // Handling duplicates
	    //int dup = findDup(mr_array,read_buffer,ip); // return the index of the duplicate message
	    int dup = -1;
	    if(dup>=0){ // Found a duplicate
	      if(msgtype==STORAGE_LOCATION_REQUEST)
		sendto(i,mr_array[dup]->message,
		       STORAGE_LOCATION_RESPONSE_SIZE,
		       0,
		       (struct sockaddr*)&clientaddr,
		       sizeof(clientaddr));
	      else if (msgtype==STORAGE_LOCATION_REQUEST)
		sendto(i,mr_array[dup]->message,
		       SERVER_AREA_RESPONSE_SIZE,
		       0,
		       (struct sockaddr*)&clientaddr,
		       sizeof(clientaddr));
	    } else if(msgtype==STORAGE_LOCATION_REQUEST){
	      struct storage_location_request * slr = (struct storage_location_request*) read_buffer;

	      // TODO: Check the ID
	      char * name = slr->name;
	      int hashval = hash(name);
	      int srindex = hashval % server_count; // The index of "DB" server

	      // TODO: Check if name is malformed
	      server_record * sr = sr_array[srindex];
	      char slrespond[STORAGE_LOCATION_RESPONSE_SIZE];

	      createslrespond(sr,ntohl(slr->id),slrespond);

	      // Add this message to the message record array
	      printf("RECORDING THE STORAGE LOCATION REQUEST MESSAGE");
	      message_record * new_mr = (message_record*) malloc(sizeof(message_record));
	      new_mr->ip = clientaddr.sin_addr.s_addr;
	      new_mr->id = slr->id;
	      new_mr->message = (unsigned char*) malloc (sizeof(char)*STORAGE_LOCATION_RESPONSE_SIZE);
	      memcpy(new_mr->message,(char*)slrespond,STORAGE_LOCATION_RESPONSE_SIZE);
	      if (mr_array[oldest]){
		free(mr_array[oldest]->message);
		free(mr_array[oldest]);
	      }
	      mr_array[oldest] = new_mr;
	      oldest++;

	      printf("Sending STORAGE_LOCATION_RESPONSE.\n");
	      printMessage(slrespond,STORAGE_LOCATION_RESPONSE_SIZE);

	      int sent_bytes = sendto(i,
				      slrespond,
				      STORAGE_LOCATION_RESPONSE_SIZE,
				      0,
				      (struct sockaddr*)&clientaddr,
				      sizeof(clientaddr));

	      if(sent_bytes < 0){
		perror("Tracker - sendto failed: Handling storage location request");
		abort();
	      }

	    } else if(msgtype==SERVER_AREA_REQUEST){
	      struct server_area_request * sareq = (struct server_area_request*) read_buffer;

	      // TODO: Check if the value in the package is valid
	      int server_id = findServer(sareq->x,server_count,sr_array);
	      server_record * sr = sr_array[server_id];

	      char sarespond[SERVER_AREA_RESPONSE_SIZE];
	      memset(&sarespond,0,SERVER_AREA_RESPONSE_SIZE);
	      createsarespond(sr,ntohl(sareq->id),sarespond);

	      // Add this message to the message record array
	      printf("RECORDING THE SERVER MESSAGE");
	      message_record * new_mr = (message_record*) malloc(sizeof(message_record));
	      new_mr->ip = clientaddr.sin_addr.s_addr;
	      new_mr->id = sareq->id;
	      new_mr->message = (unsigned char*) malloc (sizeof(char)*SERVER_AREA_RESPONSE_SIZE);
	      memcpy(new_mr->message,(char*)sarespond,SERVER_AREA_RESPONSE_SIZE);
	      if (mr_array[oldest]){
		free(mr_array[oldest]->message);
		free(mr_array[oldest]);
	      }
	      mr_array[oldest] = new_mr;
	      oldest++;

	      printf("Sending SERVER_AREA_RESPONSE.\n");
	      printMessage(sarespond,SERVER_AREA_RESPONSE_SIZE);

	      int sent_bytes = sendto(i,
				      sarespond,
				      SERVER_AREA_RESPONSE_SIZE,
				      0,
				      (struct sockaddr*)&clientaddr,
				      sizeof(clientaddr));

	      if(sent_bytes < 0){
		perror("Tracker - sendto failed: Handling server area request");
		abort();
	      }
	    }
	  }
	} // end of checking the listener
      } // end of if fd_isset
    } // end of for
  }
}
