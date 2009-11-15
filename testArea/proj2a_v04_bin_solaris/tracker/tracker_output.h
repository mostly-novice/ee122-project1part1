#ifndef TRACKER_OUTPUT
#define TRACKER_OUTPUT

void on_malformed_udp(int type){
  switch(type) {
  case 1:
    fprintf(stdout,"Received malformed packet: wrong size.\n");
    break;
  case 2:
    fprintf(stdout,"Received malformed packet: undefined type.\n");
    break;
  default:
    fprintf(stdout,"Error: Undefined malformed type.\n");
    exit(0);
  }

  fflush(stdout);
}

void on_udp_duplicate(unsigned int ipaddr) {
  fprintf(stdout,"Received duplicate packet from: %x.\n",ipaddr);
  fflush(stdout);
}

#endif
