// Helper to send messages.
void createslrespond(server_record * sr, int id, char buffer[]){
  struct storage_location_response* slr = (struct storage_location_response*) malloc (sizeof(char)*STORAGE_LOCATION_RESPONSE_SIZE);

  slr->message_type = STORAGE_LOCATION_RESPONSE;
  slr->id = htonl(id);
  slr->server_ip = sr->ip;
  slr->udpport = htons(sr->udp_port);

  printf("id:%d\n",sr->ip);
  printf("port:%d\n",sr->udp_port);
  
  printf("slr->id:%d\n",slr->id);
  printf("slr->udpport:%d\n",slr->udpport);

  unsigned char*tosent = (unsigned char *) slr;
  printMessage(slr,STORAGE_LOCATION_RESPONSE_SIZE);
  printMessage(tosent,STORAGE_LOCATION_RESPONSE_SIZE);
  int i;
  for(i = 0; i < STORAGE_LOCATION_RESPONSE_SIZE;i++){
    buffer[i] = tosent[i];
  }
  free(tosent);
}

// Creating the SERVER_AREA_RESPONSE package
void createsarespond(server_record * sr, int id,char buffer[]){
  struct server_area_response* sares = (struct server_area_response*) malloc (sizeof(char)*SERVER_AREA_RESPONSE_SIZE);

  sares->message_type = SERVER_AREA_RESPONSE;
  sares->id = htonl(id);
  sares->server_ip = sr->ip; // don't have to include htonl because sr->ip is already network-byte order
  sares->tcpport = htons(sr->tcp_port);
  sares->max_x = sr->max_x;
  sares->max_y = sr->max_y;
  sares->min_x = sr->min_x;
  sares->min_y = sr->min_y;

  unsigned char*tosent = (unsigned char *) sares;
  printMessage(tosent,SERVER_AREA_RESPONSE_SIZE);
  int i;
  for(i = 0; i < SERVER_AREA_RESPONSE_SIZE;i++){
    buffer[i] = tosent[i];
  }
  free(tosent);
}