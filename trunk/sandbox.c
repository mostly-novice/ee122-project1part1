#include<stdlib.h>
#include<stdio.h>

typedef struct P{
  char * name;
} Player;

struct list_el {
  Player * datum;
  struct list_el * next;
};

typedef struct list_el Node;

Node * findPlayer(char * name, Node * list){
  Node * p;
  for(p = list; p != NULL; p = p->next){
    if (strcmp(p->datum->name,name)==0){
      return p;
    }
  }
  return NULL;
}

void addPlayer(Node * node, Node * list, Node * tail){
  if(tail == NULL && list == NULL){ // First player
    tail = node;
    list = node;
  } else {
    tail->next = node;
    tail = node;
  }
}

int main(){
  char mystring[300] = "speak I come for peace!";
  char * pch;
  char command[80];
  char arg[255];
  fgets(mystring,300,stdin);
  if(strcmp(mystring,"\n")==0){
    return 0;
  }
  mystring[strlen(mystring)-1] = 0;
  pch = strtok(mystring," ");
  strcpy(command,pch);

  pch += strlen(pch);

  while(*(pch)==' ' || *(pch)=='\0' ){
    pch++;
  }

  strcpy(arg, pch);
  
  printf("Command:%s\n", command);
  printf("Arg:%s\n", arg);
  return 1;
}
