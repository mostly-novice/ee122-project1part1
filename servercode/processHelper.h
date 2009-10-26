#include <time.h>
#include "sendhelper.h"
#define DIR "users"

#define LOGIN_REPLY_SIZE 32
#define MOVE_NOTIFY_SIZE 32  
#define ATTACK_NOTIFY_SIZE 32
#define SPEAK_NOTIFY_SIZE 32
#define LOGOUT_NOTIFY_SIZE 32
#define INVALID_STATE_SIZE 32

int process_login_request(int sock[], int length, unsigned char * n, LinkedList * activeList){

    unsigned char name[strlen(n) + 1];
    strcpy(name,n);
    strcat(name,"\0");

    if(findPlayer(name,activeList)){ // if the guy is already logged in
	sendinvalidstate(INVALID_STATE); // Send invalid state
    }else {
	// make a new instance of Player
	Player * newplayer = (Player *) malloc(sizeof(Player));

	// randomize stats
	int hp = 100 + rand()%21;
	int exp = 0;
	int x = rand()%100;
	int y = rand()%100;

	// populate the newplayer fields
	newplayer->hp = hp;
	newplayer->exp = exp;
	newplayer->x = x;
	newplayer->y = y;

	//mkdir(DIR);
	//chdir(DIR);


	//check if the file with that name exists
	printf("opeing a file for reading\n");
	FILE * file = fopen(name,"r");
	printf("value of xp: %d\n",newplayer->hp);
	printf("value of exp: %d\n",newplayer->exp);
	printf("value of x: %d\n",newplayer->x);
	printf("value of y: %d\n",newplayer->y);



	if(file){ // if file exists
	    fscanf(file,"%d%d%d%d",&(newplayer->hp),&(newplayer->exp),&(newplayer->x),&(newplayer->y));
	} else {
	  
	  FILE * file2 = fopen(name,"w+");
	  if(file2 == NULL)	// open a new file with overwrite
	    perror("file open");
	  
	  
	  srand(time(NULL));

	  // Initialize the player and add him to the list
	  initialize(newplayer,name,hp,exp,x,y);
	  addPlayer(newplayer,activeList);
	  
	  // Write to file
	  fprintf(file2,"aheuwadgauw");
	} 

	unsigned char * lrtosent = createloginreply(sock,
						    0,
						    newplayer->hp,
						    newplayer->exp,
						    newplayer->x,
						    newplayer->y); // added field sock
	unicast(sock,lrtosent,LOGIN_REPLY_SIZE);
	printf("Done sending\n");
	unsigned char * mntosent = createmovenotify(name,newplayer->hp,newplayer->exp,newplayer->x,newplayer->y);
	broadcast(sock,sizeof(sock),mntosent,MOVE_NOTIFY_SIZE); // changed socklist to sock
    }
}

int process_move(char payload_c[], Player * self, LinkedList * mylist){
}

int process_attack(char payload_c[], Player * self, LinkedList * mylist){
}

int process_speak(char payload_c[]){
}

int process_logout(char payload_c[], LinkedList * mylist){
}

int process_invalid_state(char payload_c[]){
}
