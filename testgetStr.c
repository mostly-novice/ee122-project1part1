#include <stdio.h>

char * getNameByFD(int fd, char * list[]){
    return list[fd];
}


void putName(int fd, char* name, char*map[]){
    map[fd] = name;
}

int main(){
    char * s[] = {"abc", "def","hij","klm","nop","qrs","tuv","wux","yz"};
    int fd = 4;

    char * name = getNameByFD(fd,s);
    char * name2;

    printf("name is: %s\n",name);


    putName(5,"tony",&s);
    name2 = getNameByFD(5,s);
    printf("name of fd 5 is: %s\n",name2);

}


