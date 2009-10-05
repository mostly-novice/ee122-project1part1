#include<stdlib.h>
#include<stdio.h>

typedef struct P{
  int x;
} Player;

struct list_el {
   Player *  val;
   struct list_el * next;
};

typedef struct list_el item;

void main() {
   item * curr, * head;
   int i;

   head = NULL;

   for(i=1;i<=10;i++) {
      curr = (item *)malloc(sizeof(item));
      Player * p = (Player*) malloc(sizeof(Player));
      p->x = i;
      curr->val = p;
      curr->next  = head;
      head = curr;
   }

   curr = head;

   while(curr) {
      printf("%d\n", curr->val->x);
      curr = curr->next ;
   }
}
