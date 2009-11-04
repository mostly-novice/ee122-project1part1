// Helper to send messages.
void createlsrespond(server_record * sr, int id, char buffer[]){
  storage_location_response* slr = (storage_location_response*) malloc (sizeof(storage_location_response));

  slr->message_type = STORAGE_LOCATION_RESPONSE;
  slr->id = htons(id);
  slr->server_ip = sr->server_ip;
  slr->port = htons(sr->port);

  buffer = (unsigned char *) slr;
}

// Creating the SERVER_AREA_RESPOND package
void createsarespond(server_record * sr, int id,char buffer[]){
  server_area_response* sares = (server_area_response*) malloc (sizeof(server_area_response));

  sares->message_type = SERVER_AREA_RESPONSE;
  sares->id = htons(id);
  sares->server_ip = sr->server_ip;
  sares->port = htons(sr->port);
  sares->max_x = max_x;
  sares->max_y = max_y;
  sares->min_x = min_x;
  sares->min_y = min_y;
  sares->padding = 0x0;

  buffer = (unsigned char *) sares;
}
