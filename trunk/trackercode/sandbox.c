#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(){
  char x[5];
  x[0] = 0x3;
  x[1] = 0x0;
  x[2] = 0x0;
  x[3] = 0x1;
  x[4] = 0x1;

  char*p = (char*)x;

  int y = (x[1]<<24)+(x[2]<<16)+(x[3]<<8)+x[4];
  printf("y=%d\n",y);
}
