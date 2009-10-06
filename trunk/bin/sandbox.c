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
  Node * list;
  Node * tail;
  Player * p1 = (Player *) malloc(sizeof(Player));
  Player * p2 = (Player *) malloc(sizeof(Player));
  Player * p3 = (Player *) malloc(sizeof(Player));

  p1->name = "kevin";
  p2->name = "aepr88";
  p3->name = "tony";

  Node * newnode1 = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
  newnode1->datum = p1;
  list = newnode1;
  tail = newnode1;

  Node * newnode2 = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
  newnode2->datum = p2;
  tail->next = newnode2; 

  Node * newnode3 = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
  newnode3->datum = p3;
  tail->next = newnode3;

  Node * p = findPlayer("tony", list);
  printf("name:%s\n",p->datum->name);

  p = findPlayer("tony2", list);
  if(p) printf("found\n");
  else printf("not found\n");

  p = findPlayer("tony", list);
  if(p) printf("found\n");
  else printf("not found\n");
}
