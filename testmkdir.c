#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define DIRECTORY "users"
#define FILENAME "tony.txt"


typedef struct player_struct{
    unsigned int hp;
    unsigned int x;
    unsigned int y;
    unsigned int exp;
}Player;


int writeToFile(Player* tony){
    FILE *fd = fopen("testfile.txt","w+");
    if(fd){
	fprintf(fd,"%d %d %d %d",tony->hp,tony->exp,tony->x,tony->y);
	fclose(fd);
	return 1;
    }
    return 0;

}

int main(){

    unsigned char* n = "wowz0rs";
    unsigned char name[strlen(n) + 1];
    strcpy(name,n);
    strcat(name,"\0");

    // TEST if file exists
    FILE * tempFH = fopen("testing123.txt","r");
    if(!tempFH){
    }


    // MAKE PLAYER
    srand(time(NULL));
    int val = rand() % 10;
    int val2 = random();

    Player * tony = (Player*)malloc (sizeof(Player*));
    tony->hp = 100 + (random() %21);
    tony->x = random() % 101;
    tony->y = random() % 101;
    tony->exp = 0;

    if(writeToFile(tony)){
	printf("successful write\n");
	exit(1);
    }


    // MAKE DIRECTORY
    mkdir(DIRECTORY,777);
    chdir(DIRECTORY);

    // WRITE FILE
    FILE *file = fopen(name,"w+");
    fprintf(file,"%d %d %d %d",tony->hp,tony->exp,tony->x,tony->y);
    fputs("more stuff in the file\n",file);
    chmod(file,777);
    fclose(file);


    // READ FILE
    FILE *fh;
    int a,b,c,d;
    if((fh = fopen(name,"r"))==NULL){ printf("error! can't open file for reading\n"); }
    fscanf(fh, "%d%d%d%d",&a,&b,&c,&d ); /* read from file */
    fclose(fh);
    
    // PRINT OUT WHAT WE READ
    fprintf(stdout, "%d %d %d %d\n",a,b,c,d); /* print on screen */
    return 0;
}
