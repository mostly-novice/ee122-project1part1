#include <stdio.h>
#include <stdlib.h>
#include "constants.h"



//Helper: handle the various task in clients

int handlelogin(char* name,int sock){
  int i = 0;


  printf("handlelogin - name: %s\n", name);
  printf("client - sock: %d\n", sock);

  // Header
  struct header *hdr = (struct header *) malloc(sizeof(int)); // remember to free this
  struct login_request * payload = (struct login_request *) malloc(sizeof(int)*3);

  hdr->version = 0x04;
  hdr->len = 0x0010;
  hdr->msgtype = 0x1;

  strcpy(payload->name,name);

  unsigned char * payload_c = (unsigned char*) payload;

  int j;
  /* for(j = 0; j < 12; j++){ */
/*     printf("%02x ", payload_c[j]); */
/*   } */

  printf("\n");

  unsigned char * header_c = (unsigned char *) hdr;

  for(j = 0; j < 4; j++){
    printf("%02x ", header_c[j]);
  }

  printf("\n");

  unsigned char tosent[16];

  for(j = 0; j < 16; j++){
    if(j<4){
      tosent[j] = header_c[j];
    } else {
      tosent[j] = payload_c[j-4];
    }
  }

  /* for(j = 0; j < 16; j++){ */
/*     printf("%02x ", tosent[j]); */
/*   } */

  /* printf("\n"); */

/*   printf("tosent: [%02x]\n", *tosent); */

  // Send a login message to the server
  int bytes_sent = send(sock, tosent,16,0);
  if (bytes_sent < 0) {
    perror("send failed");
  } else {
    printf("Send: %d bytes\n", bytes_sent);
  }

  free(hdr);
  free(payload);
  return 0;
}

int handlemove(char * direction, int sock){
  struct header *msg = (struct header *) malloc(sizeof(int)*2);
  struct move * move = (struct move *) malloc(sizeof(int));
  msg->version = 0x04;
  msg->len = 0x08;
  msg->msgtype = MOVE;
  msg->payload[0] = 0x0;

  unsigned char d;

  if(strcmp(direction,"north")==0){
    d = 0;
  }else if(strcmp(direction,"south")==0){
    d = 1;
  }else if(strcmp(direction,"east")==0){
    d = 2;
  }else if(strcmp(direction,"west")==0){
    d = 3;
  }

  move->direction = d;
  //  move->padding = 0x0;

  msg->payload = (char*) move;
  
  char* tosent = (char*) msg;
  
  int bytes_sent = send(sock,tosent,msg->len,0);
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

  // Logic

  // Check if the victim is out of sight
  // Search for the guy
  


  // Sending message
  struct header *msg = (struct header *) malloc(sizeof(int)*2);
  struct attack * attack = (struct move *) malloc(sizeof(int));

  msg->version = 0x04;
  msg->len = 0x08;
  msg->msgtype = ATTACK;
  msg->payload[0] = 0x0;
  msg->payload[1] = 0x0;
  msg->payload[2] = 0x0;

  // Copy the name
  strcpy(attack->victimname,victim);

  // Cast the payload to char*
  msg->payload = (char*) attack;

  // Cast the message to tosent
  char * tosent = (char*) msg;

  // Sending
  int bytes_sent = send(sock,tosent,msg->len,0);
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
  int total_length = strlen(m);

  // Sending message
  struct header *msg = (struct header *) malloc(sizeof(int)*2);
  struct speak * speak = (struct speak *) malloc(sizeof(int));

  msg->version = 0x04;
  msg->len = 0x08;
  msg->msgtype = SPEAK;

  // Copy the name
  strcpy(speak->msg,m);

  // Cast the payload to char*
  msg->payload = (char*) speak;

  // Cast the message to tosent
  char * tosent = (char*) msg;

  // Sending
  int bytes_sent = send(sock,tosent,msg->len,0);
  if (bytes_sent < 0){
    perror("send failed.\n");
    return -1;
  } else {
    printf("Send: %d bytes\n", bytes_sent);
  }

  free(msg);
  free(speak);
  return 0;
}

int handlelogout(char * name,int sock){
  struct header *msg = (struct header *) malloc(sizeof(int));
  msg->version = 0x04;
  msg->len = 0x08;
  msg->msgtype = LOGOUT;

  char * tosent = (char*) msg;

  // Sending the message;
  int bytes_sent = send(sock,tosent,msg->len,0);
  if (bytes_sent < 0){
    perror("send failed.\n");
    return -1;
  } else {
    printf("Send: %d bytes\n", bytes_sent);
  }
  free(msg);
}

