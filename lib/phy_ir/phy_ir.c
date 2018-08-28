/******************************************************************************
 * Infrared PHY implementation                                                *
 * (for documentation, see PHY_IR.h)                                          *
 ******************************************************************************/
#include "phy_ir.h"        
#include "s6e2ccxj.h"        
#include "pdl_header.h"
#include "exint/exint.h" 
#include <string.h>

//Initial value of the RX state is IDLE.
phy_rx_state_t __rx_state            = PHY_RX_IDLE_WAIT_PREAMBLE;
uint8_t        __rx_last_pin_value   = PIN_LOW;
uint16_t       __tim_last_ticks      = 0;
uint16_t       __rx_bit_count        = 0;

//These values are passed by the higher layer (user) which uses the PHY.
uint8_t * __rx_data_buffer;
uint16_t  __rx_buffer_size_bytes;
void (* __rx_func_ptr_notify)(uint16_t received_bytes, phy_rx_result_t success, uint16_t info);

/******************************************************************************
 * Infrared PHY configuration structures                                      *
 ******************************************************************************/

//Struct for initial configuration of the interrupt
stc_exint_config_t stcExIntConfig; 
//Struct for Base Timer config in Reload mode
stc_bt_rt_config_t stcPhyTimerCfg;


/******************************************************************************
 * Interrupt-related functions                                                *
 ******************************************************************************/
void phy_ir_exint4_irq_handler(void) { //EXINT4_IRQHandler
    Exint_DisableChannel(PHY_IRQ_CHANNEL);
    
    uint8_t curPinState = Gpio1pin_Get(GPIO1PIN_P7C);
    uint16_t curTimVal = Bt_Rt_ReadCurCnt(&PHY_COUNTER);
    uint16_t timDiff = __tim_last_ticks - curTimVal;
    
    switch(__rx_state) {
        case PHY_RX_IDLE_WAIT_PREAMBLE:
            if( (__rx_last_pin_value == PIN_HIGH) && (curPinState == PIN_LOW) )
            {
                //Preamble has started. Pin must stay low for 9ms.
                //Start the timer now.
                phy_ir_start_timer();
                __tim_last_ticks = TIMER_MAX_VALUE;
                timDiff = 0;
                
                //Next edge of rx pin should be rising
                Exint_SetDetectMode(PHY_IRQ_CHANNEL, ExIntRisingEdge);
                
                //Change state
                __rx_state = PHY_RX_PREAMBLE_IN_PROGRESS;
                __rx_last_pin_value = curPinState;
            }
            else 
            {
                __rx_func_ptr_notify(phy_ir_bits_to_bytes(__rx_bit_count), RX_ERROR, 0);
                phy_ir_rx_reset_state();
            }
        break;
        case PHY_RX_PREAMBLE_IN_PROGRESS:
            if( (__rx_last_pin_value == PIN_LOW) && (curPinState == PIN_HIGH) )
            {
                //Preamble has stopped. Check if time is okay (8ms to 10ms).
                if( phy_ir_period_between(timDiff, TIMER_8MS, TIMER_10MS) )
                {
                    //Time is correct.
                    //Change state.
                    __rx_state = PHY_RX_AFTER_PREAMBLE;
                    __rx_last_pin_value = curPinState;
                    __tim_last_ticks = curTimVal;
                    
                    Exint_SetDetectMode(PHY_IRQ_CHANNEL, ExIntFallingEdge);
                }
                else 
                {
                    __rx_func_ptr_notify(phy_ir_bits_to_bytes(__rx_bit_count), RX_ERROR, 0);
                    //Preamble duration was not correct.
                    //Reset to initial state.
                    phy_ir_rx_reset_state();
                }
            }
            else 
            {
                phy_ir_rx_reset_state();
            }
        break;
        case PHY_RX_AFTER_PREAMBLE:
            if( (__rx_last_pin_value == PIN_HIGH) && (curPinState == PIN_LOW) )
            {
                //Space after preamble has finished. Check if time is okay (3.5ms to 5.5ms).
                if( phy_ir_period_between(timDiff, TIMER_3_5MS, TIMER_5_5MS) )
                {                
                    __rx_state = PHY_RX_WAIT_BIT_PULSE;
                    __rx_last_pin_value = curPinState;
                    __tim_last_ticks = curTimVal;
                    
                    Exint_SetDetectMode(PHY_IRQ_CHANNEL, ExIntRisingEdge);
                }
                else 
                {
                    __rx_func_ptr_notify(0, RX_ERR_SHORT, 0);
                    phy_ir_rx_reset_state();
                }
            }
            else 
            {
                phy_ir_rx_reset_state();
            }
        break;
        case PHY_RX_WAIT_BIT_PULSE:
            if( (__rx_last_pin_value == PIN_LOW) && (curPinState == PIN_HIGH) )
            {
                //Pulse of bit has finished. Check if time is okay (250us to 750us).
                if( phy_ir_period_between(timDiff, TIMER_0_25MS, TIMER_0_75MS) )
                {
                    __rx_state = PHY_RX_WAIT_BIT_SPACE;
                    __rx_last_pin_value = curPinState;
                    __tim_last_ticks = curTimVal; 
                    
                    Exint_SetDetectMode(PHY_IRQ_CHANNEL, ExIntFallingEdge);
                }
                else 
                {
                    __rx_func_ptr_notify(phy_ir_bits_to_bytes(__rx_bit_count), RX_ERROR, timDiff);
                    if(__rx_bit_count > 0) 
                    {
                        //TODO: What to do if only some data was successfully received?
                    }
                    phy_ir_rx_reset_state();
                }
            }
        break;
        case PHY_RX_WAIT_BIT_SPACE:
            if( (__rx_last_pin_value == PIN_HIGH) && (curPinState == PIN_LOW) )
            {
                //Space of bit has finished. 
                //Check if we have a 0 or a 1
                if( phy_ir_period_between(timDiff, TIMER_0_75MS, TIMER_1_25MS) )
                {
                    //Have a 1. Put it at the correct position in the byte data buffer.
                    __rx_data_buffer[__rx_bit_count / 8] |= (1u << (__rx_bit_count % 8));
                }
                else if( phy_ir_period_between(timDiff, TIMER_0_25MS, TIMER_0_75MS) ) 
                {
                    //Have a 0. Clear the corresponding bit.
                    __rx_data_buffer[__rx_bit_count / 8] &= ~(1u << (__rx_bit_count % 8));
                }
                else if( phy_ir_period_between(timDiff, TIMER_3_5MS, TIMER_5_5MS) ) 
                {
                    //End of transmission received.
                    __rx_func_ptr_notify(phy_ir_bits_to_bytes(__rx_bit_count), RX_SUCCESS, 0);
                    phy_ir_rx_reset_state();
                    break;
                }
                else 
                {
                    //No time of those above was valid.
                    //What did we actually receive then ?? Definitely some error. 
                    if(__rx_bit_count > 0) 
                    {
                        //Only some data has successfully been received
                        __rx_func_ptr_notify(phy_ir_bits_to_bytes(__rx_bit_count), RX_ERR_PARTIAL, timDiff);
                    }
                    else 
                    {
                        //Nothing has been received
                        __rx_func_ptr_notify(phy_ir_bits_to_bytes(__rx_bit_count), RX_ERR_NONE, 0);
                    }
                    phy_ir_rx_reset_state();
                    break;
                }
                ++__rx_bit_count;
                /*if( ((__rx_bit_count/8) > __rx_buffer_size_bytes) )
                {
                    //TODO: Overflow
                    __rx_func_ptr_notify(phy_ir_bits_to_bytes(__rx_bit_count), RX_ERR_OVERFLOW, 0);
                }*/
                __rx_state = PHY_RX_WAIT_BIT_PULSE;
                __rx_last_pin_value = curPinState;
                __tim_last_ticks = curTimVal;
                
                Exint_SetDetectMode(PHY_IRQ_CHANNEL, ExIntRisingEdge);
            }
        break;
    }
    
    Exint_EnableChannel(PHY_IRQ_CHANNEL);
} 

uint16_t phy_ir_bits_to_bytes(uint16_t bits) {
    return (bits/8) + (((bits % 8) != 0) ? 1 : 0);
}

void phy_ir_init_interrupt() {
    //Initialize RX Pin as input and disable pullup resistor
    //since IR RX module keeps pin high itself.
    Gpio1pin_InitIn(GPIO1PIN_P7C, Gpio1pin_InitPullup(0u));
    SetPinFunc_INT04_1(0u);    
    stcExIntConfig.abEnable[ExintInstanceIndexExint4] = 1u;
    //RX module pulls pin high as long as nothing happens.
    //Thus we first need to wait for a falling edge which indicates the
    //start of a signal.
    stcExIntConfig.aenLevel[ExintInstanceIndexExint4] = ExIntFallingEdge;
    stcExIntConfig.apfnExintCallback[ExintInstanceIndexExint4] = phy_ir_exint4_irq_handler;
    stcExIntConfig.bTouchNvic = 1u;
    Exint_Init(&stcExIntConfig);
}


/******************************************************************************
 * Timer-related functions                                                    *
 ******************************************************************************/
void phy_ir_init_timer() {
    //Disable timer (only for precaution)
    Bt_Rt_DisableCount(&PHY_COUNTER);
    //Set timer I/O mode (simple counter)
    Bt_ConfigIOMode(&PHY_COUNTER, BtIoMode0);
    //Initialize Reload function of BT
    stcPhyTimerCfg.enPres = RtPres1Div256; //Timer clock prescaler of 256 at 
                                           //200MHz: Results in resolution of 1.28us 
                                           //(overflow after 0.0838seconds = 83.8ms)
    stcPhyTimerCfg.enSize = RtSize16Bit; //Timer size of 16bit
    stcPhyTimerCfg.enMode = RtReload; //Configure timer as reload
    stcPhyTimerCfg.enExtTrig = RtExtTiggerDisable; //Disable external trigger
    Bt_Rt_Init(&PHY_COUNTER, &stcPhyTimerCfg);
}    

void phy_ir_start_timer() {
    //Disable timer (only for precaution)
    Bt_Rt_DisableCount(&PHY_COUNTER);
    //Write start value (maximum of 16bits = 65536)
    Bt_Rt_WriteCycleVal(&PHY_COUNTER, TIMER_MAX_VALUE);
    //Enable count operation
    Bt_Rt_EnableCount(&PHY_COUNTER);
    //Start timer by software
    Bt_Rt_EnableSwTrig(&PHY_COUNTER);
}  


/******************************************************************************
 * PHY-related functions                                                      *
 ******************************************************************************/
uint8_t phy_init(uint8_t * data_buf, uint16_t buf_len_bytes, void(*funcPtrNotify)(uint16_t, phy_rx_result_t, uint16_t)) {
    if( (data_buf == NULL) || (buf_len_bytes == 0) || (funcPtrNotify == NULL) ) 
    { 
        return PHY_ERROR; 
    }
    __rx_data_buffer = data_buf;
    __rx_buffer_size_bytes = buf_len_bytes;
    __rx_func_ptr_notify = funcPtrNotify;
    memset(__rx_data_buffer, 0, buf_len_bytes);
        
    //Initialize base-timer 2
    phy_ir_init_timer();
    
    //Configure and enable the interrupt on the RX pin
    phy_ir_init_interrupt();
    //Get initial pin value
    __rx_last_pin_value = Gpio1pin_Get(GPIO1PIN_P7C);
    
    //Initialize TX Pin as output and disable pullup resistor
    Gpio1pin_InitOut(GPIO1PIN_P64, Gpio1pin_InitPullup(0u));
    
    return PHY_SUCCESS;
}

uint8_t phy_ir_period_between(uint16_t value, uint16_t lower, uint16_t upper) {
    if( (value >= lower) && (value <= upper) )
    {
        return 1;
    }
    return 0;
}

void phy_ir_rx_reset_state() {
    Bt_Rt_DisableCount(&PHY_COUNTER);
    //Wait for pin to get HIGH
    while(Gpio1pin_Get(GPIO1PIN_P7C) != PIN_HIGH);
    __rx_state = PHY_RX_IDLE_WAIT_PREAMBLE;
    __rx_last_pin_value = PIN_HIGH;
    __tim_last_ticks = 0;
    __rx_bit_count = 0;
    phy_ir_init_timer();
    Exint_SetDetectMode(PHY_IRQ_CHANNEL, ExIntFallingEdge);
}

void phy_ir_tx_reset_state() {
    Bt_Rt_DisableCount(&PHY_COUNTER);
    __tim_last_ticks = 0;
    phy_ir_init_timer();
}

void phy_ir_delay_ticks(uint16_t length_ticks) {
    __tim_last_ticks = Bt_Rt_ReadCurCnt(&PHY_COUNTER);
    volatile uint16_t timerTicks = __tim_last_ticks;
    while( (__tim_last_ticks - timerTicks) < length_ticks)
    {
        timerTicks = Bt_Rt_ReadCurCnt(&PHY_COUNTER);
    }
}
void phy_ir_send_pulse(uint16_t length_ticks) {
    if (length_ticks == 0) return;
    
    uint16_t timerTicksBegin = TIMER_MAX_VALUE;
    volatile uint16_t ticksNow = TIMER_MAX_VALUE;
    phy_ir_start_timer();
    
    while ((timerTicksBegin - ticksNow) < length_ticks) 
    {
        //Oscillate with 50/50 duty cycle at 38kHz
        Gpio1pin_Put(GPIO1PIN_P64 , 1u);
        phy_ir_delay_ticks(PHY_PWM_HALF_PERIOD_TICKS);
        Gpio1pin_Put(GPIO1PIN_P64 , 0u);
        phy_ir_delay_ticks(PHY_PWM_HALF_PERIOD_TICKS);
        
        ticksNow = Bt_Rt_ReadCurCnt(&PHY_COUNTER);
    }
}

void phy_ir_send_space(uint16_t length_ticks) {
    if (length_ticks == 0) return;
    
    Gpio1pin_Put(GPIO1PIN_P64, 0u);
    phy_ir_delay_ticks(length_ticks);
}

void phy_ir_send_byte(uint8_t data) {
  for (int i = 0; i < 8; ++i) 
  {
    //Always send a pulse
    phy_ir_send_pulse(TIMER_0_5MS);
    
    //for a 1-bit: pulse followed by a 1ms space
    if( (data >> i) & 1 ) phy_ir_send_space(TIMER_1MS);
    //for a 0-bit: pulse followed by 500us space
    else phy_ir_send_space(TIMER_0_5MS); 
  }
}

void phy_send(uint8_t * dataBuf, uint16_t bufLen) {
    //Disable interrupts to not receive our own signal
    Exint_DisableChannel(PHY_IRQ_CHANNEL);
    
    phy_ir_init_timer();
    
    phy_ir_send_pulse(TIMER_9MS); //Preamble
    phy_ir_send_space(TIMER_4_5MS); //Space
    
    //Send bytes
    for (uint16_t i = 0; (i < bufLen) ; ++i)
    {
      phy_ir_send_byte(dataBuf[i]);  
    }
  
    //Send end sequence
    phy_ir_send_pulse(TIMER_0_5MS); 
    phy_ir_send_space(TIMER_4_5MS);
    phy_ir_send_pulse(TIMER_0_25MS);
    
    phy_ir_tx_reset_state();
    
    //Re-enable interrupts
    Exint_EnableChannel(PHY_IRQ_CHANNEL);
}


