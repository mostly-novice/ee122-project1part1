#include<stdlib.h>
#include<stdio.h>

int main(){
  char * s = "login phuc";
  char command[80] = "login phuc";
  char arg[80];
  
  sscanf(command,"%s %s", command,arg);
  puts(command);
  printf("%s\n", arg);
}
