/******************************************************************************
 * NetPong game header file                                                   *
 * A simple pong game which can be played over a connection link              *
 * (such as infrared)                                                         *
 *                                                                            *
 * Which player is allowed to start is determined by a random number that is  *
 * sent during connection establishment.                                      *
 *                                                                            *
 * Connection establishment procedure:                                        *
 * - Host is waiting for connection from a guest.                             *
 *                                                                            *
 * Needed hardware:                                                           *
 * - Base timer 3                                                             *
 * Author: Nicolas Himmelmann                                                 *
 ******************************************************************************/

#ifndef NET_PONG_H
#define NET_PONG_H
#include "s6e2ccxj.h" 
#include "netpong/NetPong_Types.h"

#define NETPONG_SECONDS_COUNTER BT0

#define NETPONG_WIDTH 480
#define NETPONG_HEIGHT 320

#define NETPONG_VELOCITY_MULTIPLIER 2

#define NETPONG_CONNECTION_TIMEOUT_SEC 30 

#define NETPONG_DEFAULT_BAR_WIDTH 150
#define NETPONG_DEFAULT_BAR_HEIGHT 10
#define NETPONG_MOVEMENT_PX 10
#define NETPONG_BALL_RADIUS 10


/**
 * Initializes the NetPong game. 
 *
 */
void NetPong_initPlayer(void);
void NetPong_initBall(void);
uint8_t NetPong_init(void(*funcPtrPhyTx)(uint8_t * data, uint16_t bufLen));

/**
 * Runs the game logic and all necessary stuff.
 * Call this regularly in an endless loop.
 */ 
void NetPong_main(void);
uint16_t NetPong_getRandomSeed(void);

/*** Menu Logic ***/
void NetPong_changeMenu(void);
void NetPong_drawMenu(void);

/*** Connection Logic ***/
void NetPong_sendStruct(const uint8_t * structPtr, uint8_t structSize);
void NetPong_manageConnection(void);
void NetPong_newDataFromRemote(uint8_t * data, uint16_t length);
void NetPong_sendInGamePayload(uint8_t we_lost);
void NetPong_checkTimeout(void);

/*** Game Logic ***/
void NetPong_movePlayer(void);
void NetPong_moveBall(void);

/*** Seconds Timer Logic ***/
//void NetPong_secondsCounterCb(void);
void NetPong_initSecondsCounter(void);
void NetPong_startSecondsCounter(void);

/*** Drawing Logic ***/
void NetPong_drawConnectionScreen(void);
void NetPong_drawBar(void);
void NetPong_drawBall(void);
void NetPong_drawScoreLine(void);
void NetPong_writeSeconds(void);


/*** Input Logic ***/
en_joystick_pos_t NetPong_getJoystickPosX(void);
en_joystick_pos_t NetPong_getJoystickPosY(void);
uint8_t NetPong_isLeftJoystickPressed(void);

#endif
