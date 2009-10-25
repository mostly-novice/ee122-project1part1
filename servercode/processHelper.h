#include <time.h>
#include "sendhelper.h"

#define DIR "users"

#define LOGIN_REPLY_SIZE
#define MOVE_NOTIFY_SIZE
#define ATTACK_NOTIFY_SIZE
#define SPEAK_NOTIFY_SIZE
#define LOGOUT_NOTIFY_SIZE
#define INVALID_STATE_SIZE
int process_login_request(int sock[], int length, char payload_c[],char * name, LinkedList * activeList){
  if(findPlayer(name,activeList)){ // if the guy is already logged in
    sendinvalidstate(); // Send invalid state
  }else {
    // make a new instance of Player
    Player * newplayer = (Player *) malloc(sizeof(Player));

    mkdir(DIR);
    chdir(DIR);

    //check if the file with that name exists
    File * file = fopen(name,"r");
    if(file){ // if file exists
      fscanf(file,"%d%d%d%d",&(newplayer->hp),&(newplayer->exp),&(newplayer->x),&(newplayer->y));
    } else {
      file = fopen(name,'w+'); // open a new file with overwrite
      srand(time(NULL));

      // randomize stats
      int hp = 100 + rand()%21;
      int exp = 0;
      int x = rand()%100;
      int y = rand()%100;

      // Initialize the player and add him to the list
      initialize(newPlayer,name,hp,exp,x,y);
      addPlayer(newPlayer,activelist);

      // Write to file
      fprintf(file,"%d %d %d %d",newplayer->hp,newplayer->exp,newplayer->x,newplayer->y);
    }

    unsigned char * lrtosent = createloginreply(0,newplayer->hp,newplayer->exp,newplayer->x,newplayer->y);
    unicast(sock,lrtosent,LOGIN_REPLY_SIZE);
    unsigned char * mntosent = createmovenotify(name,newplayer->hp,newplayer->exp,newplayer->x,newplayer->y);
    broadcast(socklist,socklen,mntosent,MOVE_NOTIFY_SIZE);
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
