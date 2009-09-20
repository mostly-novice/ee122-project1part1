enum messages {
  LOGIN_REQUEST = 1,
  LOGIN_REPLY
};

char * build_message(char version, char* length, char msg_type, char* payload){
  char message[17];
  message[0] = version;
  message[1] = length[0];
  message[1] = length[1];
  message[16] = "\0";
  return message;
}

int main(){
  //char * message = build_message(0x4,0x10,LOGIN_REQUEST,0x1);
  int message[4];
  int i = 0;
  char * name = "kevintony";
  printf("The size of name is: %d\n", strlen(name));
  message[0] = LOGIN_REQUEST;
  message[0] += (0x10 << 4);
  message[0] += (0x04 << 12);

  message[1] = 0;
  message[2] = 0;
  message[3] = 0;

  printf("Hello - The message2 is %x\n", message[1]);

  for(i = 0; i < strlen(name); i++){
    int messageIndex = i/4 + 1;
    int shiftAmount = 8*(3- i % 4);

    message[messageIndex] += name[i] << shiftAmount;
  }

  printf("Hello - The message is %x\n", message[0]);
  
  printf("Hello - The message2 is %x\n", message[1]);

  printf("Hello - The message3 is %x\n", message[2]);

  printf("Hello - The message4 is %x\n", message[3]);
}