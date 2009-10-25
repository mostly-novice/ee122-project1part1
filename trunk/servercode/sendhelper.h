// Helper to send messages.
unsigned char * createloginreply(int sock[], unsigned char error_code,
				 unsigned int hp, unsigned int exp,
				 unsigned char x,
				 unsigned y){
  int i = 0;
  int j;
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct login_reply * payload = (struct login_reply *) malloc(sizeof(int)*3); // remember to free this
  unsigned char tosent[16];

  hdr->version = 0x04;
  hdr->len = htons(0x0010);
  hdr->msgtype = LOGIN_REPLY;

  payload->error_code = error_code;
  payload->hp = hp;
  payload->exp = exp;
  payload->x = htons(hp);
  payload->hp = htons(hp);
  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;


  for(j = 0; j < 16; j++){
    if(j<4) tosent[j] = header_c[j];
    else tosent[j] = payload_c[j-4];
  }

  // Send a login message to the server
  free(hdr);
  free(payload);
  return tosent;
}

unsigned char * createmovenotify(unsigned char* name,
				 unsigned int hp,
				 unsigned int exp,
				 unsigned char x,
				 unsigned char y){
  int i = 0;
  int j;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct move_notify * payload = (struct move_notify *) malloc(sizeof(int)*3); // remember to free this
  hdr->version = 0x04;
  hdr->len = htons(0x0018);
  hdr->msgtype = 0x4;

  strcpy(payload->name,name);
  payload->hp = hp;
  payload->exp = exp;
  payload->x = x;
  payload->y = y;

  unsigned char * payload_c = (unsigned char*) payload;	// flatten payload struct
  unsigned char * header_c = (unsigned char *) hdr;	// flatten header struct
  unsigned char tosent[24];

  for(j = 0; j < 24; j++){
    if(j<4) tosent[j] = header_c[j];
    else tosent[j] = payload_c[j-4];
  }

  // Send move notify message to client
  free(hdr);
  free(payload);
  return tosent;
}

unsigned char * createattacknotify(unsigned char* attacker_name,
				   unsigned char* victim_name,
				   unsigned int hp,
				   unsigned int damage){
  int i = 0;
  int j;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct attack_notify * payload = (struct attack_notify *) malloc(sizeof(int)*7); // remember to free this
  hdr->version = 0x04;
  hdr->len = htons(0x0020);
  hdr->msgtype = 0x6;

  payload->hp = hp;
  payload->damage = damage;
  strcpy(payload->attacker_name,attacker_name);
  strcpy(payload->victim_name,victim_name);

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;
  unsigned char tosent[32];

  for(j = 0; j < 32; j++){
    if(j<4) tosent[j] = header_c[j];
    else tosent[j] = payload_c[j-4];
  }

  // Send a login message to the server
  free(hdr);
  free(payload);
  return tosent;
}


unsigned char * createspeaknotify(unsigned char* broadcaster,
				  char* m){
  int i = 0;
  int j;
  unsigned int payloadLength;
  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct speak_notify * payload = (struct speak_notify *) malloc (sizeof(int)*4); // remember to free this

  if((strlen(m)+1)%4 != 0)
	payloadLength = strlen(m) + 1 + 4 - ((strlen(m)+1)%4);
  else
	payloadLength = strlen(m) + 1 + 4;
			  
  
  // *** IS THIS CORRECT?? ** //
  unsigned int totalMessageLength = payloadLength + 4 + sizeof(int)*4;
  strcpy(payload->broadcaster,broadcaster);

  hdr->version = 0x04;
  hdr->len = htons(totalMessageLength);
  hdr->msgtype = 0x7;

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;
  unsigned char tosent[totalMessageLength];


  for(j = 0; j < totalMessageLength; j++){
    if(j<4) tosent[j] = header_c[j];
    else tosent[j] = *(m+j-4);
  }

  // Send a login message to the server
  free(hdr);
  return tosent;
}

unsigned char * sendlogoutnotify(unsigned char* name){
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
  unsigned char tosent[16];

  for(j = 0; j < 16; j++){
    if(j<4) tosent[j] = header_c[j];
    else tosent[j] = payload_c[j-4];
  }

  free(hdr);
  free(payload);
  return tosent;
}

unsigned char * sendinvalidstate(unsigned char error_code){
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
  unsigned char tosent[8];

  for(j = 0; j < 8; j++){
    if(j<4) tosent[j] = header_c[j];
    else tosent[j] = payload_c[j-4];
  }

  free(hdr);
  free(payload);
  return tosent;
}

int broadcast(int socklist[], int socklen, unsigned char * tosent,int expected){
  int i;
  for(i=0; i<socklen;i++){
    int bytes_sent = send(socklist[i], tosent,expected,0);
    if (bytes_sent < 0) perror("send failed");
  }
}

/*
 * sock is the socket you want to send to
 * tosent is the packet you want to send
 * expected: is the size you expected
 */
int unicast(int sock, unsigned char * tosent, int expected){
  int bytes_sent = send(sock, tosent,expected,0);
  if (bytes_sent < 0) perror("send failed");
}
