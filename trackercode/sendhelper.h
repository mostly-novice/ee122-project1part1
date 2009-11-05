// Helper to send messages.
void createslrespond(server_record * sr, int id, char buffer[]){
  struct storage_location_response* slr = (struct storage_location_response*) malloc (sizeof(struct storage_location_response));

  slr->message_type = STORAGE_LOCATION_RESPONSE;
  slr->id = htons(id);
  slr->server_ip = sr->ip;
  slr->udpport = htons(sr->udp_port);

  printf("id:%d\n",sr->ip);
  printf("port:%d\n",sr->udp_port);

  unsigned char*tosent = (unsigned char *) slr;
  printMessage(slr,STORAGE_LOCATION_RESPONSE);
  int i;
  for(i = 0; i < STORAGE_LOCATION_RESPONSE;i++){
    buffer[i] = tosent[i];
  }
  free(slr);
}

// Creating the SERVER_AREA_RESPONSE package
void createsarespond(server_record * sr, int id,char buffer[]){
  struct server_area_response* sares = (struct server_area_response*) malloc (sizeof(struct server_area_response));

  sares->message_type = SERVER_AREA_RESPONSE;
  sares->id = htons(id);
  sares->server_ip = sr->ip;
  sares->tcpport = htons(sr->tcp_port);
  sares->max_x = sr->max_x;
  sares->max_y = sr->max_y;
  sares->min_x = sr->min_x;
  sares->min_y = sr->min_y;
  buffer = (unsigned char *) sares;
}
