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

typedef struct sr{
  unsigned int ip;
  unsigned short tcp_port;
  unsigned short udp_port;
  unsigned char min_x;
  unsigned char max_x;
  unsigned char min_y;
  unsigned char max_y;
} server_record;

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

int main(int argc, char* argv[]){
  int listener;
  int myport;
  int done = 0;
  int status;
  char * configpath;
  server_record ** sr_array = malloc(sizeof(*sr_array)*100);
  int server_count;

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

  printf("Configpath:%s\n",configpath);
  printf("myport:%d\n",myport);

  // Initialize the server record data structure
  server_count = initsr(sr_array,configpath);
  printf("Done with initializing\n");
  
  if(setvbuf(stdout,NULL,_IONBF,NULL) != 0){ perror('setvbuf');}

  listener = socket(AF_INET, SOCK_DGRAM, 0);
  if(listener < 0){
    perror("socket() failed\n"); abort();
  } else { printf("Listenning sock is ready. Sock: %d\n",listener);}
  
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
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
    //if (select(fdmax+1,&readfds,NULL,NULL,NULL) == -1){
    //  perror("select");
    //  exit(-1);
    //}
    // run through the existing connections looking for data to read
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
	    char msgtype = read_buffer[0];

	    if(msgtype==STORAGE_LOCATION_REQUEST){
	      struct storage_location_request * slr = (struct storage_location_request*) read_buffer;
	      // Find the server record

	      // TODO: Check the ID
	      char * name = slr->name;
	      int hashval = hash(name);
	      int srindex = hashval % server_count; // The index of "DB" server

	      // TODO: Check if name is malformed
	      server_record * sr = sr_array[srindex];
	      char slrespond[STORAGE_LOCATION_RESPONSE_SIZE];

	      createslrespond(sr,ntohl(slr->id),slrespond);

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
	      printMessage(read_buffer,SERVER_AREA_REQUEST_SIZE);
	      struct server_area_request * sareq = (struct server_area_request*) read_buffer;
	      // TODO: Check if the value in the package is valid
	      int server_id = findServer(sareq->x,server_count,sr_array);
	      server_record * sr = sr_array[server_id];

	      char sarespond[STORAGE_LOCATION_RESPONSE_SIZE];
	      memset(&sarespond,0,STORAGE_LOCATION_RESPONSE_SIZE);
	      createsarespond(sr,ntohl(sareq->id),sarespond);

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
