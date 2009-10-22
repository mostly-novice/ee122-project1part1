/**************************************************************************
 * The reference implementation of the EE122 Project #1
 *
 * Author: Junda Liu (liujd@cs)
 * Author: DK Moon (dkmoon@cs)
 * Author: David Zats (dzats@cs)
**************************************************************************/

#ifndef _constants_h_
#define _constants_h_

#define EE122_VALID_VERSION 4
#define EE122_MAX_NAME_LENGTH 9
#define EE122_SIZE_X 100
#define EE122_SIZE_Y 100
#define EE122_VISION_RANGE 10
#define EE122_MAX_MSG_LENGTH 255

enum messages {
  LOGIN_REQUEST = 1,
  LOGIN_REPLY,
  MOVE,
  MOVE_NOTIFY,
  ATTACK,
  ATTACK_NOTIFY,
  SPEAK,
  SPEAK_NOTIFY,
  LOGOUT,
  LOGOUT_NOTIFY,
  INVALID_STATE,

  MAX_MESSAGE,
};

enum direction {NORTH = 0, SOUTH, EAST, WEST};

#endif /* _constants_h_ */
