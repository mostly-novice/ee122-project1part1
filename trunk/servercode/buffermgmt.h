#include <stdio.h>
#include <stdlib.h>

int disconnect(int fd, char* buffermap[]){
    // why are we trying to free anything? why not just set the array pointer to NULL ...
    // you can't free something that you dont malloc in the first place.
    // free(buffermap[fd]);
    buffermap[fd] = NULL;
}

int putBuffer(int fd, char* buffaddr, char* buffermap[]){
    buffermap[fd] = buffaddr;
}
