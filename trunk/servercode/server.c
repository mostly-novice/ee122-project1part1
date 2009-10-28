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
  fd_set login;
  int fdmax;
  
  char ** fdnamemap = malloc(sizeof(*fdnamemap)*20);
  int k;
  for(k=0; k<22; ++k ){
    fdnamemap[k] = NULL;
  }


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
  FD_ZERO(&login);

  FD_SET(listener,&master);

  fdmax = listener;

  while(1){ // main accept() log
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
	      FD_CLR(i,&login);
	      Player * player = findPlayer(fdnamemap[i],mylist);
	      if(player){
		removePlayer(fdnamemap[i],mylist);
		FILE *file = fopen(fdnamemap[i],"w+");
		fprintf(file,"%d %d %d %d",player->hp,player->exp,player->x,player->y);
		fclose(file);
		
		unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
		createlogoutnotify(fdnamemap[i],lntosent);
		broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
		FD_CLR(i,&master);
		// Clean up the buffer
		fdnamemap[i] = NULL;
	      }
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
		char payload_c[desire_length];
		int j;
		for(j = 0; j < desire_length; j++){ payload_c[j] = *(buffer+j);}
		printf("Received: ");
		printMessage(hdr,4);
		printMessage(payload_c,ntohs(hdr->len));
		if(hdr->msgtype == LOGIN_REQUEST){ // LOGIN REQUEST

		  if (FD_ISSET(i,&login)){
		    // Send invalid state with error code = 1
		    unsigned char ivstate[8];
		    createinvalidstate(1,ivstate);
		    int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
		    if (bytes_sent < 0){
		      perror("send failed");
		      abort();
		    }


		  } else { // If he is not logged in
		    struct login_request * lr = (struct login_request *) payload_c;
		    if (isnameinmap(lr->name,fdnamemap)){ // If the name is already used
		      // Send invalid state with error code = 1;
		      unsigned char ivstate[8];
		      createinvalidstate(1,ivstate);
		      int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
		      if (bytes_sent < 0){
			perror("send failed");
			abort();
		      }
		    } else {
		      FD_SET(i,&login); // Log him in

		      Player * newplayer = process_login_request(i,fdmax,login,lr->name,mylist);
		      
		      if (!fdnamemap[i]){ fdnamemap[i] = malloc(sizeof(char)*11);}
		      strcpy(fdnamemap[i],lr->name);
		      strcpy(newplayer->name,fdnamemap[i]);
		      Node * node = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
		      node->datum = newplayer;
		      node->next = NULL;
		      if(mylist->head == NULL){
			mylist->head = node;
			mylist->tail = node;
		      }else {
			mylist->tail->next = node;
			mylist->tail = node;
		      }
		    }
		    printMap(fdnamemap);
		  }

		} else if(hdr->msgtype == MOVE){ // MOVE
		  if (!FD_ISSET(i,&login)){ // if not login,
		    // Send invalid state
		    unsigned char ivstate[INVALID_STATE_SIZE];
		    createinvalidstate(0,ivstate);
		    int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
		    if (bytes_sent < 0){
		      perror("send failed");
		      abort();
		    }


		  } else { // If logged in, good to proceed
		    struct move * m = (struct move *) payload_c;
		    int direction = m->direction;
		    Player * player = findPlayer(fdnamemap[i],mylist);
		    if(player){
		      stats(player);
		      if(direction==NORTH){
			player->y += 3;
			player->y = (100+player->y) % 100;
		      }else if(direction==SOUTH){
			player->y -= 3;
			player->y = (100+player->y) % 100;
		      }else if(direction==WEST){
			player->x -= 3;
			player->x = (100+player->x) % 100;
		      }else if(direction==EAST){
			player->x += 3;
			player->x = (100+player->x) % 100;
		      }
		    
		      unsigned char mntosent[MOVE_NOTIFY_SIZE];
		      createmovenotify(fdnamemap[i],
				       player->hp,
				       player->exp,
				       player->x,
				       player->y,
				       mntosent);
		      printMessage(mntosent,MOVE_NOTIFY_SIZE);
		      broadcast(login,i,fdmax,mntosent,MOVE_NOTIFY_SIZE);
		    }
		  }


		} else if(hdr->msgtype == ATTACK){ // ATTACK
		  struct attack * attackPayload = (struct attack *) payload_c;
		  char * attacker = fdnamemap[i];
		  char * victim = attackPayload->victimname;
		  process_attack(i,
				 fdmax,
				 login,
				 attacker,
				 victim,
				 mylist);
		} else if(hdr->msgtype == SPEAK){ // SPEAK
		  struct speak * speakPayload = (struct speak *) payload_c;
		  int msglen = strlen(payload_c)+1+10+HEADER_LENGTH;
		  int totallen;
		  if((msglen+1)%4){
		    totallen = msglen + (4 - msglen%4);
		  } else {
		    totallen = msglen;
		  }
		  unsigned char spktosent[totallen];
		  createspeaknotify(fdnamemap[i],payload_c,totallen,spktosent);
		  printMessage(spktosent,totallen);
		  broadcast(login,i,fdmax,spktosent,totallen);

		} else if(hdr->msgtype == LOGOUT){ 
		  Player * player = findPlayer(fdnamemap[i],mylist);
		  if(player){
		    removePlayer(fdnamemap[i],mylist);
		    
		    FILE *file = fopen(fdnamemap[i],"w+");
		    fprintf(file,"%d %d %d %d",player->hp,player->exp,player->x,player->y);
		    fclose(file);
		    
		    unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
		    createlogoutnotify(fdnamemap[i],lntosent);
		    FD_CLR(i,&login);
		    broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
		    FD_CLR(i,&master);
		    close(i);
		    
		    // Clean up the buffer
		    free(fdnamemap[i]);
		    fdnamemap[i] = NULL;
		  }
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
  }
}
