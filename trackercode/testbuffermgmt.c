#include <stdio.h>

#include "buffermgmt.h"


int main(){
    char **map;
    int fd = 5;


    // add stuff to buffer
    char *string = "hello world";
    putBuffer(fd,string,map);

    // did putBuffer work?
    printf("map at %d is %s\n",fd,map[fd]);


    // call disconnect
    disconnect(fd, map);

    // did disconnect work?
    if(map[fd] != NULL){
	printf("boo, it didn't work\n");
    }else{
	printf("yay, it worked!\n");
    }
}
