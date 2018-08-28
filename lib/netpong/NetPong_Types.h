/******************************************************************************
 * Type definitions for the NetPong game.                                     *
 *                                                                            *
 * Author: Nicolas Himmelmann                                                 *
 ******************************************************************************/
 
#include <stdint.h>
 
// This enum defines the possible joystick positions
typedef enum en_joystick_pos { POS_LEFT3, POS_LEFT2, POS_LEFT1, POS_MIDDLE, POS_RIGHT1, POS_RIGHT2, POS_RIGHT3, POS_TOP, POS_BOTTOM } en_joystick_pos_t;

// This enum defines the possible game states
typedef enum en_game_state_t { MENU, WAITING, IN_GAME, DATA_TIMEOUT } game_state_t;

typedef enum en_ball_state_t { BALL_ON_SCREEN, BALL_OFF_SCREEN } ball_state_t;

// This enum defines the possible connection states
typedef enum en_connection_state_t { NOT_CONNECTED, WAITING_FOR_CONNECTION, CONNECTING_TO_HOST, CONNECTED } connection_state_t;

/***********************
 * Payload definitions *
 ***********************/
typedef struct 
{
    uint16_t ball_x;
    int8_t ball_speed_x;
    int8_t ball_speed_y;
    uint8_t changeScreen;
    uint8_t otherPlayerLost;
       
} netpong_payload_ingame_t;

typedef struct 
{
    uint16_t gameId;
    uint8_t isHost;
    uint8_t reserved;
       
} netpong_payload_conn_req_t;

typedef struct
{
    uint8_t gameId;
    uint8_t reserved;
    
} netpong_payload_conn_ack_t;


/**
 * Speed (or velocity) structure.
 *  
 * x, y: The vectorial components in x and y direction of the speed the bar is moving at.
 *       Valid values are between -10 and 10.
 */
typedef struct 
{
    int8_t x, y;
} speed_t;

/**
 * 
 * x, y: Player position in cartesian coordinates.
 * barWidth: The current width of the bar in pixels
 *
 
 */
typedef struct
{
    uint16_t x, y;
    uint8_t barWidth;
    
    speed_t speed;
    
} player_t;

typedef struct
{
    uint16_t x, y;
    //Ball speed
    speed_t speed;
    
} ball_t;

typedef struct
{
    uint8_t btnHostSelected;
    uint8_t btnGuestSelected;
    
} game_menu_t;

typedef struct 
{
    game_state_t state;
    connection_state_t connection;
    ball_state_t ballState;
    uint8_t isHost;
    
    game_menu_t gameMenu;

    player_t player;
    ball_t ball;
    en_joystick_pos_t joyPos;
    
    uint8_t score_local;
    uint8_t score_remote;
    
    uint16_t currentGameId;
    
    volatile uint8_t thirdSecCount;
    volatile uint32_t seconds;
    volatile uint32_t secondsLastTX;
	  //volatile uint8_t watchForTx;
	  netpong_payload_ingame_t lastPayload;
    
} netpong_game_t;