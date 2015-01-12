#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "ctl.h"
#include "sensor-interface.h"


void timersetup(void){
//setting up timer (create more comments that will reference pictures and give more details to exactly what is happening.)
TACTL = TASSEL_1 | ID_3; // using ACLK for timer, dividing timer by 8 timer is ~4KHz , 
//since the timer is opperating at ~4KHz, the timer can count to 4000 to make the frequncy of ~1Hz
TACCR0 = 4000;
TACCR1 = 3750; 
TACCTL1 = OUTMOD_3 ; //

//P2SEL |= BIT3;//this bit is the visual output pin that shows the high out pin from the timer
//P2DIR |= BIT3;//

TACTL |= MC_1; // in up mode counts to TACCR0,
}


void timersetup4kHz(void){
//setting up timer (create more comments that will reference pictures and give more details to exactly what is happening.)
TACTL = TASSEL_1 | ID_0; // using ACLK for timer, dividing timer by 1 timer is ~32KHz , 
//since the timer is opperating at ~32KHz, the timer can count to  to make the frequncy of ~4000Hz
TACCR0 = 8;
TACCR1 = 6; 
TACCTL1 = OUTMOD_3 ; //

P2SEL |= BIT3;//this bit is the visual output pin that shows the high out pin from the timer
P2DIR |= BIT3;//

TACTL |= MC_1; // in up mode counts to TACCR0,
}



// removing this due to ARC_setup will take care of all the timer setup 
//setup timer A to run off 32.768kHz xtal
void init_timerA(void){
  //setup timer A 
  //TACTL=TASSEL_1|ID_0|TACLR;//AClK|divide the clock by 1 ~32 kHz// done by arc bus
  //init CCR0 for tick interrupt
  //TACCR0=32;
 // TACCTL0=CCIE; 
  TACCR1=1000;
  TACCTL1=CCIE|OUTMOD_4;
}

//start timer A in continuous mode
void start_timerA(void){
//start timer A
  TACTL|=MC_2;
}



//================[Time Tick interrupt]=========================
/*void task_tick(void) __ctl_interrupt[TIMERA0_VECTOR]{
  //set rate to 1024Hz
  TACCR0+=32;
  //increment timer
  ctl_increment_tick_from_isr();
}
*/

void task_tick_forADC(void) __ctl_interrupt[TIMERA1_VECTOR]{
  extern MAG_TIME mag_time;
  extern short int_count;
  extern CTL_EVENT_SET_t sens_ev;
  switch(TAIV){
    case TAIV_TACCR1:
        //set rate to 8000Hz to change this later on, change the TACCR1+=4 for final value to take accelerometer data at 4kHz 
        TACCR1+=4;
        //increment timer
    break;
    case TAIV_TACCR2:
        //setup next interrupt
        TACCR2+=mag_time.T;
        //decrement count
        int_count--;
        if(int_count<=0){
          ctl_events_set_clear(&sens_ev,SENS_EV_READ,0);
          int_count=mag_time.n;
        }
     break;
}

}
