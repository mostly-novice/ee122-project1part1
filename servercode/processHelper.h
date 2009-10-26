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
      char invstatetosent[8];
      createinvalidstate(0,invstatetosent);
      unicast(sock,invstatetosent,INVALID_STATE_SIZE);
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

	mkdir(DIR);
	chdir(DIR);

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

int process_move(int listener,
		 int sock,
		 int fdmax,
		 fd_set master,
		 char * name,
		 char direction,
		 LinkedList * mylist){

  Player * player = findPlayer(name,mylist);
  if(player){
    if(direction==NORTH){
      player->x-= 3;
    }else if(direction==SOUTH){
      player->x+= 3;
    }else if(direction==WEST){
      player->y-= 3;
    }else if(direction==EAST){
      player->y+= 3;
    }
    
    //writeToFile(player);

    unsigned char mntosent[MOVE_NOTIFY_SIZE];
    createmovenotify(name,
		     player->hp,
		     player->exp,
		     player->x,
		     player->y,
		     mntosent);
    broadcast(master,listener,sock,fdmax,mntosent,MOVE_NOTIFY_SIZE);
  }else{ // Handle by the client
    // Send 
  }
}

int process_attack(int listener,
		   int sock,
		   int fdmax,
		   fd_set master,
		   char * attacker,
		   char * victim,
		   LinkedList * mylist){
  if(strcmp(attacker,victim)){ // If the attacker is different than the victim
    Player * victim = findPlayer(victim,mylist);
    if(victim){ // If victim is not in the list
      int damage = 10+rand()%11; // Randomize the damage from 10 to 20
      if(damage > victim->hp){
	damage = victim->hp;
      }

      victim->hp -= damage;
      victim->exp += damage;
      

      // Broadcasting attack_notify
      unsigned char antosent[ATTACK_NOTIFY_SIZE];
      createattacknotify(attacker,
			 victim,
			 victim->hp,
			 damage,
			 antosent);
      broadcast(master,listener,sock,fdmax,antosent,ATTACK_NOTIFY_SIZE);

      if(victim->hp == 0){
	victim->hp = 30+rand()%21;
	victim->x = rand()%100;
	victim->y = rand()%100;
      

	// Broadcasting move notify
	unsigned char mntosent[MOVE_NOTIFY_SIZE];
	createmovenotify(victim,
			 victim->hp,
			 victim->exp,
			 victim->x,
			 victim->y,
			 mntosent);
	broadcast(master,listener,sock,fdmax,mntosent,MOVE_NOTIFY_SIZE);
      }
    }
  }
}

int process_speak(char payload_c[]){
}

int process_logout(int listener,
		    int sock,
		    int fdmax,
		    fd_set master,
		    char * name,
		    LinkedList * mylist){
  // Remove the player from the list
  Player * player = findPlayer(name,mylist);
  if(player){
    writeToFile(player); // Write the data to file

    // Clean up
    FD_CLR(sock,&master);
    removePlayer(name,mylist);

    unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
    createlogoutnotify(lntosent);
    broadcast(master,listener,sock,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
  } else {
    fprintf(stderr,"Process Logout: player %s is not online.\n",name);
  }
}

int process_invalid_state(char payload_c[]){
}

int updateHP(LinkedList * mylist){
   if(mylist->head == NULL){
       return 0; // nothing updated
   }
   Player * i;
   for(i=mylist->head->datum; i!=NULL;i=i->next->datum){
       printf("updating %s's hp\n",i->name);
       i->hp = i->hp + 1;
   }

}
