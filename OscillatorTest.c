#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "uart.h"

void OscillatorTest(void){
int OscTstFin; //OscTstFin is a variable that will show up when the test is done so the test can move forward 
printf("The Oscillator Test\r\nTo test the Oscillator probe the crystal and measure with an osilliscope to determine if it is operation at 32 kHz\r\n");
printf("Place the results in Document ARC1-LEDL-TST-LEDLPROTO-R01 section 3.4 Oscillator Test.\r\n");
printf("When finished press the space bar to continue.\r\n");
while(OscTstFin!=' '){//continues to check to see if the space bar was hit.  
OscTstFin = Getc();//We expect the ascii key for return to be in the return value.  
}
}

