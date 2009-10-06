struct header {
  unsigned char version;
  unsigned short len;
  unsigned char msgtype;
}__attribute__((packed));

// Payload Type
struct login_request {
  unsigned char name[10];
  unsigned char padding[2];
}__attribute__((packed));

struct logout_reply{
  unsigned  char name [10];
  unsigned char padding[2];
}__attribute__((packed));

struct login_reply {
  unsigned char error_code;
  unsigned int hp;
  unsigned int exp;
  unsigned char x;
  unsigned char y;
  unsigned char padding;
}__attribute__((packed));

struct move_notify {
  unsigned char name[10];
  unsigned char x;
  unsigned char y;
  unsigned int hp;
  unsigned int exp;
}__attribute__((packed));


struct move {
  unsigned char direction;
  unsigned char padding[3];
}__attribute__((packed));

struct attack {
  char victimname[10];
  char padding[2];
}__attribute__((packed));

struct attack_notify {
  unsigned char attacker_name[10];
  unsigned char victim_name[10];
  unsigned char damage;
  unsigned int hp; // Specifies the updated HP of the victim in network-byte-order. Cannot be negative
  char padding[3];
}__attribute__((packed));

struct speak {
  char * msg;
  char * padding;
}__attribute__((packed));

struct speak_notify {
  unsigned char broadcaster[10];
  unsigned char * msg;
}__attribute__((packed));

struct invalid_state {
  unsigned char error_code;
}__attribute__((packed));
