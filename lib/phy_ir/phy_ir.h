/******************************************************************************
 * Infrared PHY header file                                                   *
 *                                                                            *
 * Needs the following hardware:                                              *
 * - Base timer 2 (used as downcounter)                                       *
 * - Interrupt on the RX pin                                                  *
 *                                                                            *
 * Author: Nicolas Himmelmann                                                 *
 ******************************************************************************/

#ifndef PHY_IR_H
#define PHY_IR_H 
 
#include "bt/bt.h" //Base timer, not Bluetooth!

//Using pin 83 for interrupt:
//Port 7 Pin C (INT04_1)
//Need to set PDL_PERIPHERAL_ENABLE_EXINT4 to PDL_ON in pdl_user.h
#define PHY_IRQ_CHANNEL 4u                    

#define PHY_COUNTER BT2

//Important: Changing it here is not sufficient, you also need to change
//the pin in the PHY_IR.c code, due to the macros of the Cypress PDL 
#define PHY_RX_PIN GPIO1PIN_P7C
#define PHY_TX_PIN GPIO1PIN_P64

#define PIN_HIGH    1
#define PIN_LOW     0

//Values are calculated as follows: (desired time in us) / (Timer resolution of 1.28us)
#define TIMER_MAX_VALUE     65535
#define TIMER_10MS          7813
#define TIMER_9MS           7032
#define TIMER_8MS           6250
#define TIMER_5_5MS         4297
#define TIMER_4_5MS         3516
#define TIMER_3_5MS         2735
#define TIMER_2_25MS        1758
#define TIMER_1_75MS        1368
#define TIMER_1_25MS        977
#define TIMER_1MS           782
#define TIMER_0_75MS        586
#define TIMER_0_5MS         391
#define TIMER_0_25MS        196

#define PHY_SUCCESS 1
#define PHY_ERROR   0
#define PHY_ERROR_1   5
#define PHY_ERROR_2   6
#define PHY_ERROR_3   7
#define PHY_ERROR_4   8

//PWM properties
#define PHY_PWM_FREQ               38000 //Hz
#define PHY_PWM_PERIOD_US          26    //1000000/pwm_freq
#define PHY_PWM_HALF_PERIOD_TICKS  11    //(pwm_period_us/2) / 1.28;

/**
 * Defines the possible states of the infrared PHY implementation.
 *
 * PHY_RX_IDLE_WAIT_PREAMBLE: We are waiting for an incoming signal.
 * PHY_RX_PREAMBLE_IN_PROGRESS: Signal has started, currently 
 *                              receiving the 9ms preamble.
 * PHY_RX_AFTER_PREAMBLE: 4.5ms space after the preamble.
 * PHY_RX_WAIT_BIT_PULSE: Currently in the 500us pulse of a bit.
 * PHY_RX_WAIT_BIT_SPACE: Currently in the 500us or 1ms space of a bit 
 *                       (following a pulse).
 */
typedef enum { 
    PHY_RX_IDLE_WAIT_PREAMBLE, 
    PHY_RX_PREAMBLE_IN_PROGRESS, 
    PHY_RX_AFTER_PREAMBLE, 
    PHY_RX_WAIT_BIT_PULSE, 
    PHY_RX_WAIT_BIT_SPACE 
} phy_rx_state_t;

typedef enum {
    RX_SUCCESS,
    RX_ERROR,
    RX_ERR_PARTIAL,
    RX_ERR_NONE,
    RX_ERR_OVERFLOW,
    RX_ERR_SHORT
} phy_rx_result_t;

void EXINT4_IRQHandler(void);
void phy_ir_init_interrupt(void);

uint16_t phy_ir_bits_to_bytes(uint16_t bits);

/**
* Initialize the 16 bit reload basetimer 2 for the physical IR layer.
*/
void phy_ir_init_timer(void);
void phy_ir_start_timer(void);

/**
 * This function initializes the infrared PHY layer.
 * Needs pointer to the rx data buffer and rx buffer size as parameter. 
 * Also, a callback must be passed which notifies the app about new data.
 *
 * @return PHY_SUCCESS if all ok. PHY_ERROR in case of error.
 */
uint8_t phy_init(uint8_t * data_buf, const uint16_t buf_len, void(*funcPtrNotify)(uint16_t, phy_rx_result_t, uint16_t));

/**
 * Function to check if a value is between two others.
 * 
 * @param value: The value to check
 * @param lower: The minimum lower bound the period is allowed to last
 * @param upper: The upper bound.
 *
 * @return 1 if the value is between lower and upper, 0 if not.
 */
uint8_t phy_ir_period_between(uint16_t value, uint16_t lower, uint16_t upper);

/**
 * Resets the RX state machine to the initial values.
 */
void phy_ir_rx_reset_state(void);

/**
 * Resets the TX variables to the initial values.
 */
void phy_ir_tx_reset_state(void);

void phy_ir_delay_ticks(uint16_t length_us);
void phy_ir_send_pulse(uint16_t length_us);
void phy_ir_send_space(uint16_t length_us);
void phy_ir_send_byte(uint8_t data);
void phy_send(uint8_t * dataBuf, uint16_t bufLen);



#endif
