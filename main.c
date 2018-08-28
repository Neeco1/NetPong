/*
* Main file of the NetPong game for the C/C++ lab Microcontroller 
* of the real-time systems lab at TU Darmstadt.
*
* Running on 200MHz FM4 (S6E2C)
* 
* Required Hardware:
* - Scheduler: BaseTimer 1
* - IR Phy: BaseTimer 2
* - NetPong: BaseTimer 3
*
* Author: Nicolas Himmelmann
*/                                                              

#include "s6e2ccxj.h" 
#include "sched/sched.h"
#include "cppp_board/cppp_init.h"  
#include "cppp_board/cppp_lcd.h"     
#include "pdl_header.h" 
#include "cppp_board/cppp_gfx.h"       
#include "phy_ir/phy_ir.h" 
#include "bt/bt.h" 
#include "netpong/NetPong.h" 
  
#define RX_BUFFER_SIZE 300
//Data buffer for received data  
uint8_t __dataBuffer[RX_BUFFER_SIZE]; 

/** Function predefinitions **/
void init_sched_timer(void);
static uint32_t get_current_time(void * hint);
static void pong_task(void * myCtx);
void rx_notify(uint16_t received_bytes, phy_rx_result_t success, uint16_t info);
int main(void);
  

//Struct for Base Timer config in Reload mode
stc_bt_rt_config_t stcTimerCfg = { 
    .enPres = RtPres1Div256 , //Timer clock prescaler of 256 at 200MHz: Results in resolution of 1.28us
                              //(overflow after 0.0838seconds = 83.8ms)
    .enSize = RtSize16Bit, //Timer size of 16bit
    .enMode = RtReload, //Configure timer as reload
    .enExtTrig = RtExtTiggerDisable //Disable external trigger
};


//Initialize the 16 bit reload basetimer for the scheduler.
void init_sched_timer(void) {
    // Set I/O mode
    Bt_ConfigIOMode(&BT1, BtIoMode0);
    // Initialize Reload function of BT
    Bt_Rt_Init(&BT1, &stcTimerCfg);
    // Write start value (maximum of 16bits = 65536)
    Bt_Rt_WriteCycleVal(&BT1, 65535);
    //Enable count operation
    Bt_Rt_EnableCount(&BT1);
    //Start timer by software
    Bt_Rt_EnableSwTrig(&BT1);
} 

static uint32_t
get_current_time(void * hint)
{
    //Timer resolution is 1.28us. 
    //Timer counts down from maximum value, so raw value needs to be flipped 
    //to be compatible with the scheduler.
    return 65535 - Bt_Rt_ReadCurCnt(&BT1);
}


struct pong_task_ctx {
    uint16_t tick_counter;
    uint16_t sec_counter;
};
static void 
pong_task(void * myCtx)
{
    //struct pong_task_ctx * ctx = (struct pong_task_ctx *) myCtx;
    NetPong_main();
}

//Notification callback is called from PHY implementation.
void rx_notify(uint16_t received_bytes, phy_rx_result_t success, uint16_t info) {
    if(success == RX_SUCCESS)             
    {  
			  //Tell netpong that we received new data
        NetPong_newDataFromRemote(__dataBuffer, received_bytes);
    }
}
  
int main(void) { 
    cppp_initBoard();
    init_sched_timer();
    
    //782 results in roughly 1ms tick time
    struct sched_ctx * scheduler = sched_alloc_context(NULL, get_current_time, 65535, 782);
        
    // Register Pong task in scheduler
    struct pong_task_ctx pong_ctx;                                            
    //struct sched_task * pong_task_handle = 
    sched_alloc_task(scheduler, &pong_ctx, pong_task, "NetPong game task", TASK_TICK_1);
                     
    gfx_fillScreen(BLACK);       
    lcd_setTextSize(2);        
    lcd_setTextColor(YELLOW); 

    if( phy_init(__dataBuffer, RX_BUFFER_SIZE, rx_notify) == PHY_ERROR )
    {
        lcd_writeText("Error initializing PHY.");
        while(1);
    }
    if( NetPong_init(phy_send) != 1)
    {
        lcd_writeText("Error initializing NetPong game.");
        while(1);
    }
    //NetPong initialized
        
    while(1) {      
        sched_run(scheduler);
    }
}
