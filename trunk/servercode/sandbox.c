#include <stdlib.h>
#include <time.h>

void printArray(int array[],int i){
  int j;
  for(j=0;j<i;j++)
    printf("%d ",array[j]);
  printf("\n");
}

int main(){
  int a[] = {1,2,3,4,5};
  printArray(a,5);
}
