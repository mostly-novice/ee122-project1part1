#include <stdio.h>

int main(){

  char name [80];
  char command [80];
  int done = 0;

  printf("Hello!\n");
  printf("Who are you?");
  scanf("%s",name);
  while(!done){
    printf("%s >> ",name);
    scanf("%s",command);
    if(strcmp(command,"exit") == 0){
      done = 1;
    }
  }



}
