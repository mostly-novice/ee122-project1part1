struct header {
				unsigned char version;
				unsigned short len;
				unsigned char msgtype;
}__attribute__((packed));

struct p2p_join_request {
				unsigned char MsgVer;
				unsigned short MsgLength;
				unsigned char MsgType;
				unsigned int P2P_ID;
}__attribute__((packed));

struct p2p_join_response {
				unsigned char MsgVer;
				unsigned short MsgLength;
				unsigned char MsgType;
				unsigned int UserNumber;
				unsigned char* UserDataArray;
}__attribute__((packed));

struct p2p_bkup_request {
				unsigned char MsgVer;
				unsigned short MsgLength;
				unsigned char MsgType;
				unsigned char UserData[20];
}__attribute__((packed));

struct p2p_bkup_response {
				unsigned char MsgVer;
				unsigned short MsgLength;
				unsigned char MsgType;
				unsigned char ErrorCode;
}__attribute__((packed));


