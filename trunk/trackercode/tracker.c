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
#define MSGTYPE 0
#define PAYLOAD 1

#define MAX_CONNECTION 30

typedef struct sr{
  char * ip;
  int tcp_port;
  int udp_port;
  char min_x;
  char max_x;
  char min_y;
  char max_y;
} server_record;

typedef struct buffer{
  int flag;
  int desire_length;
  int buffer_size;
  char * buffer;
} bufferdata;

void printMessage(char * message, int len){
  int i;
  for(i = 0; i < len; i++){
    printf("%02x ", *(message+i));
  }
  printf("\n");
}

// Hashing function
unsigned int hash(char*s){
  unsigned int hashval;
  for (hashval=0;*s!=0;s++){
    hashval += *s+31*hashval;
  }
  return hashval;
}


// Return the server count
int initsr(server_record ** sr_array, char * configpath){
  File * file = fopen(configpath,'r');
  char server_ip[30];
  int tcp_port;
  int udp_port;

  int count = 0;
  while(fscanf(file,"%s %d %d", server_ip, &tcp_port, &udp_port)){   
    server_record * newrecord = (server_record *) malloc(sizeof(server_record));
    newrecord->ip = (char*)malloc(sizeof(char)*(strlen(server_ip)+1));
    strcpy(newrecord->ip,server_ip);
    newrecord->tcp_port = tcp_port;
    newrecord->udp_port = udp_port;
    newrecord->min_y = 0;
    newrecord->max_y = 99;
    sr_array[count] = server_record;
    count++;
  }

  // NOTE: This is different than the spec.
  int i;
  for(i=0;i<count;i++){
    sr_array[count]->min_x = ceil(99/count)*i;
    sr_array[count]->max_x = ceil(99/count)*(i+1)-1;
  }

  // Closing the file
  fclose(file);
  return count;
}

int findServer(char x, int server_count){
  return x/(100/server_count);
}

int main(int argc, char* argv[]){
  struct sockaddr_in client_sin;
  Node * p;
  int listener;
  int myport;
  int done = 0;
  int status;
  char * configpath;
  server_record ** sr_array;
  int server_count;

  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));

  if(argc != 5){ printf("Usage: ./tracker -f <configuration file> -p port");  exit(0);}

  // Initilizations
  int c;
  char* pvalue=NULL;

  // TODO: Implemnting the getopt

  configpath = argv[2];
  myport = atoi(argv[4]);

  // Initialize the server record data structure
  server_count = initsr(sr_array,configpath);
  
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

  FD_ZERO(&master);
  FD_ZERO(&readfds);
  FD_SET(listener,&master);
  fdmax = listener;

  while(1){ // main accept() lo
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
	} else { // If someone has data
	  unsigned char read_buffer[4096];
	  int read_bytes = recvfrom(i,read_buffer,expected_data_len, 0);
	  if (read_bytes < 0){
	    perror("Tracker - Recvfrom Failed - read_bytes return -1\n");
	    close(i); // bye!
	  } else {
	    char msgtype = read_buffer[0];

	    if(msgtype==STORAGE_LOCATION_REQUEST){
	      storage_location_request * slr = (storage_location_request*) read_buffer;
	      // Find the server record

	      // TODO: Check the ID
	      char * name = slr->name;
	      int hashval = hash(name);
	      int srindex = hashval % server_count;

	      // TODO: Check if name is malformed
	      server_record * sr = sr_array[srindex];
	      char slrespond[STORAGE_LOCATION_RESPONSE_SIZE];

	      createslrespond(server_record,ntohl(slr->id),slrepond);

	      int sent_byte = sendto(sock,
				     slrespond,
				     STORAGE_LOCATION_RESPONSE_SIZE,
				     0,
				     (struct sockaddr*)&sin,
				     sizeof(sin));

	      if(sent_bytes < 0){
		perror("Tracker - sendto failed: Handling storage location request");
		abort();
	      }

	    } else if(msgtype==SERVER_AREA_REQUEST){
	      server_area_request * sareq = (server_area_request*) read_buffer;
	      // TODO: Check if the value in the package is valid
	      int server_id = findServer(sareq->x);
	      server_record * sr = sr_array[server_id];

	      char sarespond[STORAGE_LOCATION_RESPONSE_SIZE];
	      createsarespond(sr,id,sarespond);

	      int sent_byte = sendto(sock,
				     sarespond,
				     SERVER_AREA_RESPONSE_SIZE,
				     0,
				     (struct sockaddr*)&sin,
				     sizeof(sin));

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
