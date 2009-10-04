#include <stdio.h>

struct header {
  unsigned char version;
  unsigned short len;
  unsigned char msgtype;
  unsigned int payload[3];
};

struct login_reply {
  unsigned char version;
  unsigned char l;
  unsigned char len;
  unsigned char msgtype;
  unsigned char error_code;
  unsigned int hp;
  unsigned int exp;
  unsigned char x;
  unsigned char y;
  unsigned char padding;
}_attribute_((packed));

union {
  uint16_t number;
  uint8_t bytes[2];
} test;

int main(){
  test2();
}

int test2(){
  test.number = 0x0A0B;
  print("%02x%02x\n", test.bytes[0], test.bytes[1]);
}

int test1(){
  unsigned char r[32] = 0x04001002000000008b00000000232700;
  struct login_reply * lreply = (struct login_reply *) r;
  /* printf("error code: %d\n", lreply->error_code); */
/*   printf("hp: %d\n", lreply->hp); */
/*   printf("exp: %d\n", lreply->exp); */
/*   printf("x: %d\n", lreply->x); */
/*   printf("y: %d\n", lreply->y); */
}


int test(){
  struct header *hdr = (struct header *) malloc(sizeof(int));
  char * name = "phuc";

  hdr->version = 0x01;
  hdr->len = 0x0010;
  hdr->msgtype = 0x01;

  hdr->payload[0] = 0x0;
  hdr->payload[1] = 0x0;
  hdr->payload[2] = 0x0;
  

  int i;
  for(i = 0; i < strlen(name); i++){
    int messageIndex = i/4;
    int shiftAmount = 8*(3- i % 4);

    hdr->payload[messageIndex] += name[i] << shiftAmount;
  }

  printf("version: %d\n", hdr->version);
  printf("length: %d\n", hdr->len);
  printf("msgtype: %d\n", hdr->msgtype);

  unsigned char *msg = (unsigned char*) hdr;

  printf("len of msg:%d.\n", strlen(msg));

  for (i=0; i<4; i++){
    printf("%02x ", msg[i]);
  }
  printf("\n");

  for (i=0; i<4; i++){
    printf("%d ", msg[i]);
  }
  printf("\n");


  struct header * h2 = (struct header *) msg;

  printf("version: %d\n", h2->version);
  printf("length: %d\n", h2->len);
  printf("msgtype: %d\n", h2->msgtype);
  printf("payload[0]: %x\n", h2->payload[0]);
  printf("payload[1]: %x\n", h2->payload[1]);
}
