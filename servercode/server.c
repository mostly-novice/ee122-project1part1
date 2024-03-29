// Server code
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <unistd.h>

#include "header.h"
#include "messages.h"
#include "server_output.h"

#define STDIN 0
#define HEADER_LENGTH 4
#define DIR "users"

// Flags
#define HEADER 0
#define PAYLOAD 1

#define MAX_CONNECTION 30
#define MAX_MESSAGE_RECORD 50

enum {
	FAULT_TYPE_NONE = 0,
	FAULT_LOSSY_LINK,
	FAULT_MESSAGE_TYPE,
	FAULT_ERROR_CODE,
};

static unsigned s_fault = FAULT_TYPE_NONE;

typedef struct mr{
	unsigned int ip;
	unsigned int id;
	unsigned char * message;
} message_record;

typedef struct buffer{
	int flag;
	int desire_length;
	int buffer_size;
	char * buffer;
} bufferdata;

int findDup(message_record ** mr_array,int id, int ip){
	int i;
	for(i = 0; i <MAX_MESSAGE_RECORD;i++){
		message_record * mr = mr_array[i];
		if(mr){
			if(mr->ip == ip && mr->id == id){
				return i;
			}
		}
	}
	return -1;
}


// Debugging variables
int mc; // malloc counter
int fc; // free counter

void printMessage(char * message, int len){
	int i;
	for(i = 0; i < len; i++){
		printf("%02x ", *(message+i));
	}
	printf("\n");
}

void cleanBuffer(bufferdata ** fdbuffermap,int i){
	if(fdbuffermap[i]){
		if(fdbuffermap[i]->buffer)
			free(fdbuffermap[i]->buffer);
		free(fdbuffermap[i]);

		bufferdata * bufferd = (bufferdata *) malloc(sizeof(bufferdata));
		bufferd->flag = HEADER;
		bufferd->desire_length = HEADER_LENGTH;
		bufferd->buffer_size = 0;
		bufferd->buffer = NULL;

		fdbuffermap[i] = bufferd;
	}
}

void cleanNameMap(char ** fdnamemap,int i){
	printf("Cleaning up name map \n");
	if(fdnamemap[i]){
		free(fdnamemap[i]);
		fdnamemap[i] = NULL;
	}
}

#include "model.h"
#include "processHelper.h"
#include "aux.h"
int main(int argc, char* argv[]){

	mkdir(DIR,0700);
	chdir(DIR);

	// Model Variables
	LinkedList * mylist = (LinkedList *) malloc (sizeof(LinkedList));
	mylist->head = NULL;
	mylist->tail = NULL;
	struct sockaddr_in client_sin;
	Node * p;
	char command[80];
	char arg[4000];
	struct timeval tv;
	int oldest = 0;

	// Connection variables
	// Keep track of the list of sockets

	srand(time(NULL));
	int id = rand();

	int listener;
	int udplistener;
	int myport;
	int myudpport;
	int done = 0;
	int status;

	message_record ** mr_array = malloc(sizeof(*mr_array)*MAX_MESSAGE_RECORD);

	fd_set master;
	fd_set readfds;
	fd_set login;
	int fdmax;

	char ** fdnamemap = malloc(sizeof(*fdnamemap)*MAX_CONNECTION);
	bufferdata ** fdbuffermap = malloc(sizeof(*fdbuffermap)*MAX_CONNECTION);
	int k;
	for(k=0; k<MAX_CONNECTION; ++k ){
		fdnamemap[k] = NULL;
		bufferdata * bufferd = (bufferdata *) malloc(sizeof(bufferdata));
		bufferd->flag = HEADER;         // error: parse error before '.' token
		bufferd->desire_length = HEADER_LENGTH;
		bufferd->buffer_size = 0;
		bufferd->buffer = NULL;
		fdbuffermap[k] = bufferd;
	}
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));

	struct sockaddr_in udpsin;
	memset(&udpsin, 0, sizeof(udpsin));
	int sin_len;


	//  if(argc != 5){ printf("Usage: ./server -t <server TCP port> -u <server UDP port");  exit(0);}

	// Error  flags

	int badMessageTypeFlag = 0;
	int lossy = 0;
	int faultyErrorCodeFlag = 0;
	int lossycount = 0;
	// Initilizations
	int c;
	char* uvalue=NULL;
	char* tvalue=NULL;

	while( (c=getopt( argc,argv,"u:t:f:"))!=-1){
		switch(c){
			case 'u':
				uvalue=optarg;
				break;
			case 't':
				tvalue=optarg;
				break;
			case 'f':
				s_fault = atoi(optarg);break;
			default:
				printf("Usage: ./server -t <server TCP port> -u <server UDP port");
				return 0;
		}
	}
	myport = atoi(tvalue);
	myudpport = atoi(uvalue);
	if(setvbuf(stdout,NULL,_IONBF,NULL) != 0){
		perror('setvbuf');
	}

	listener = socket(AF_INET, SOCK_STREAM, 0);
	udplistener = socket(AF_INET, SOCK_DGRAM, 0);
	if(listener < 0 || udplistener < 0){
		perror("socket() failed\n");
		abort();
	}

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(myport);

	udpsin.sin_family = AF_INET;
	udpsin.sin_addr.s_addr = INADDR_ANY;
	udpsin.sin_port = htons(myudpport);

	int optval = 1;
	if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) < 0){
		perror("Reuse failed");
		abort(); // TODO: check if this is the appropriate behavior.
	}

	if (setsockopt(udplistener,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval)) < 0){
		perror("Reuse failed");
		abort(); // TODO: check if this is the appropriate behavior.
	}

	if (bind(listener,(struct sockaddr *) &sin, sizeof(sin)) < 0){
		// DO I NEED TO SEND THIS??? message_on_server_port_bind_failure();
		perror("Bind failed");
		abort();
	}

	if (bind(udplistener,(struct sockaddr *) &udpsin, sizeof(udpsin)) < 0){
		perror("UDP Bind failed");
		abort();
	}
	if (listen(listener,MAX_CONNECTION)){
		perror("listen");
		abort();
	}

	FD_ZERO(&master);
	FD_ZERO(&readfds);
	FD_ZERO(&login);

	FD_SET(listener,&master);
	FD_SET(udplistener,&master);

	if(udplistener>fdmax) fdmax = udplistener;
	if(listener>fdmax) fdmax = listener;

	int timeout = 1;
	time_t lasttime = time(NULL);

	while(1){ // main accept() lo
		time_t currenttime = time(NULL);
		tv.tv_sec = 5;
		tv.tv_usec = 0;
		timeout = 1;

		readfds = master; // copy it
		if (select(fdmax+1,&readfds,NULL,NULL,&tv) == -1){
			perror("select");
			exit(-1);
		}

		// run through the existing connections looking for data to read
		int i;
		for(i=0; i<= fdmax; i++){
			if (FD_ISSET(i,&readfds)){
				if (currenttime-lasttime < 5) timeout = 0;

				/*
				 *	Handling Data from UDP PORT
				 */ 

				if (i==udplistener){ 

					/* MAKE IT LOSSY */	
					if(s_fault == FAULT_LOSSY_LINK){
						lossycount++;
						if(lossycount%2){
							break;
						}
					}

					printf("Got a udp message\n");
					unsigned char udp_read_buffer[4096];
					int expected_data_len = sizeof(udp_read_buffer);
					int read_bytes = recvfrom(udplistener,
							udp_read_buffer,
							expected_data_len,
							0,
							(struct sockaddr*)&udpsin,&sin_len);

						if(read_bytes < 0){
							perror("Server - Recvfrom Failed - read_bytes return -1\n");
							close(i); // bye!
						} else { // read succedded, proceed

							char msgtype = udp_read_buffer[0];
							unsigned int ip = udpsin.sin_addr.s_addr;
							unsigned int id = (udp_read_buffer[4]<<24)+(udp_read_buffer[3]<<16)+(udp_read_buffer[2]<<8)+udp_read_buffer[1];
							int dup = findDup(mr_array,id,ip); // return the index of the duplicate message

							// ON DUPLICATE
							if(dup >= 0){
								on_udp_duplicate(htonl(ip));
								// resend message player state response
								if(msgtype==PLAYER_STATE_RESPONSE){
									sendto(i,mr_array[dup]->message,
											PLAYER_STATE_RESPONSE_SIZE,
											0,
											(struct sockaddr*)&udpsin,
											sizeof(udpsin));
									// resend message save state response
								}else if(msgtype==SAVE_STATE_RESPONSE){
									sendto(i,mr_array[dup]->message,
											SAVE_STATE_RESPONSE_SIZE,
											0,
											(struct sockaddr*)&udpsin,
											sizeof(udpsin));
								}
							}else{	// NOT A DUPLICATE

								if(udp_read_buffer[0] == PLAYER_STATE_REQUEST){
									if(read_bytes != PLAYER_STATE_REQUEST_SIZE){
										on_malformed_udp(1);
									}else{
										struct player_state_request * psr = (struct player_state_request *) udp_read_buffer;
										//check name
										if(check_player_name(psr->name)==0){	
											printf("PLAYER NAME IS BAD\n");
											continue;
										}
										process_psr(psr->name,udplistener,udpsin,psr->id,oldest,mr_array,badMessageTypeFlag);
									}


								} else if (udp_read_buffer[0] == SAVE_STATE_REQUEST){

									if(read_bytes != SAVE_STATE_REQUEST_SIZE){
										on_malformed_udp(1);
									}else{
										struct save_state_request * ssr = (struct save_state_request *) udp_read_buffer;

										//check name
										if(check_player_name(ssr->name)==0){
											printf("PLAYER NAME IS BAD\n");
											continue;
										}

										//check stats
										if(check_malformed_stats(ssr->x,ssr->y,ntohl(ssr->hp),ntohl(ssr->exp))!=0){
											printf("STATS ARE BAD\n");
											continue;
										}

										process_ss_request(ssr->name,ssr->hp,ssr->exp,ssr->x,ssr->y,udplistener,udpsin,ssr->id,oldest,mr_array,faultyErrorCodeFlag);
									}
								}else{
									// malformed type
									on_malformed_udp(3);
									continue;
								}

								//	Update Duplicate message check
								oldest = (oldest + 1)%MAX_MESSAGE_RECORD;


							}
						}

					/*
					 *	Handling Data from TCP PORT
					 */ 

				} else if (i==listener){ 
					printf("Received a new tcp connection\n");
					// handle new connection
					int addr_len = sizeof(client_sin);
					int newfd = accept(listener,(struct sockaddr*) &client_sin,&addr_len);
					if (newfd < 0){
						perror("accept failed");
					} else {
						FD_SET(newfd,&master);
						if (newfd > fdmax) fdmax = newfd;
						printf("New connection in socket %d\n", newfd);
					}

				} else {
					unsigned char read_buffer[4096];
					int expected_data_len = sizeof(read_buffer);
					unsigned char *p = (char*) read_buffer; // Introduce a new pointer
					int offset = 0;
					bufferdata * bufferd = fdbuffermap[i];
					unsigned char header_c[HEADER_LENGTH];
					struct header * hdr;

					int read_bytes = recv(i,read_buffer,expected_data_len, 0);

					if (read_bytes <= 0){ 
						close(i); // bye!
						FD_CLR(i,&login);
						FD_CLR(i,&master); // remove from the master set

						// got error or connection closed by client
						if(read_bytes == 0){
							printf("Socket %d hung up\n",i);
							if (fdnamemap[i]){
								Player * player = findPlayer(fdnamemap[i],mylist);
								if(player){
									unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
									createlogoutnotify(fdnamemap[i],lntosent);
									broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
									removePlayer(fdnamemap[i],mylist);
								}
							}
						} else { // read_bytes == -1
							printf("Socket %d hung up\n",i);
							if (fdnamemap[i]){
								Player * player = findPlayer(fdnamemap[i],mylist);
								if(player){
									unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
									createlogoutnotify(player,lntosent);
									broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
									removePlayer(fdnamemap[i],mylist);
								}
							}
						}
						cleanNameMap(fdnamemap,i);
						cleanBuffer(fdbuffermap,i);

					} else {
						if(bufferd->buffer == NULL){
							bufferd->buffer = (char*)malloc(sizeof(char)*read_bytes);
							memcpy(bufferd->buffer,read_buffer,read_bytes);
						} else {
							bufferd->buffer = (char*)realloc(bufferd->buffer,bufferd->buffer_size+read_bytes);
							memcpy(bufferd->buffer+bufferd->buffer_size,read_buffer,read_bytes);
						}
						bufferd->buffer_size += read_bytes;
						while (bufferd->buffer_size >= bufferd->desire_length){
							if(bufferd->flag == HEADER){
								int j;
								for(j = 0; j < HEADER_LENGTH; j++){ header_c[j] = *(bufferd->buffer+j);}
								hdr = (struct header *) header_c;
								if (check_malformed_header(hdr->version,ntohs(hdr->len),hdr->msgtype) < 0){
									close(i);
									FD_CLR(i,&login);
									FD_CLR(i,&master);
									if(fdnamemap[i]){
										Player * player = findPlayer(fdnamemap[i],mylist);
										if(player){ unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
											createlogoutnotify(player,lntosent);
											broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
											removePlayer(fdnamemap[i],mylist);
										}
										cleanNameMap(fdnamemap,i);
									}
									if(fdbuffermap) cleanBuffer(fdbuffermap,i);
									break;
								} else { // If not malform
									char * temp = (char*) malloc(sizeof(char)*(bufferd->buffer_size-HEADER_LENGTH));
									memcpy(temp,bufferd->buffer+4,bufferd->buffer_size-4);
									free(bufferd->buffer);
									bufferd->buffer = temp;
									bufferd->buffer_size -= HEADER_LENGTH;
									bufferd->desire_length = ntohs(hdr->len)-HEADER_LENGTH;
									bufferd->flag = PAYLOAD;
								}

							} else { // Payload

								char payload_c[bufferd->desire_length];
								int j;
								for(j = 0; j < bufferd->desire_length; j++){ payload_c[j] = *(bufferd->buffer+j);}

								if(hdr->msgtype == LOGIN_REQUEST){ // LOGIN REQUEST
									if (FD_ISSET(i,&login)){ // Is he typing in login again?
										unsigned char ivstate[8];
										createinvalidstate(1,ivstate);
										int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
										if (bytes_sent < 0){
											perror("send failed");
											abort();
										}
									} else { // If he is not logged in

										//if(check_malformed_stats(ssr->x,ssr->y,ntohl(ssr->hp),ntohl(ssr->exp))!=0){
										struct login_request * lr = (struct login_request *) payload_c;
										if(check_player_name(lr->name)==0 || check_malformed_stats(lr->x,lr->y,ntohl(lr->hp),ntohl(lr->exp))!=0){
											close(i);
											FD_CLR(i,&login);
											FD_CLR(i,&master);

											Player * player = findPlayer(lr->name,mylist);
											if(player){
												// Broadcasting the logout notify to other client
												unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
												createlogoutnotify(player,lntosent);
												broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
												removePlayer(fdnamemap[i],mylist);
												cleanNameMap(fdnamemap,i);
											}
											cleanBuffer(fdbuffermap,i);
											break;
										}else if (isnameinmap(lr->name,fdnamemap)){ // If the name is already used
											// Send a login request with an errorcode 1
											Player * newplayer = process_login_request(1,i,fdmax,login,lr->name,lr->hp,lr->exp,lr->x,lr->y,mylist);
										} else {
											FD_SET(i,&login); // Log him in
											Player * newplayer = process_login_request(0,i,fdmax,login,lr->name,lr->hp,lr->exp,lr->x,lr->y,mylist);
											if (!fdnamemap[i]){ fdnamemap[i] = malloc(sizeof(char)*11);}
											strcpy(fdnamemap[i],lr->name);
											strcpy(newplayer->name,fdnamemap[i]);

											// Adding the player
											Node * node = (Node*) malloc(sizeof(Node)); // TODO: remember to free this
											node->datum = newplayer;
											node->next = NULL;
											if(mylist->head == NULL){
												mylist->head = node;
												mylist->tail = node;
											}else {
												mylist->tail->next = node;
												mylist->tail = node;
											}
										}
										//printMap(fdnamemap);
									}

									} else if(hdr->msgtype == MOVE){ // MOVE
										if (!FD_ISSET(i,&login)){ // if not login,

											// Send invalid state
											unsigned char ivstate[INVALID_STATE_SIZE];
											createinvalidstate(0,ivstate);
											int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
											if (bytes_sent < 0){
												perror("send failed");
												abort();
											}

										} else { // If logged in, good to proceed
											struct move * m = (struct move *) payload_c;
											int direction = m->direction;
											Player * player;                     
											if (fdnamemap[i]) { player = findPlayer(fdnamemap[i],mylist);
											} else {
												fprintf(stderr, "THIS SHOULD NEVER HAPPEN\n");
											}
											if(player){
												stats(player);
												if(direction==NORTH){ player->y -= 3; player->y = (100+player->y) % 100;
												}else if(direction==SOUTH){ player->y += 3; player->y = (100+player->y) % 100;
												}else if(direction==WEST){ player->x -= 3; player->x = (100+player->x) % 100;
												}else if(direction==EAST){ player->x += 3; player->x = (100+player->x) % 100;
												}else {
													close(i);
													FD_CLR(i,&login);
													FD_CLR(i,&master);
													if(fdnamemap[i]){
														Player * player = findPlayer(fdnamemap[i],mylist);
														if(player){

															// Broadcast to other clients
															unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
															createlogoutnotify(player,lntosent);
															broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
															removePlayer(fdnamemap[i],mylist);

														} else {
															fprintf(stderr, "THIS SHOULD NEVER HAPPEN\n");
															exit(-1);
														}
														cleanNameMap(fdnamemap,i);
													}
													cleanBuffer(fdbuffermap,i);
													break;
												}
												unsigned char mntosent[MOVE_NOTIFY_SIZE];
												createmovenotify(fdnamemap[i],player->hp,player->exp,player->x,player->y,mntosent);
												broadcast(login,i,fdmax,mntosent,MOVE_NOTIFY_SIZE);
											}
										}
									} else if(hdr->msgtype == ATTACK){ // ATTACK
										if (!FD_ISSET(i,&login)){ // if not login,
											// Send invalid state
											unsigned char ivstate[INVALID_STATE_SIZE];
											createinvalidstate(0,ivstate);
											int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
											if (bytes_sent < 0){
												perror("send failed");
												abort();
											}
										} else {
											// #TODO: HAVE TO CECK FOR THE NAME
											struct attack * attackPayload = (struct attack *) payload_c;
											char * victim = attackPayload->victimname;
											if (check_player_name(victim) == 0){
												close(i);
												FD_CLR(i,&login);
												FD_CLR(i,&master);
												if(fdnamemap[i]){
													Player * player = findPlayer(fdnamemap[i],mylist);
													if(player){
														unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
														createlogoutnotify(player,lntosent);
														broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
														removePlayer(fdnamemap[i],mylist);
													} else {
														fprintf(stderr, "THIS SHOULD NEVER HAPPEN\n");
														exit(-1);
													}
													cleanNameMap(fdnamemap,i);
												}
												cleanBuffer(fdbuffermap,i);
												break;
											}
											char * attacker;
											if (fdnamemap[i]){
												attacker = fdnamemap[i];
											} else {
												fprintf(stderr,"server.c - attack - THIS SHOULD NEVER HAPPEN.\n");
											}
											process_attack(i,fdmax,login,attacker,victim,mylist);
										}
									} else if(hdr->msgtype == SPEAK){ // SPEAK
										if (!FD_ISSET(i,&login)){ // if not login,
											// Send invalid state
											unsigned char ivstate[INVALID_STATE_SIZE];
											createinvalidstate(0,ivstate);
											int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
											if (bytes_sent < 0){
												perror("send failed");
												abort();
											}
										} else {
											struct speak * speakPayload = (struct speak *) payload_c;

											// Check for malformed message
											if(check_player_message(payload_c)==0){
												close(i);
												FD_CLR(i,&login);
												FD_CLR(i,&master);
												if(fdnamemap[i]){
													Player * player = findPlayer(fdnamemap[i],mylist);
													if(player){
														unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
														createlogoutnotify(player,lntosent);
														broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
														removePlayer(fdnamemap[i],mylist);
													} else {
														fprintf(stderr,"Internal data structure error\n");
													}
													cleanNameMap(fdnamemap,i);
												}
												cleanBuffer(fdbuffermap,i);
												break;
											}
											int msglen = strlen(payload_c)+1+10+HEADER_LENGTH;
											int totallen;
											if(msglen%4) totallen = msglen + (4 - msglen%4);
											else totallen = msglen;
											unsigned char spktosent[totallen];
											createspeaknotify(fdnamemap[i],payload_c,totallen,spktosent);
											printMessage(spktosent,totallen);
											broadcast(login,i,fdmax,spktosent,totallen);
										}
									} else if(hdr->msgtype == LOGOUT){
										if (!FD_ISSET(i,&login)){ // if not login,
											unsigned char ivstate[INVALID_STATE_SIZE];
											createinvalidstate(0,ivstate);
											int bytes_sent = send(i, ivstate,INVALID_STATE_SIZE,0);
											if (bytes_sent < 0){
												perror("send failed");
												abort();
											}
										} else {  // Player is logged in
											if(fdnamemap[i]){
												Player * player = findPlayer(fdnamemap[i],mylist);
												if(player){
													printf("found player %s...logging him out\n",player->name);
													unsigned char lntosent[LOGOUT_NOTIFY_SIZE];
													createlogoutnotify(player,lntosent);
													broadcast(login,i,fdmax,lntosent,LOGOUT_NOTIFY_SIZE);
													removePlayer(fdnamemap[i],mylist);
													close(i);
													FD_CLR(i,&login);
													FD_CLR(i,&master);
													cleanNameMap(fdnamemap,i);
												} else {
													fprintf(stderr, "Internal data structure error");
												}
											}
											cleanBuffer(fdbuffermap,i);
											bufferd = fdbuffermap[i];
											break;
										}
									} else {
										printf("We got nothing");
									}

									printf("Before adjusting the buffer.\n");

									// Move the pointers
									char * temp = (char*) malloc(sizeof(char)*(bufferd->buffer_size-bufferd->desire_length));
									memcpy(temp,
											bufferd->buffer+bufferd->desire_length,
											bufferd->buffer_size-bufferd->desire_length);
									free(bufferd->buffer);
									bufferd->buffer = temp;
									bufferd->buffer_size -= bufferd->desire_length;
									bufferd->desire_length = HEADER_LENGTH;
									bufferd->flag = HEADER;

									printf("After adjusting the buffer\n");

								} // end of handling payload
							} // End of while more desired length
						} // end else
					}  // end else someone has data
				} // end FD_ISSET
			}  // end foreach fd
			if (timeout){
				//updateHP(mylist);
				lasttime = currenttime;
			}
		} // end main while(1) loop

	}
