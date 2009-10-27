// Helper to send messages.
unsigned char * createloginreply( unsigned char error_code,
				  unsigned int hp,
				  unsigned int exp,
				  unsigned char x,
				  unsigned char y,
				  char buffer[]
				  ){
  printf("Creating Login Reply\n");
  printf("x=%d\n",x);
  int i = 0;
  int j;
  struct header *hdr = (struct header *) malloc(sizeof(int));
  struct login_reply * payload = (struct login_reply *) malloc(sizeof(int)*3);

  hdr->version = 0x04;
  hdr->len = htons(0x0010);
  hdr->msgtype = LOGIN_REPLY;

  payload->error_code = error_code;
  payload->hp = htonl(hp);
  payload->exp = htonl(exp);
  payload->x = x;
  payload->y = y;
  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;


  for(j = 0; j < 16; j++){
    if(j<4) buffer[j] = header_c[j];
    else buffer[j] = payload_c[j-4];
  }

  // Send a login message to the server
  free(hdr);
  free(payload);
}

void createmovenotify(unsigned char* name,
				 unsigned int hp,
				 unsigned int exp,
				 unsigned char x,
				 unsigned char y,
				 char buffer[]){
  int i = 0;
  int j;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct move_notify * payload = (struct move_notify *) malloc(sizeof(int)*5); // remember to free this
  hdr->version = 0x04;
  hdr->len = htons(0x0018);
  hdr->msgtype = MOVE_NOTIFY;

  strcpy(payload->name,name);
  payload->hp = htonl(hp);
  payload->exp = htonl(exp);
  payload->x = x;
  payload->y = y;

  unsigned char * payload_c = (unsigned char*) payload;
  printf("Payload: ");
  printMessage(payload_c,20);
  unsigned char * header_c = (unsigned char *) hdr;	// flatten header struct

  for(j = 0; j < 24; j++){
    if(j<4) buffer[j] = header_c[j];
    else buffer[j] = payload_c[j-4];
  }

  printMessage(buffer,24);
  free(hdr);
  free(payload);
}

unsigned char * createattacknotify(unsigned char* attacker_name,
				   unsigned char* victim_name,
				   unsigned int hp,
				   unsigned char damage,
				   char buffer[]){
  int i = 0;
  int j;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct attack_notify * payload = (struct attack_notify *) malloc(sizeof(int)*7); // remember to free this
  hdr->version = 0x04;
  hdr->len = htons(0x0020);
  hdr->msgtype = 0x6;

  payload->hp = htonl(hp);
  payload->damage = damage;
  strcpy(payload->attacker_name,attacker_name);
  strcpy(payload->victim_name,victim_name);

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;

  for(j = 0; j < 32; j++){
    if(j<4) buffer[j] = header_c[j];
    else buffer[j] = payload_c[j-4];
  }

  // Send a login message to the server
  free(hdr);
  free(payload);
}


unsigned char * createspeaknotify(unsigned char* broadcaster,
				  char* m,
				  char buffer[]){
  int i = 0;
  int j;
  unsigned int payloadLength;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct speak_notify * payload = (struct speak_notify *) malloc (sizeof(int)*4); // remember to free this

  if((strlen(m)+1)%4 != 0)
	payloadLength = strlen(m) + 1 + 4 - ((strlen(m)+1)%4);
			  
  
  // *** IS THIS CORRECT?? ** //
  unsigned int totalMessageLength = payloadLength + 4;
  strcpy(payload->broadcaster,broadcaster);

  hdr->version = 0x04;
  hdr->len = htons(totalMessageLength);
  hdr->msgtype = 0x7;

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;


  for(j = 0; j < totalMessageLength; j++){
    if(j<4) buffer[j] = header_c[j];
    else buffer[j] = *(m+j-4);
  }

  // Send a login message to the server
  free(hdr);
}

unsigned char * createlogoutnotify(unsigned char* name, char buffer[]){
  int i = 0;
  int j;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct logout_reply * payload = (struct logout_reply *) malloc(sizeof(int)*3); // remember to free this
  hdr->version = 0x04;
  hdr->len = htons(0x0010);
  hdr->msgtype = 0x0a;
  strcpy(payload->name,name);
  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;

  for(j = 0; j < 16; j++){
    if(j<4) buffer[j] = header_c[j];
    else buffer[j] = payload_c[j-4];
  }

  free(hdr);
  free(payload);
}

void createinvalidstate(unsigned char error_code, char buffer[]){
  int i = 0;
  int j;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct invalid_state * payload = (struct invalid_state *) malloc(sizeof(int)); // remember to free this
  hdr->version = 0x04;
  hdr->len = htons(0x0008);
  hdr->msgtype = 0x1;

  payload->error_code = error_code;

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;

  for(j = 0; j < 8; j++){
    if(j<4) buffer[j] = header_c[j];
    else buffer[j] = payload_c[j-4];
  }

  free(hdr);
  free(payload);
}

int broadcast(fd_set master, int listener, int sock, int fdmax, unsigned char * tosent,int expected){
  int i;
  printf("Broadcasting\n");
  printf("fdmax=%d\n",fdmax);
  printf("listener = %d\n", listener);
  printMessage(tosent,24);
  for(i=0; i<fdmax+1;i++){
    printf("Is %d in the master set:%d\n",i,FD_ISSET(i,&master));
    if (FD_ISSET(i,&master) && i != listener){
      printf("Sending to sock:%d\n",i);
      int bytes_sent = send(i, tosent,expected,0);
      if (bytes_sent < 0) perror("send failed");
    }
  }
}

/*
 * sock is the socket you want to send to
 * tosent is the packet you want to send
 * expected: is the size you expected
 */
int unicast(int sock, unsigned char * tosent, int expected){
  printf("Unicast to sock:%d\n", sock);
  printf("Expected:%d\n", expected);
  printMessage(tosent,16);
  int bytes_sent = send(sock, tosent,expected,0);
  if (bytes_sent < 0){
    perror("send failed");
    abort();
  } else {
    printf("Success\n");
  }
}
