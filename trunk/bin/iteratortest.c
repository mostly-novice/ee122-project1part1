struct p {
  int x;
  int y;
};

void main(){
  struct p list[3];
  struct p p1;
  p1.x = 1;
  p1.y = 1;
  struct p p2;
  p2.x = 2;
  p2.y = 2;
  struct p p3;
  p3.x = 3;
  p3.y = 3;

  list[0] = p1;
  list[1] = p2;
  list[2] = p3;

  
  //for(q = list; q; q++){
    //printf("x:%d", ((struct p*)q)->x);
    //}
}
