#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int ceiling(double x){
  return ((int)x)+1;
}

int main(){
  double a=6.04;
  int b = ceiling(a);
  printf("%d\n",b);
}
