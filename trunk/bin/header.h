struct header {
  uint8_t version;
  uint8_t l1;
  uint8_t len;
  uint8_t msgtype;
}__attribute__((packed));

// Payload Type
struct login_request {
  unsigned char name[80];
  uint16_t padding;
}__attribute__((packed));


struct login_reply {
  uint8_t error_code;
  uint32_t hp;
  uint32_t exp;
  uint8_t x;
  uint8_t y;
  uint8_t padding;
}__attribute__((packed));

struct move_notify {
  char name[80];
  uint8_t x;
  uint8_t y;
  uint32_t hp;
  uint32_t exp;
}__attribute__((packed));


struct move {
  uint8_t direction;
  char padding[24];
}__attribute__((packed));

struct attack {
  char victimname[80];
  char padding[16];
}__attribute__((packed));

struct attack_notify {
  unsigned char attacker_name[80];
  unsigned char victim_name[80];
  uint8_t damage;
  uint32_t hp; // Specifies the updated HP of the victim in network-byte-order. Cannot be negative
  char padding[24];
}__attribute__((packed));

struct speak {
  char * msg;
  char * padding;
}__attribute__((packed));

struct speak_notify {
  char broastcaster[80];
  char * msg;
}__attribute__((packed));
