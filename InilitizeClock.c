#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "uart.h"

void initCLK(void){
  //set XT1 load caps, do this first so XT1 starts up sooner//sets up Aclk
  BCSCTL3=XCAP_0;
  //stop watchdog
  WDTCTL = WDTPW|WDTHOLD;
  //setup clocks

  //set DCO to 16MHz from calibrated values both smclock and mclk are functing off DCO
  DCOCTL=0;
  BCSCTL1=CALBC1_16MHZ;
  DCOCTL=CALDCO_16MHZ;
 
}
