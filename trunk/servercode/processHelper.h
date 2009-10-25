#include <time.h>
#include "sendhelper.h"

#define DIR "users"

#define LOGIN_REPLY_SIZE 32
#define MOVE_NOTIFY_SIZE 32  
#define ATTACK_NOTIFY_SIZE 32
#define SPEAK_NOTIFY_SIZE 32
#define LOGOUT_NOTIFY_SIZE 32
#define INVALID_STATE_SIZE 32
int process_login_request(int sock[], int length, char payload_c[],char * name, LinkedList * activeList){
  if(findPlayer(name,activeList)){ // if the guy is already logged in
    sendinvalidstate(INVALID_STATE); // Send invalid state
  }else {
    // make a new instance of Player
    Player * newplayer = (Player *) malloc(sizeof(Player));

    mkdir(DIR);
    chdir(DIR);

    //check if the file with that name exists
    FILE * file = fopen(name,"r");
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
      initialize(newplayer,name,hp,exp,x,y);
      addPlayer(newplayer,activeList);

      // Write to file
      fprintf(file,"%d %d %d %d",newplayer->hp,newplayer->exp,newplayer->x,newplayer->y);
    }

    unsigned char * lrtosent = createloginreply(sock,0,newplayer->hp,newplayer->exp,newplayer->x,newplayer->y); // added field sock
    unicast(sock,lrtosent,LOGIN_REPLY_SIZE);
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

//int process_invalid_state(char payload_c[]){
//}
