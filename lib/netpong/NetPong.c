#include <string.h>
#include "NetPong.h"
#include "pdl_user.h"
#include "bt/bt.h" 
#include "cppp_board/cppp_gfx.h"
#include "cppp_board/cppp_lcd.h"
#include "cppp_board/joystick.h"
#include "framebuffer/framebuffer.h"

netpong_game_t __netPong;
void (* __phy_send)(uint8_t * data, uint16_t bufLen);

//Structs for Seconds Timer config
static stc_bt_rt_config_t stcSecsCounterCfg;
static stc_rt_irq_en_t stcIrqEn;
static stc_rt_irq_cb_t stcIrqCb;

/********************************************************************
 *                     Initialization functions                     *
 ********************************************************************/

void NetPong_initPlayer() {
    __netPong.player.barWidth = NETPONG_DEFAULT_BAR_WIDTH;
    __netPong.player.x = (NETPONG_WIDTH - __netPong.player.barWidth) / 2;
    __netPong.player.y = NETPONG_DEFAULT_BAR_HEIGHT + 5;
    __netPong.player.speed.x = 0;
    __netPong.player.speed.y = 0;
}

void NetPong_initBall() {
    __netPong.ball.x = NETPONG_WIDTH/2;
    __netPong.ball.y = NETPONG_HEIGHT/2;
    __netPong.ball.speed.x = 0;
    __netPong.ball.speed.y = -6;
    __netPong.ballState = BALL_ON_SCREEN;
}

uint8_t NetPong_init(void(*funcPtrPhyTx)(uint8_t * data, uint16_t bufLen)) {
    if(funcPtrPhyTx == NULL) { return 0; }
		
    __phy_send = funcPtrPhyTx;
    
    srand(NetPong_getRandomSeed());
    NetPong_initSecondsCounter();

    __netPong.state = MENU;
    __netPong.connection = NOT_CONNECTED;
    __netPong.isHost = 0;
    __netPong.gameMenu.btnHostSelected = 1;
    __netPong.gameMenu.btnGuestSelected = 0;
    
    __netPong.score_local = 0;
    __netPong.score_remote = 0;
    
    __netPong.currentGameId = 0;
    
    __netPong.thirdSecCount = 0;
    __netPong.seconds = 0;
    __netPong.secondsLastTX = 0;
    
    NetPong_initPlayer();
    NetPong_initBall();
    
    //Initialize PF5 (pushbutton of left joystick) as input and activate pullup resistor
    Gpio1pin_InitIn(GPIO1PIN_PF5, Gpio1pin_InitPullup(1u));
    
    return 1;
}

uint16_t NetPong_getRandomSeed() {
    //Generate random value by reading
    //the last bit of the analog channels
    uint16_t random = 0;
    uint8_t a, b, c, d, e, f, g;
    for(int i=0; i<2; ++i) 
    {
        random = random << 7;
        cppp_getAnalogValues(&a, &b, &c, &d, &e, &f, &g);  
        random |= (a&1) << 0 | (b&1) << 1 | (c&1) << 2 | (d&1) << 3 | (e&1) << 4 | (f&1) << 5 | (g&1) << 6;
    }
    return random;
}

void NetPong_main() {
    fb_fill(0);
    NetPong_writeSeconds();
    
    switch(__netPong.state)
    {
        case IN_GAME:
            //Execute game logic
				    NetPong_checkTimeout();
						
            //Execute bar movement
            NetPong_movePlayer();
            NetPong_drawBar();
            
            //Only call ball movement if ball is on screen
            if(__netPong.ballState == BALL_ON_SCREEN)
            {
                NetPong_moveBall();
                NetPong_drawBall();
            }
            
            NetPong_drawScoreLine();
        break;
        
        case MENU: 
            //Execute menu logic
            NetPong_changeMenu(); 
            NetPong_drawMenu();
        break;
        
        case WAITING: 
            //Execute connection logic
            NetPong_drawConnectionScreen();
            //Manage connection every 2 seconds
            if( (__netPong.seconds - __netPong.secondsLastTX) > 5)
            {
                NetPong_manageConnection();
            }
        break;
        
        case DATA_TIMEOUT:
        { 
            char timeoutText[29] = "Your connection timed out...";
            fb_writeTextCentered(200, timeoutText, 0x1F, 0x00, 4);
        }
        break;
    }
    
    fb_sendToDisplay();
}

/********************************************************************
 *                            Menu Logic                            *
 ********************************************************************/
void NetPong_changeMenu() {
    en_joystick_pos_t joyPos = NetPong_getJoystickPosY();

    if( (__netPong.gameMenu.btnHostSelected == 1) && (joyPos == POS_BOTTOM) )
    {
        __netPong.gameMenu.btnHostSelected = 0;
        __netPong.gameMenu.btnGuestSelected = 1;
    }
    if( (__netPong.gameMenu.btnGuestSelected == 1) && (joyPos == POS_TOP) )
    {
        __netPong.gameMenu.btnHostSelected = 1;
        __netPong.gameMenu.btnGuestSelected = 0;
    }
    
    if(NetPong_isLeftJoystickPressed())
    {
        if(__netPong.gameMenu.btnHostSelected == 1)
        {
            __netPong.isHost = 1;
            __netPong.state = WAITING;
            __netPong.connection = WAITING_FOR_CONNECTION;
						NetPong_startSecondsCounter();
        }
        if(__netPong.gameMenu.btnGuestSelected == 1)
        {
            __netPong.isHost = 0;
            __netPong.currentGameId = rand();
            __netPong.state = WAITING;
            __netPong.connection = CONNECTING_TO_HOST;
						NetPong_startSecondsCounter();
        }
        
        return;
    }
}
void NetPong_drawMenu() {
    
    char welcomeText[13] = "netPong v0.9";
    fb_writeTextCentered(250, welcomeText, 0x1F, 0x00, 4);
   
    char question[33] = "Do you want to be host or guest?";
    fb_writeTextCentered(220, question, 0x1F, 0x00, 2);
    
    
    char txtBtnHost[5] = "Host";
    char txtBtnGuest[6] = "Guest";
    
    if(__netPong.gameMenu.btnHostSelected == 1)
    {
        fb_drawButtonCentered(140, txtBtnHost, 0x1F, 0x05, 0x15, 3);
        fb_drawButtonCentered(80, txtBtnGuest, 0x1F, 0x41, 0x15, 3);
    }
    else if(__netPong.gameMenu.btnGuestSelected == 1)
    {
        fb_drawButtonCentered(140, txtBtnHost, 0x1F, 0x41, 0x15, 3);
        fb_drawButtonCentered(80, txtBtnGuest, 0x1F, 0x05, 0x15, 3);
    }
}


/********************************************************************
 *                         Connection Logic                         *
 ********************************************************************/
void NetPong_sendStruct(const uint8_t * structPtr, uint8_t structSize) {
    uint8_t * buffer = (uint8_t*)malloc(structSize);
    memcpy(buffer, structPtr, structSize);
    __phy_send(buffer, structSize);
		
	  //Remember seconds time of this tx,
		//to be able to handle retransmissions later
	  __netPong.secondsLastTX = __netPong.seconds;
}
 
 void NetPong_manageConnection() {
    if(__netPong.isHost == 0)
    {
        //Need to send a connection packet to the host regularly, 
        //until an acknowledgement is received.
        netpong_payload_conn_req_t reqPayload = { 
            .isHost = 0,
            .gameId = __netPong.currentGameId
        }; 
        NetPong_sendStruct(
            (const uint8_t *)&reqPayload, 
            (uint8_t)sizeof(netpong_payload_conn_req_t));
        __netPong.state = IN_GAME;
        __netPong.connection = CONNECTED;
        __netPong.ballState = BALL_OFF_SCREEN;
    }
}

void NetPong_sendConnAck() {
    netpong_payload_conn_ack_t ackPayload = {
        .gameId = __netPong.currentGameId
    };
    NetPong_sendStruct(
        (const uint8_t *)&ackPayload, 
        (uint8_t)sizeof(netpong_payload_conn_ack_t));
}

void NetPong_newDataFromRemote(uint8_t * data, uint16_t length) {
    
    if( (length == sizeof(netpong_payload_ingame_t)) && (__netPong.state == IN_GAME) )
    {
        //Received an in-game payload.
        
        //Cast received bytes to netpong_payload_ingame_t struct.
        netpong_payload_ingame_t * payload = (netpong_payload_ingame_t *)data;
        
			  uint8_t validPayload = 0;
			
        //Check if other player lost this round
        if( (__netPong.ballState == BALL_OFF_SCREEN) && (payload->otherPlayerLost == 1) )
        {
            __netPong.score_local++;
            //Reset ball, since I won and am allowed to start the new round
            NetPong_initBall();
            //Reset me to the middle
            NetPong_initPlayer();
            //Change ball state back to "on screen"
            __netPong.ballState = BALL_ON_SCREEN;
					  
					  validPayload = 1;
        }
        
        //Check if we need to change screen to ours
        if( (__netPong.ballState == BALL_OFF_SCREEN) && (payload->changeScreen == 1) )
        {
            //Correct too high x coordinate
            if(payload->ball_x > NETPONG_WIDTH) { payload->ball_x = NETPONG_WIDTH - NETPONG_BALL_RADIUS-1; }
            //Invert x coordinate
            __netPong.ball.x = NETPONG_WIDTH - payload->ball_x;
            //Set y coordinate
            __netPong.ball.y = NETPONG_HEIGHT - NETPONG_BALL_RADIUS-1;
            //Invert velocities
            __netPong.ball.speed.x = -(payload->ball_speed_x);
            __netPong.ball.speed.y = -(payload->ball_speed_y);
            //Change ball state back to "on screen"
            __netPong.ballState = BALL_ON_SCREEN;
						
					  validPayload = 1;
        }
				
				//Avoid retransmit if payload was valid
				if(validPayload) {
			    __netPong.secondsLastTX = __netPong.seconds;
				}
        
        return;
    }
    
    if( (length == sizeof(netpong_payload_conn_req_t)) && (__netPong.connection == WAITING_FOR_CONNECTION) && (__netPong.isHost == 1) )
    {
        //Received a connection request payload from a client
        
        //Cast received bytes to netpong_payload_conn_req_t struct.
        netpong_payload_conn_req_t * payload = (netpong_payload_conn_req_t *)data;
        
        //Only accept connections from guests
        if( payload->isHost == 0 )
        {   
            __netPong.state = IN_GAME;
            __netPong.connection = CONNECTED;
            __netPong.ballState = BALL_ON_SCREEN;
            __netPong.currentGameId = payload->gameId;
        }
        
        return;
    }   

    if( (length == sizeof(netpong_payload_conn_ack_t)) && (__netPong.connection == CONNECTED) && (__netPong.ballState == BALL_OFF_SCREEN) )
    {
        //Received an acknowledgement for a payload we sent earlier
    }
    
    if( (length == sizeof(netpong_payload_conn_ack_t)) && (__netPong.connection == CONNECTING_TO_HOST) && (__netPong.isHost == 0) )
    {
        //Received a connection acknowledgement payload from the host
        
        //Cast received bytes to netpong_payload_conn_ack_t struct.
        netpong_payload_conn_ack_t * payload = (netpong_payload_conn_ack_t *)data;
        
        //Check the game id
        if(payload->gameId == __netPong.currentGameId)
        {
            //Game id is correct. We can start
            __netPong.state = IN_GAME;
            __netPong.connection = CONNECTED;
            __netPong.ballState = BALL_OFF_SCREEN;
        }
    }
    
}

void NetPong_sendInGamePayload(uint8_t we_lost) {
    netpong_payload_ingame_t payload;
    if(we_lost == 0)
    {
        payload.ball_x = __netPong.ball.x;
        payload.ball_speed_x = __netPong.ball.speed.x;
        payload.ball_speed_y = __netPong.ball.speed.y;
        payload.changeScreen = 1;
        payload.otherPlayerLost = 0;
			  
			  //Remember this payload for eventual retransmission
			  __netPong.lastPayload = payload;
    }
    else
    {   
        //Other fields are 0 by default
        payload.otherPlayerLost = 1;
    }
    NetPong_sendStruct(
        (const uint8_t *)&payload, 
        (uint8_t)sizeof(netpong_payload_ingame_t));
}

void NetPong_checkTimeout(void) {
    //Retransmit last payload, if we have a timeout
	  if( (__netPong.seconds - __netPong.secondsLastTX) > 3)
		{
		    NetPong_sendStruct(
            (const uint8_t *)&(__netPong.lastPayload), 
            (uint8_t)sizeof(netpong_payload_ingame_t));
		}
}
 

/********************************************************************
 *                            Game Logic                            *
 ********************************************************************/

void NetPong_movePlayer_check(uint8_t multiplier, char addOrSub) {
    //Checks, if the desired player movement in pixels is allowed
    //(e.g. if the number of pixels to add or subtract would be negative or too large)
    
    int16_t newValue = NETPONG_MOVEMENT_PX + multiplier*NETPONG_VELOCITY_MULTIPLIER;
    if(addOrSub == '-')
    {    
        if(newValue <= __netPong.player.x)
        {
            __netPong.player.x -= (uint16_t)newValue;
            __netPong.player.speed.x = (-1)*multiplier*NETPONG_VELOCITY_MULTIPLIER;
            return;
        }
    }
    else if(addOrSub == '+')
    {
        if( (__netPong.player.x + newValue) < (NETPONG_WIDTH - __netPong.player.barWidth) )
        {
            __netPong.player.x += (uint16_t)newValue;
            __netPong.player.speed.x = multiplier*NETPONG_VELOCITY_MULTIPLIER;
            return;
        }
    }
    __netPong.player.speed.x = 0;
} 
void NetPong_movePlayer() {
    en_joystick_pos_t curJoyPos = NetPong_getJoystickPosX();
    
    switch(curJoyPos)
    {
        case POS_LEFT1:
        case POS_LEFT2:
        case POS_LEFT3:
            //Only allow movement if we are not at the left screen side.
            if(__netPong.player.x > 0)
            {
                if(curJoyPos == POS_LEFT1)
                {
                    NetPong_movePlayer_check(1, '-');
                    break;
                }
                
                if(curJoyPos == POS_LEFT2)
                {
                    NetPong_movePlayer_check(5, '-');
                    break;
                }
                
                if(curJoyPos == POS_LEFT3)
                {
                    NetPong_movePlayer_check(10, '-');
                    break;
                }
            }
            else 
            {
                __netPong.player.speed.x = 0;
            }
        break;
        
        case POS_RIGHT1:
        case POS_RIGHT2:
        case POS_RIGHT3:
            //Only allow movement if we are not at the right screen side.
            if( __netPong.player.x < (NETPONG_WIDTH - __netPong.player.barWidth) )
            {
                if(curJoyPos == POS_RIGHT1)
                {
                    NetPong_movePlayer_check(1, '+');
                    break;
                }
                if(curJoyPos == POS_RIGHT2)
                {
                    NetPong_movePlayer_check(5, '+');
                    break;
                }
                if(curJoyPos == POS_RIGHT3)
                {
                    NetPong_movePlayer_check(10, '+');
                    break;
                }
            }
            else 
            {
                __netPong.player.speed.x = 0;
            }
        break;
        
        case POS_MIDDLE:
            __netPong.player.speed.x = 0;
        break;
				
				case POS_TOP:
				case POS_BOTTOM:
				break;
    }
}

void NetPong_moveBall() {
    //nextX = oldX + v_x*T
    int16_t nextX = __netPong.ball.x + __netPong.ball.speed.x*NETPONG_VELOCITY_MULTIPLIER;
    
    //nextY = oldY + v_y*T
    int16_t nextY = __netPong.ball.y + __netPong.ball.speed.y*NETPONG_VELOCITY_MULTIPLIER;
    
    //Check collision left or right
    if(nextX <= NETPONG_BALL_RADIUS || nextX >= (NETPONG_WIDTH - NETPONG_BALL_RADIUS) )
    {
        //In both cases: Change sign of velocity x component
        __netPong.ball.speed.x = -__netPong.ball.speed.x;
        nextX = (nextX <= NETPONG_BALL_RADIUS) ? (NETPONG_BALL_RADIUS) : (NETPONG_WIDTH - NETPONG_BALL_RADIUS);
    }
    
    //Check collision top --> Means we have to change the screen
    if(nextY >= NETPONG_HEIGHT - NETPONG_BALL_RADIUS-1)
    {
        NetPong_sendInGamePayload(0);
        __netPong.ballState = BALL_OFF_SCREEN;
    }
    
    //Check collision bottom
    //Case 1: Colliding with bar
    if( (nextY <= __netPong.player.y + NETPONG_BALL_RADIUS) && (nextX >= __netPong.player.x) && (nextX <= __netPong.player.x+__netPong.player.barWidth) )
    {
        //Change sign of both velocities
        //Only change x velocity if bar is moving differently
        if( ((__netPong.player.speed.x > 0) && (__netPong.ball.speed.x <= 0)) || ((__netPong.player.speed.x < 0) && (__netPong.ball.speed.x >= 0)) )
        { 
            __netPong.ball.speed.x = -__netPong.ball.speed.x;
            if(__netPong.ball.speed.x == 0)
            {
                __netPong.ball.speed.x = __netPong.player.speed.x;
            }
        }
        __netPong.ball.speed.y = -__netPong.ball.speed.y;
        
        //Calculate new nextX
        nextX = __netPong.ball.x + __netPong.ball.speed.x*NETPONG_VELOCITY_MULTIPLIER;
    }
    
    //Case 2: Missed the bar
    if( (nextY <= __netPong.player.y) && ((nextX <= __netPong.player.x) || (nextX >= __netPong.player.x+__netPong.player.barWidth)) )
    {
        //Update score of remote player
        __netPong.score_remote++;
        //Send packet that we lost
        NetPong_sendInGamePayload(1);
        //Other player has won --> Allowed to start the new round
        __netPong.ballState = BALL_OFF_SCREEN;
        //Reset me to the middle
        NetPong_initPlayer();
        return;
    }
        
    __netPong.ball.x = nextX;
    __netPong.ball.y = nextY;
}

/********************************************************************
 *                      Seconds Timer Logic                         *
 ********************************************************************/
static void NetPong_secondsCounterCb(void) {
    __netPong.thirdSecCount++;
    if(__netPong.thirdSecCount == 3) 
    {
        //Increase the game seconds counter by 1.
        __netPong.seconds++;
        __netPong.thirdSecCount = 0;
    }
}
 
void NetPong_initSecondsCounter() {
    //Disable timer (only for precaution)
    Bt_Rt_DisableCount(&NETPONG_SECONDS_COUNTER);
    
    stcSecsCounterCfg.pstcRtIrqEn = &stcIrqEn;
    stcSecsCounterCfg.pstcRtIrqCb = &stcIrqCb;
    //Set timer I/O mode (simple counter)
    Bt_ConfigIOMode(&NETPONG_SECONDS_COUNTER, BtIoMode0);
    
    //Initialize Reload function of BT
    stcSecsCounterCfg.enPres = RtPres1Div2048; //Timer clock prescaler of 2048 at 200MHz: 
                                                  //Results in resolution of 10.24us per tick)
    stcSecsCounterCfg.enSize = RtSize16Bit; //Timer size of 16bit
    stcSecsCounterCfg.enMode = RtReload; //Configure timer as reload
    stcSecsCounterCfg.enExtTrig = RtExtTiggerDisable; //Disable external trigger
    stcSecsCounterCfg.enOutputPolarity = RtPolarityLow;
    
    //Configure interrupt structures
    stcSecsCounterCfg.pstcRtIrqEn->bRtTrigIrq = 0;
    stcSecsCounterCfg.pstcRtIrqEn->bRtUnderflowIrq = 1; //Enable interrupt on underflow
    stcSecsCounterCfg.pstcRtIrqCb->pfnRtUnderflowIrqCb = NetPong_secondsCounterCb; //Set callback routine
    stcSecsCounterCfg.bTouchNvic = TRUE;
    
    Bt_Rt_Init(&NETPONG_SECONDS_COUNTER, &stcSecsCounterCfg);
} 
 
void NetPong_startSecondsCounter() {
    //Disable timer (only for precaution)
    Bt_Rt_DisableCount(&NETPONG_SECONDS_COUNTER);
    //Write start value (Generate overflow after 0.333 seconds)
    Bt_Rt_WriteCycleVal(&NETPONG_SECONDS_COUNTER, 32552);
    //Enable interrupt
    //Bt_Rt_EnableIrq(&NETPONG_SECONDS_COUNTER, RtUnderflowIrq);
    //Enable count operation
    Bt_Rt_EnableCount(&NETPONG_SECONDS_COUNTER);
    //Start timer by software
    Bt_Rt_EnableSwTrig(&NETPONG_SECONDS_COUNTER);
}
 
/********************************************************************
 *                         Drawing Logic                            *
 ********************************************************************/
void NetPong_drawConnectionScreen() {
    char txtWelcome[13] = "netPong v0.9";
    fb_writeTextCentered(250, txtWelcome, 0x1F, 0x00, 4);

    char txtQuestion[14] = "Connecting...";
    fb_writeTextCentered(120, txtQuestion, 0x1F, 0x00, 3);
       
}

void NetPong_drawBar() {
    fb_fillRectangle(__netPong.player.x, 
             __netPong.player.y, 
             __netPong.player.barWidth, 
             NETPONG_DEFAULT_BAR_HEIGHT, 
             0x1F);
}

void NetPong_drawBall() {
    
    fb_fillCircle(__netPong.ball.x, 
             __netPong.ball.y, 
             NETPONG_BALL_RADIUS, 
             0x1F);
}

void NetPong_drawScoreLine() {
    //Draw a rectangle at the top containing the scores.
    fb_drawRectangle(0, NETPONG_HEIGHT-23-1, NETPONG_WIDTH-2-1, 21, 1, 0x1F);
    
    char txtYourScore[9] = "You: ";
    sprintf(txtYourScore+5, "%d", __netPong.score_local);
    fb_writeText(2, NETPONG_HEIGHT-5-2*8, txtYourScore, 0x1F, 0x00, 2);
    
    char txtRemoteScore[11] = "Rival: ";
    sprintf(txtRemoteScore+7, "%d", __netPong.score_remote);
    fb_writeTextRight(NETPONG_HEIGHT-5-2*8, txtRemoteScore, 0x1F, 0x00, 2);
    
}

void NetPong_writeSeconds() {
    char txtGameTime[6];
    sprintf(txtGameTime, "%02d:%02d", (int)(__netPong.seconds/60), (int)(__netPong.seconds%60));
    
    //char txtGameTime[11];
    //sprintf(txtGameTime, "%d", (int)(Bt_Rt_ReadCurCnt(&NETPONG_SECONDS_COUNTER)));
    fb_writeTextCentered(NETPONG_HEIGHT-5-2*8, txtGameTime, 0x1F, 0x00, 2);
}


/********************************************************************
 *                            Input Logic                           *
 ********************************************************************/
en_joystick_pos_t NetPong_getJoystickPosX() {
    uint8_t analog11;
    uint8_t analog12;
    uint8_t analog13;
    uint8_t analog16;
    uint8_t analog19;
    uint8_t analog23;
    uint8_t analog17;
    cppp_getAnalogValues(&analog11, &analog12, &analog13, &analog16, &analog17, &analog19, &analog23);
    
    // JS1X is analog16                             
    // left
    if(analog16 >= 245) return POS_LEFT3;
    if(analog16 < 245 && analog16 >= 240) return POS_LEFT2;
    if(analog16 < 240 && analog16 >= 220) return POS_LEFT1;
    
    // middle
    if(analog16 < 220 && analog16 >= 180) return POS_MIDDLE;
    
    // right
    if(analog16 < 180 && analog16 >= 100) return POS_RIGHT1;
    if(analog16 < 100 && analog16 >= 30) return POS_RIGHT2;
    if(analog16 < 30) return POS_RIGHT3;
    
    return POS_MIDDLE;
}
en_joystick_pos_t NetPong_getJoystickPosY() {
    uint8_t analog11;
    uint8_t analog12;
    uint8_t analog13;
    uint8_t analog16;
    uint8_t analog19;
    uint8_t analog23;
    uint8_t analog17;
    cppp_getAnalogValues(&analog11, &analog12, &analog13, &analog16, &analog17, &analog19, &analog23);
    
    // JS1Y is analog19 
    if(analog19 >= 220)  return POS_BOTTOM;
    if(analog19 < 220 && analog19 >= 180) return POS_MIDDLE;
    if(analog19 < 180) return POS_TOP;
    
    return POS_MIDDLE;
}

uint8_t NetPong_isLeftJoystickPressed() {
  // Inverted logic as input is pulled up
  return Gpio1pin_Get(GPIO1PIN_PF5) == 0;                             
}
