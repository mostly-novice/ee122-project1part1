#include <time.h>
#include "sendhelper.h"
#define DIR "users"

#define LOGIN_REPLY_SIZE 16
#define MOVE_NOTIFY_SIZE 24  
#define ATTACK_NOTIFY_SIZE 32
#define SPEAK_NOTIFY_SIZE 32
#define LOGOUT_NOTIFY_SIZE 32
#define INVALID_STATE_SIZE 32

int process_login_request(int listener, int sock, int fdmax, fd_set master, unsigned char * n, LinkedList * activeList){

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

	newplayer->hp = hp;
	newplayer->exp = exp;
	newplayer->x = x;
	newplayer->y = y;

	//mkdir(DIR);
	//chdir(DIR);

	//check if the file with that name exists
	FILE * file = fopen(name,"r");

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
	unsigned char lrtosent[LOGIN_REPLY_SIZE];
	unsigned char mntosent[MOVE_NOTIFY_SIZE];
	createloginreply(0,
			 newplayer->hp,
			 newplayer->exp,
			 newplayer->x,
			 newplayer->y,
			 lrtosent);
	unicast(sock,lrtosent,LOGIN_REPLY_SIZE);
	createmovenotify(name,newplayer->hp,
			 newplayer->exp,
			 newplayer->x,
			 newplayer->y,
			 mntosent);
	broadcast(master,listener,sock,fdmax,mntosent,MOVE_NOTIFY_SIZE);
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
