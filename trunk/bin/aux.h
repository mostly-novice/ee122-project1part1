int isVisible(int x1, int y1, int x2, int y2){
  return (abs(x1 - x2)<=5 && abs(y1 - y2)<=5);
}

void readstdin(char * command, char * arg){
  char mystring[300];
  char * pch;
  fgets(mystring,300,stdin);
  mystring[strlen(mystring)-1] = 0;
  pch = strtok(mystring," ");
  strcpy(command,pch);
  strcpy(arg, pch+strlen(pch)+1);
}

void stats(Player * p){
  fprintf(stdout, "%s: location=(%u,%u), HP=%u, EXP=%u\n",
          p->name, p->x, p->y, p->hp, p->exp);
}
