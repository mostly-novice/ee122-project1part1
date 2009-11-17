int isVisible(int x1, int y1, int x2, int y2){
  return (abs(x1 - x2)<=5 && abs(y1 - y2)<=5);
}

int readstdin(char * command, char * arg){
  char mystring[300];
  char * pch;

  setvbuf(stdin,NULL,_IONBF,1024 );
  fgets(mystring,300,stdin);
  if(strcmp(mystring,"\n")==0){
    return 0;
  }
  mystring[strlen(mystring)-1] = 0;
  pch = strtok(mystring," ");
  strcpy(command,pch);

  pch += strlen(pch);

  while(*(pch)==' ' || *(pch)=='\0'){
    pch++;
  }

  strcpy(arg, pch);
  return 1;
}

typedef struct M{
  unsigned int ip;
  unsigned int id;
  unsigned char * message;
} message_record;

int findDup(message_record ** mr_array,int id, int ip){
  int i;
  for(i = 0; i <MAX_MESSAGE_RECORD;i++){
    message_record * mr = mr_array[i];
    if(mr){
      if(mr->ip == ip && mr->id == id){
	return i;
      }
    }
  }
  return -1;
}

void freeMessageRecords(message_record ** mr_array){
  int i;
  for(i = 0; i <MAX_MESSAGE_RECORD;i++){
    message_record * mr = mr_array[i];
    if(mr){
      if(mr->message) free(mr->message); // free the message first
      free(mr);
    }
  }
  fprintf(stdout,"Freed all message records\n");

  free(mr_array);
  fprintf(stdout,"Free mr_array.\n");
}

enum {
  FAULT_TYPE_NONE=0,
  FAULT_INVALID_MSGTYPE_ON_SLR,
  FAULT_INVALID_MSGTYPE_ON_PSR,
  FAULT_INVALID_MSGTYPE,
  FAULT_LOSSY_CHANNEL,
  FAULT_INVALID_SIZE_ON_SLR,
  FAULT_INVALID_SIZE_ON_PSR,
  FAULT_INVALID_SIZE_ON_SARES,
  FAULT_INVALID_SIZE_ON_SSR,
  FAULT_DUPLICATE_ON_SLR,
  FAULT_DUPLICATE_ON_PSR,
  FAULT_DUPLICATE_ON_SAR,
  FAULT_INVALID_X_ON_PSR
};

static void usage()
{
  fprintf(stdout,
          " On a fault, the client will disconnect.\n"
          " Thus, only one fault can be given.\n"
          " Fault types: %d=%s\n"
          "              %d=%s\n"
          "              %d=%s\n"
          "              %d=%s\n"
          "              %d=%s\n"
	  "              %d=%s\n"
          "              %d=%s\n"
          "              %d=%s\n"
	  "              %d=%s\n"
          "              %d=%s\n",
          FAULT_INVALID_MSGTYPE_ON_SLR, "Invalid message type on slr",
	  FAULT_INVALID_MSGTYPE_ON_PSR,  "Invalid message type on psr",
	  FAULT_LOSSY_CHANNEL, "Lossy channel",
	  FAULT_INVALID_SIZE_ON_SLR,    "Invalid UDP size on slr",
	  FAULT_INVALID_SIZE_ON_PSR,    "Invalid UDP size on psr",
	  FAULT_INVALID_SIZE_ON_SARES,  "Invalid UDP size on sares",
	  FAULT_INVALID_SIZE_ON_SSR,    "Invalid UDP size on ssr",
          FAULT_DUPLICATE_ON_SLR, "Send slrequest twice",
	  FAULT_DUPLICATE_ON_PSR, "Send psrequest twice",
	  FAULT_DUPLICATE_ON_SAR, "Send sarequest twice",
	  FAULT_INVALID_X_ON_PSR, "Send a bad x to the tracker"
	  );
}
