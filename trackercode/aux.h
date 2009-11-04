#include <math.h>

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
  while(fscanf(file,"%s %d %d", server_ip, &tcp_port, &udp_port)!= EOF){   
    unsigned int a1,a2,a3,a4;
    unsigned int ip = (a1 << 24)+(a1 << 16)+(a1 << 8)+a4;
    sscanf(server_ip,"%d.%d.%d.%d",&a1,&a2,&a3,&a4);
    printf("%x.%x.%x.%x\n",a1,a2,a3,a4);
    printf("%x\n",ip);
    server_record * newrecord = (server_record *) malloc(sizeof(server_record));
    newrecord->ip = ip;
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
    sr_array[i]->min_x = ceiling(99.0/count)*i;
    sr_array[i]->max_x = ceiling(99.0/count)*(i+1)-1;

    /* printf("server_ip:%d\n", sr_array[i]->ip); */
/*     printf("tcp_port:%d\n", sr_array[i]->tcp_port); */
/*     printf("udp_port:%d\n", sr_array[i]->udp_port); */
/*     printf("min_x:%d\n", sr_array[i]->min_x); */
/*     printf("max_x:%d\n", sr_array[i]->max_x); */
  }

  // Closing the file
  fclose(file);
  return count;
}
