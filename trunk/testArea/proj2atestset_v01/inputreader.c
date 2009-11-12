#include <stdio.h>

int main(int argc, char* argv[]) {
  FILE* fin;
  char c;

  fin = fopen(argv[1],"r");

  while(1) {
    if(fscanf(fin,"%c",&c)> 0) {
      if(c =='~') {
	break;
      }
      printf("%c",c);
      fflush(stdout);
    }
  }

  fclose(fin);
}
