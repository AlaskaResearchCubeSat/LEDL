#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include <ctl.h>
#include "SensorsOnOff.h"




//P7.7 Sensors on/off    //P8.6 ACC on/off    //P8.7 MAG on/off
 void SENSORSon(void){
 int checksensors; 
  P7DIR |= BIT7;//TURNS ONLY P7.7 OFF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  P7OUT &= ~BIT7;//TURNS ONLY P7.7 0FF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
//  P4DIR |= BIT2;//lED are an output
 // P4OUT |= BIT2;//LED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  /*
   printf("When finished taking voltage measurment for sensor on press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = getchar();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
 }

 void SENSORSoff(void){
 int checksensors; 
  P7DIR |= BIT7;//TURNS ONLY P7.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P7OUT |= BIT7;//TURNS ONLY P7.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
 // P4DIR |= BIT2;//lED are and output 
 // P4OUT &= ~BIT2;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  /*
  printf("When finished taking voltage measurment for sensor off press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = getchar();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }

 void ACCon(void){
 int checksensors; 
  //P8SEL &= ~(BIT6|BIT7);
  P8DIR |= BIT6;//TURNS ONLY P8.6 OFF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  P8OUT &= ~BIT6;//TURNS ONLY P8.6 0FF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
//  P4DIR |= BIT3;//lED is an output 
 // P4OUT |= BIT3;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  /*
  printf("When finished taking voltage measurment for acc on press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = getchar();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }

 void ACCoff(void){
 int checksensors; 
  P8DIR |= BIT6;//TURNS ONLY P8.6 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P8OUT |= BIT6;//TURNS ONLY P8.6 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
 // P4DIR |= BIT3;//lED is an output 
//  P4OUT &= ~BIT3;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  /*printf("When finished taking voltage measurment for acc off press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = getchar();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }
 void MAGon(void){
 int checksensors; 
  P8SEL &= ~(BIT6|BIT7);
  P8DIR &= ~BIT7;//TURNS ONLY P8.7 OFF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  P8OUT &= ~BIT7;//TURNS ONLY P8.7 0FF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  //P4DIR |= BIT4;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
 // P4OUT |= BIT4;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  /*
  printf("When finished taking voltage measurment for mag on press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = getchar();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }

 void MAGoff(void){
 int checksensors; 
  P8SEL &= ~(BIT6|BIT7);
  P8DIR |= BIT7;//TURNS ONLY P8.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P8OUT |= BIT7;//TURNS ONLY P8.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  //P4DIR &= ~BIT4;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  //P4OUT &= ~BIT4;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  /*
  printf("When finished taking voltage measurment for mag off press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = getchar();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }
void VREGon(void){
  P7OUT |= BIT3;
  }

void VREGoff(void){
  P7OUT &= ~BIT3;
   }

void VREGinit(void){
 P7DIR |=BIT3;
 P7OUT &= ~BIT3;
 }

 void GyroMuxon(void){
 P8OUT |= BIT3;
 }

 void GyroMuxoff(void){
 P8OUT &= ~BIT3;
 }

 void GyroSelfTeston(void){
 P8OUT |=BIT0;
 }

 void GyroSelfTestoff(void){
 P8OUT &= ~BIT0;
 }

 void GyroSleep(void){
 P8OUT |= BIT1;
 }

 void GyroWakeUp(void){
 P8OUT &= ~BIT1; 
 }

 void Gyroinit(void){//INITALIZE ALL GYRO CONTROLS, SELFTEST, SLEEP, AND MUX TO SWITCH BETWEEN AXES. 
 P8DIR |= BIT0|BIT1|BIT3;
 P8OUT |= BIT1;//START GYRO IN SLEEP MODE TO REDUCE POWER. 
 }
 

void UnusedPinSetup(void){
P2DIR |= BIT0|BIT1|BIT2|BIT3|BIT4|BIT5;
P2OUT &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);
P4DIR |= BIT0|BIT1|BIT6|BIT7;
P4OUT &= ~(BIT0|BIT1|BIT6|BIT7);
P5DIR |= BIT3|BIT7;
P5OUT &= ~(BIT3|BIT7);
P7DIR |= BIT0|BIT1|BIT6;
P7OUT &= ~(BIT0|BIT1|BIT6);

}

void SD_LED(void){
P4OUT ^= BIT1;
}

void RESET_LED(void){
P4OUT ^= BIT0;
}

void SD_LED_OFF(void){
P4OUT &=~BIT1;
}

void LEDL_SWITCH_TO_EPS(void){
P7DIR|=BIT2;
P7OUT&=~BIT2;
}

void LEDL_BLOW_FUSE(void){
P7OUT|=BIT5;
ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 
P7OUT&=~BIT5;
}
