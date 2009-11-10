int ceiling(double x){
  return ((int)x)+1;
}

int initsr(server_record ** sr_array, char * configpath){
  printf("file:%s\n",configpath);
  FILE * file = fopen("server.config","r");
  char server_ip[30];
  int tcp_port;
  int udp_port;

  int count = 0;
  // Read every line in file
  while(fscanf(file,"%s %d %d", server_ip, &tcp_port, &udp_port)!= EOF){   
    printf("%d\n",inet_addr(server_ip));
    server_record * newrecord = (server_record *) malloc(sizeof(server_record));
    newrecord->ip = inet_addr(server_ip);
    newrecord->tcp_port = tcp_port;
    newrecord->udp_port = udp_port;
    newrecord->min_y = 0;
    newrecord->max_y = 99;
    sr_array[count] = newrecord;
    count++;
  }

  printf("count\n");

  // NOTE: This is different than the spec.
  int i;
  for(i=0;i<count;i++){
    sr_array[i]->min_x = 99/count*i;
    if(i==count-1){
      sr_array[i]->max_x = 99;
    } else {
      sr_array[i]->max_x = (99/count)*(i+1)-1;
    }

    printf("server_ip:%d\n", sr_array[i]->ip);
    printf("tcp_port:%d\n", sr_array[i]->tcp_port);
    printf("udp_port:%d\n", sr_array[i]->udp_port);
    printf("min_x:%d\n", sr_array[i]->min_x);
    printf("max_x:%d\n", sr_array[i]->max_x);
  }

  // Closing the file
  fclose(file);
  return count;
}

void printMessageRecord(message_record ** mr_array){
  int i;
  for(i = 0; i < 50;i++){
    if(mr_array[i]){
      fprintf(stdout,"Slot %d: ID:%d IP:%x\n",i,ntohl(mr_array[i]->id),ntohl(mr_array[i]->ip));
    }
  }
}
