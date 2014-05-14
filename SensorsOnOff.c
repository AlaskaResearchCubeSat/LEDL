#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "uart.h"




//P7.7 Sensors on/off    //P8.6 ACC on/off    //P8.7 MAG on/off
 void SENSORSon(void){
 int checksensors; 
  P7DIR &= ~BIT7;//TURNS ONLY P7.7 OFF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  P7OUT &= ~BIT7;//TURNS ONLY P7.7 0FF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
 // P4DIR |= BIT2;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  //P4OUT |= BIT2;//LED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  /*
   printf("When finished taking voltage measurment for sensor on press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = Getc();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
 }

 void SENSORSoff(void){
 int checksensors; 
  P7DIR |= BIT7;//TURNS ONLY P7.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P7OUT |= BIT7;//TURNS ONLY P7.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P4DIR &= ~BIT2;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  P4OUT &= ~BIT2;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  /*
  printf("When finished taking voltage measurment for sensor off press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = Getc();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }

 void ACCon(void){
 int checksensors; 
  P8SEL &= ~(BIT6|BIT7);
  P8DIR &= ~BIT6;//TURNS ONLY P8.6 OFF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  P8OUT &= ~BIT6;//TURNS ONLY P8.6 0FF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
 // P4DIR |= BIT3;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
 // P4OUT |= BIT3;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  /*
  printf("When finished taking voltage measurment for acc on press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = Getc();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }

 void ACCoff(void){
 int checksensors; 
  P8SEL &= ~(BIT6|BIT7);
  P8DIR |= BIT6;//TURNS ONLY P8.6 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P8OUT |= BIT6;//TURNS ONLY P8.6 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P4DIR &= ~BIT3;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  P4OUT &= ~BIT3;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  /*printf("When finished taking voltage measurment for acc off press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = Getc();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }
 void MAGon(void){
 int checksensors; 
  P8SEL &= ~(BIT6|BIT7);
  P8DIR &= ~BIT7;//TURNS ONLY P8.7 OFF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  P8OUT &= ~BIT7;//TURNS ONLY P8.7 0FF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
  P4DIR |= BIT4;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  P4OUT |= BIT4;//lED TURNS ON TO INDICATE SENSORS ARE ACTIVE
  /*
  printf("When finished taking voltage measurment for mag on press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = Getc();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }

 void MAGoff(void){
 int checksensors; 
  P8SEL &= ~(BIT6|BIT7);
  P8DIR |= BIT7;//TURNS ONLY P8.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P8OUT |= BIT7;//TURNS ONLY P8.7 On (signal is applied to a PMOS gate, being high makes it turn off, cutting voltage)
  P4DIR &= ~BIT4;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  P4OUT &= ~BIT4;//lED TURNS OFF TO INDICATE SENSORS ARE OFF
  /*
  printf("When finished taking voltage measurment for mag off press the space bar to continue.\r\n");
    while(checksensors!=' '){//continues to check to see if the space bar was hit.  
    checksensors = Getc();//We expect the ascii key for return to be in the return value.  
    }
    checksensors=0;
    */
  }
void VREGon(void){
  P7DIR |= BIT3;
  P7OUT |= BIT3;
  }

void VREGoff(void){
  P7DIR |=BIT3;
  P7OUT &= ~BIT3;
  }