#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "uart.h"
#include <ctl.h>
#include "ARCbus.h"
#include "ADC_setup.h"
#include <math.h>
#include "log_data.h"




//set up the event that will cause the MSP to switch 
CTL_EVENT_SET_t handle_LaunchDetect;

void setup_launch_detect(void){
 ctl_events_init(&handle_LaunchDetect, 0);

 P2DIR &= ~BIT7;//TURNS ONLY P2.7 OFF (signal is applied to a PMOS gate, being low makes it active, providing voltage to sensors)
 P2IES &= ~BIT7;//sets up interrupt to be rising edge
 P2IFG &= ~BIT7;//you clear the flag before the interrupt is set
 P2IE |= BIT7;//sets up interrupt 
 
}

void verify_launch_int(void) __interrupt[PORT2_VECTOR]{//this is an interrupt function the _interrupt tells its a function interrupt 
//placed in code that sets the flag
  if (P2IFG&BIT7){
  P2IFG=0;
  ctl_events_set_clear(&handle_LaunchDetect,1<<0,0);
  }
  else if (P2IFG&BIT6){
  P2IFG=0;
  printf("Port 2.6 Flag was set");// this interrupts are not being used for anything. in case one of the interrupts gets set then system will not loop
  }
  else if (P2IFG&BIT5){
  P2IFG=0;
  printf("Port 2.5 Flag was set");// this interrupts are not being used for anything. in case one of the interrupts gets set then system will not loop
  }
  else if (P2IFG&BIT4){
  P2IFG=0;
  printf("Port 2.4 Flag was set");// this interrupts are not being used for anything. in case one of the interrupts gets set then system will not loop
  }
  else if (P2IFG&BIT3){
  P2IFG=0;
  printf("Port 2.3 Flag was set");// this interrupts are not being used for anything. in case one of the interrupts gets set then system will not loop
  }
  else if (P2IFG&BIT2){
  P2IFG=0;
  printf("Port 2.2 Flag was set");// this interrupts are not being used for anything. in case one of the interrupts gets set then system will not loop
  }
  else if (P2IFG&BIT1){
  P2IFG=0;
  printf("Port 2.1 Flag was set");// this interrupts are not being used for anything. in case one of the interrupts gets set then system will not loop
  }
  else  {
  P2IFG=0;
  printf("Port 2.0 Flag was set");// this interrupts are not being used for anything. in case one of the interrupts gets set then system will not loop
  }
 
}
//Code will verify if there is a launch 
void VerifyLaunchDetect(void){

 float checkacc[6];
 unsigned int data;
 int i;
 float acc18gconversion = 0.057; // volts per g
 float acc70gconversion = 0.0242; // volts per g
 float adcconversion = 0.0008057; // volts per adc reference 0 to 4096
 unsigned e;
printf("\rLEDL launch test. Press s to stop. \r\n>");// 
adcsetup();
 while(async_CheckKey()!='s')
 {
   unsigned checking_for_launch;
   checking_for_launch=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_LaunchDetect,(1<<0),CTL_TIMEOUT_NONE,0);
   if(checking_for_launch&(0x01))
   {
   //P7OUT = BIT7;
   //P7DIR = BIT7;

   //Code to check for acceleration, just pulled code from log_data file
 
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_adc,(1<<0),CTL_TIMEOUT_NONE,0);
      if(e&(0x01))
      {
        volatile unsigned int *arr = &ADC12MEM0;// creates a pointer, looks at the memory of mem0, checks adc register 
           { 
              for (i=0; i<6; ++i)// look at first 6 adc measurements of acceleration  
               {
                checkacc[i]=arr[i];
                //data is represented as a number from 0 to 4096 ( 0 V to 3.3 V), 0.0008057; volts per adc reference 0 to 4096
                //a voltage conversion for the accelerometers
                //   ADXL321 18 g accelerometer   57.0 mV per g 
                //   ADXL001 70 g accelerometer   24.2 mV per g

                checkacc[i]*=adcconversion;
                checkacc[i]-=1.65;
                checkacc[i]=fabsf(checkacc[i]);
               
                  if (i==0) // AXDL321 are located in MEM0-MEM2 
                  {
                  if ( checkacc[0] > 0.057){
                 // P7OUT |= BIT0;
                 // P7DIR |= BIT0;
                  }}
                  if (i==1) //ADXL001 are located at MEM3-MEM4
                  {
                  if(checkacc[1] > 0.057){
                 // P7OUT |= BIT1;
                 // P7DIR |= BIT1;
                  }}
                  if (i==2) //ADXL001 are located at MEM3-MEM4
                  {
                  if (checkacc[2] > 0.057){
                  //P7OUT |= BIT2;
                 // P7DIR |= BIT2;
                  }}
                  if (i==3) //ADXL001 are located at MEM3-MEM4
                  {
                  if (checkacc[3]  > 0.0242){
                 /// P7OUT |= BIT3;
                  //P7DIR |= BIT3;
                  }}
                  if (i==4) //ADXL001 are located at MEM3-MEM4
                  {
                  if(checkacc[4]  > 0.0242){
                 // P7OUT |= BIT4;
                 // P7DIR |= BIT4;
                  }}
                  if (i==5) //ADXL001 are located at MEM3-MEM4
                  {
                  if(checkacc[5] > 0.0242){ 
                 // P7OUT |= BIT5;
                 // P7DIR |= BIT5;
                  }}
                  else
                  {
                 // P7OUT |= BIT6;
                  //P7DIR |= BIT6;
                  }
                  
                }
               }  




            }
      }
   }
 }


/*launch detec is based on the piezo sensor, when the output of the piezo is sufficient there is a voltage that will be seen on pin 2.7 of the microprocessor
the idea is that until the vibration LEDL will be in  a low power mode. when the vibration happens it will act like a function and be automatically go into
determining if there is launch. To determine it will have to analyze the accelerometers and determine if there is any acceleration which could not be zero.
 ledl will wake from lPM check the acceleration, if not in low power mode then it will go back to sleep perhaps wait some time (30 seconds to 5 min) to check 
 for vibrations again. that way in the case its being handeld it will not wake from lowpower mode for some x amount of time to reduce the amount of time LEDL
 checks for launch. To do this, it should be like when i had the log data as an automatic function that is dependant on 2.7 being high. This could be done simply with events. 
 need to check out events to set up this as an automatic function. */






