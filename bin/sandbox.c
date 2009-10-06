#include<stdlib.h>
#include<stdio.h>

int main(){
  char mystring[300];
  char * pch;
  char command[80] = "login phuc";
  char arg[80];
  
  fgets(mystring,300,stdin);
  pch = strtok (str," ");
  printf("pch:%s");
  puts(command);
  printf("%s\n", arg);
}
