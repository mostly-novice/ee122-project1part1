#include <time.h>
#include "sendhelper.h"

#define LOGIN_REPLY_SIZE 16
#define MOVE_NOTIFY_SIZE 24  
#define ATTACK_NOTIFY_SIZE 32
#define SPEAK_NOTIFY_SIZE 20
#define LOGOUT_NOTIFY_SIZE 16
#define INVALID_STATE_SIZE 8

Player * process_login_request(int sock, int fdmax, fd_set login, unsigned char * n, LinkedList * activeList){
  char * name = (char *) malloc(sizeof(char)*(strlen(n)+1));
  strcpy(name,n);
  strcat(name,"\0");
  // make a new instance of Player
  Player * newplayer = (Player *) malloc(sizeof(Player));
  
  // randomize stats
  int hp = 100 + rand()%21;
  int exp = 0;
  int x = rand()%100;
  int y = rand()%100;

  memcpy(newplayer->name,name,10);
  newplayer->hp = hp;
  newplayer->exp = exp;
  newplayer->x = x;
  newplayer->y = y;
  
  //check if the file with that name exists
  FILE * file = fopen(name,"r");
  if(file){ // if file exists
    fscanf(file,"%d%d%d%d",&(newplayer->hp),&(newplayer->exp),&(newplayer->x),&(newplayer->y));
    fclose(file);
  } else {
    FILE * file2 = fopen(name,"w+");
    if(file2 == NULL)	// open a new file with overwrite
      perror("file open");
    srand(time(NULL));
    
    // Write to file
    fprintf(file2,"%d %d %d %d",newplayer->hp,newplayer->exp,newplayer->x,newplayer->y);
    fclose(file2);
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
  broadcast(login,sock,fdmax,mntosent,MOVE_NOTIFY_SIZE);
  return newplayer;
}
int process_move(int listener,
		 int sock,
		 int fdmax,
		 fd_set master,
		 Player * player,
		 char direction,
		 LinkedList * mylist){
}

int process_attack(int sock,
		   int fdmax,
		   fd_set login,
		   char * attackername,
		   char * victimname,
		   LinkedList * mylist){
  if(strcmp(attackername,victimname)){ // If the attacker is different than the victim
    Player * victim = findPlayer(victimname,mylist);
    Player * attacker = findPlayer(attackername,mylist);

    if(victim){ // If victim is not in the list
      int damage = 10+rand()%11; // Randomize the damage from 10 to 20
      if(damage > victim->hp){
	damage = victim->hp;
      }

      victim->hp -= damage;
      attacker->exp += damage;
      

      // Broadcasting attack_notify
      unsigned char antosent[ATTACK_NOTIFY_SIZE];
      createattacknotify(attacker->name,
			 victimname,
			 victim->hp,
			 damage,
			 antosent);
      printMessage(antosent,ATTACK_NOTIFY_SIZE);
      broadcast(login,sock,fdmax,antosent,ATTACK_NOTIFY_SIZE);

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
	broadcast(login,sock,fdmax,mntosent,MOVE_NOTIFY_SIZE);
      }
    } else {
      printf("Ignoring the message because attacker has the same name as victim.\n");
    }
  }
}

int process_speak(char * broadcaster,char*message){
}

int process_logout(int sock,
		   int fdmax,
		   fd_set master,
		   char * name,
		   LinkedList * mylist){
}

int process_invalid_state(char payload_c[]){
}

int updateHP(LinkedList * mylist){
  if(mylist->head == NULL){
    return 0; // nothing updated
  }
  Node * i;
  for(i=mylist->head; i!=NULL;i=i->next){
    printf("updating %s's hp\n",i->datum->name);
    i->datum->hp = i->datum->hp + 1;
  }

}
