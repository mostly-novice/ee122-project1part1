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
