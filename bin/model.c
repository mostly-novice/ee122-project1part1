#include <stdio.h>

struct Location {
  int x;
  int y;
};

struct Player {
  char name[80];
  int hp;
  int exp;
  Location *location;
};