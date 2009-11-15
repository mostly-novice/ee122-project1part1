#include <stdio.h>
#include <stdlib.h>
#include "constants.h"

void printMessage(char * message, int len){
  int i;
  for(i = 0; i < len; i++){
    printf("%02x ", *(message+i));
  }
  printf("\n");
}

int handlelogout(char * name,int sock){
  struct header *hdr = (struct header *) malloc(sizeof(int));
  int j;

  hdr->version = 0x4;
  hdr->len = htons(0x4);
  hdr->msgtype = LOGOUT;

  char * header_c = (char*) hdr;
  unsigned char tosent[4];

  for(j = 0; j < 4; j++){
      tosent[j] = header_c[j];
  }

  // Sending the message;
  int bytes_sent = send(sock,tosent,4,0);
  if (bytes_sent < 0){
    perror("send failed.\n");
    free(hdr);
  }
  free(hdr);
}

int handleudplogin(char * name, int sock);
