#include <stdio.h>
#include <stdlib.h>
#include "constants.h"



//Helper: handle the various task in clients

int handlelogin(char* name,int sock){
  int i = 0;
  int j;

  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct login_request * payload = (struct login_request *) malloc(sizeof(int)*3);

  hdr->version = 0x04;
  hdr->len = htons(0x0010);
  hdr->msgtype = 0x1;

  strcpy(payload->name,name);

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;

  /* for(j = 0; j < 4; j++){ */
/*     printf("%02x ", header_c[j]); */
/*   } */

  unsigned char tosent[16];

  for(j = 0; j < 16; j++){
    if(j<4){
      tosent[j] = header_c[j];
    } else {
      tosent[j] = payload_c[j-4];
    }
  }

  // Send a login message to the server
  int bytes_sent = send(sock, tosent,16,0);
  if (bytes_sent < 0) {
    perror("send failed");
  } else {
    printf("Sent: %d bytes\n", bytes_sent);
  }

  free(hdr);
  free(payload);
  return 0;
}

int handlemove(char * direction, int sock){
  struct header *hdr = (struct header *) malloc(sizeof(int)); // Remember to free this
  struct move * payload = (struct move *) malloc(sizeof(int)); // Remember to free this
  int j;

  hdr->version = 0x4;
  hdr->len = htons(0x08);
  hdr->msgtype = MOVE;

  unsigned char d;

  if(strcmp(direction,"north")==0){
    payload->direction = 0x0;

  }else if(strcmp(direction,"south")==0){
    payload->direction = 0x1;

  }else if(strcmp(direction,"east")==0){
    payload->direction = 0x2;

  }else if(strcmp(direction,"west")==0){
    payload->direction = 0x3;

  } else {
    // Error
  }

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;

  unsigned char tosent[8];

  for(j = 0; j < 8; j++){
    if(j<4){
      tosent[j] = header_c[j];
    } else {
      tosent[j] = payload_c[j-4];
    }
  }
  
  int bytes_sent = send(sock,tosent,8,0);
  if (bytes_sent < 0){
    perror("send failed.\n");
  } else {
    printf("Send: %d bytes\n", bytes_sent);
  }
  return 0;
}


// Helper for attack
// Assuming that the input is valid
int handleattack(char * victim, int sock){
  int j = 0;

  // Must be null-terminated

  // Check for out of sight

  // Sending message
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct attack * payload = (struct attack *) malloc(sizeof(int)); // remember to free this

  hdr->version = 0x4;
  hdr->len = htons(0x08);
  hdr->msgtype = ATTACK;

  // Copy the name
  strcpy(payload->victimname,victim);

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;

  unsigned char tosent[16];

  for(j = 0; j < 16; j++){
    if(j<4){
      tosent[j] = header_c[j];
    } else {
      tosent[j] = payload_c[j-4];
    }
  }

  // Sending
  int bytes_sent = send(sock,tosent,16,0);
  if (bytes_sent < 0){
    perror("send failed.\n");
    return -1;
  } else {
    printf("Send: %d bytes\n", bytes_sent);
  }
  return 0;
}

// Speak
int handlespeak(char * m, int sock){
 // Header
  unsigned int payloadLenth = strlen(m) + 1 + 4 - ((strlen(m) % 4));
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct speak * payload = (struct speak *) malloc(sizeof(char)*payloadLenth);
  int j;


  unsigned int totalMessageLength = payloadLenth + 4;

  hdr->version = 0x4;
  hdr->len = htons(totalMessageLength);
  hdr->msgtype = SPEAK;

  strcpy(payload->name,m);

  unsigned char * payload_c = (unsigned char*) payload;
  unsigned char * header_c = (unsigned char *) hdr;

  /* for(j = 0; j < 4; j++){ */
/*     printf("%02x ", header_c[j]); */
/*   } */

  unsigned char tosent[totalMessageLength];

  for(j = 0; j < totalMessageLength; j++){
    if(j<4){
      tosent[j] = header_c[j];
    } else {
      tosent[j] = payload_c[j-4];
    }
  }

  // Send a speak message to the server
  int bytes_sent = send(sock, tosent,totalMessageLength,0);
  if (bytes_sent < 0) {
    perror("send failed");
  } else {
    printf("Sent: %d bytes\n", bytes_sent);
  }

  free(hdr);
  free(payload);
  return 0;
}

int handlelogout(char * name,int sock){
  struct header *hdr = (struct header *) malloc(sizeof(int));
  int j;

  hdr->version = 0x4;
  hdr->len = htons(0x4);
  hdr->msgtype = LOGOUT;

  char * tosent = (char*) hdr;

  printf("tosent: ");
  for(j = 0; j < 8; j++){
    printf("%02x ", tosent[j]);
  }
  printf("\n");

  // Sending the message;
  int bytes_sent = send(sock,tosent,4,0);
  if (bytes_sent < 0){
    perror("send failed.\n");
    return -1;
  } else {
    printf("Send: %d bytes\n", bytes_sent);
  }
  free(hdr);
}

