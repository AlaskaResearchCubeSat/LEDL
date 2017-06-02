#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include <ctl.h>
#include <ARCbus.h>
#include "ADC_setup.h"
#include <math.h>
#include "log_data.h"
#include "LaunchDetect.h"





//set up the event that will cause the MSP to switch 
CTL_EVENT_SET_t handle_LaunchDetect;
CTL_EVENT_SET_t handle_LaunchData;
CTL_EVENT_SET_t handle_OrbitData;

int switch_is_on=0;
int orbit_switch_is_on=0;
unsigned int vib_tab_count=0;

int testing_start=0;

void setup_launch_detect(void){
  //ctl_events_init(&handle_LaunchDetect, 0);//this is when we use the launchDetect Algorithim 
  ctl_events_init(&handle_LaunchData, 0);//this is for use with EMI testing, for actual satellite mission this will be set after the verification of there being a positive launch 
 //this is here for testing until i can figure out what is going on with Port 1 interrupt
 //ctl_events_init(&handle_OrbitData, 0);//this is for use when satellite makes it to orbit-
 P1DIR &= ~(BIT7|BIT6);//TURNS ONLY P2.7 and P2.6 as an input
 P1IES |=  BIT7;//sets up interrupt to be a high to low transition
 
 P1IFG &= ~(BIT7|BIT6);//you clear the flag before the interrupt is set
 P1IE  |= BIT7|BIT6;//sets up interrupt 
 
}
/*
void setup_orbit_start(void){
  //ctl_events_init(&handle_LaunchDetect, 0);//this is when we use the launchDetect Algorithim 
  ctl_events_init(&handle_OrbitData, 0);//this is for use when satellite makes it to orbit 
 
 P1DIR &= ~(BIT0);//TURNS ONLY P1.0 as an input
 P1IES &= ~BIT0;// sets up interrupt to be a low to high transition
 P1IFG &= ~(BIT0);//you clear the flag before the interrupt is set
 P1IE  |= BIT0;//sets up interrupt 
 
}
*//*
void verify_launch_int(void) __interrupt[PORT2_VECTOR]{//this is an interrupt function the _interrupt tells its a function interrupt 
//placed in code that sets the flag

  if (P2IFG&BIT7){

                  P2IFG &= ~BIT7;
                  BUS_lp_mode=ML_LPM0;

                  //this is for launch 
                  ctl_events_set_clear(&handle_LaunchDetect,1<<0,0);//this is for use during analysis of if there is a current launch happening                     
                   //use these two when doing testing 
                   //switch_is_on=1;
                  //ctl_events_set_clear(&handle_LaunchData,LAUNCHDATA_FLAG,0);//this should start the data logging 
                
                  }

  else if (P2IFG&BIT6){//this interrupt is used for testing purposes.
   //the stitch used is a push button switch. The switch is depressed and the interrupt input is connected to vcc momentarlly causing an interrupt. 
                       
                       //turn off the flag 
                       P2IFG &= ~BIT6;
                       //switch has just been selected, switch_is_on is checked for initial conditions and enters this code to begin taking data. 
                       if (switch_is_on==0)
                       {
                        //switch_is_on is a state for the launch_data_log 
                        switch_is_on=1; 
                        ctl_events_set_clear(&handle_LaunchData,LAUNCHDATA_FLAG,0);//this should start the data logging 
                         
                       }
                     else 
                      { 
                        switch_is_on=0;//turn off, so code will exit out of launch_data_log()
                        
                      }

        }
  else if (P2IFG&BIT5){
  P2IFG=0;
  }
  else if (P2IFG&BIT4){
  P2IFG=0;
  }
  else if (P2IFG&BIT3){
  P2IFG=0;
  }
  else if (P2IFG&BIT2){
  P2IFG=0;
   }
  else if (P2IFG&BIT1){
  P2IFG=0;
  }
  else  {
  P2IFG=0;
   }
 
}
*/
void verify_launch_int(void) __interrupt[PORT1_VECTOR]{//this is an interrupt function the _interrupt tells its a function interrupt 
//placed in code that sets the flag
  if (P1IFG&BIT7){

                  P1IFG &= ~BIT7;
                  //BUS_lp_mode=ML_LPM0;
                  //use a counter to count how many times it triggers for each cycle 
                  vib_tab_count++;

                  //below is for original code 
                  //this is for launch 
                  //ctl_events_set_clear(&handle_LaunchDetect,1<<0,0);//this is for use during analysis of if there is a current launch happening                     
                   //use these two when doing testing 
                   //switch_is_on=1;
                  //ctl_events_set_clear(&handle_LaunchData,LAUNCHDATA_FLAG,0);//this should start the data logging 
                
                  }

  else if (P1IFG&BIT6){//this interrupt is used for testing purposes.
   //the stitch used is a push button switch. The switch is depressed and the interrupt input is connected to vcc momentarlly causing an interrupt. 
      extern unsigned checking_for_launch; 
      //P2IE&=~BIT6;//TURN OFF, IT MIGHT TRIGGER AN INTERRUPT
                       //turn off the flag 
                       P1IFG &= ~BIT6;
                       
                       //switch has just been selected, switch_is_on is checked for initial conditions and enters this code to begin taking data. 
                       if (checking_for_launch==0)
                       {
                       //need to have a setteling time for the voltage regulator
                        //switch_is_on is a state for the launch_data_log, will turn on data logging  
                        switch_is_on=1;
                        testing_start=1;//set it as 1 so next time around we will exit the data logging loop and turn off the LED
                        //this is for test launch 
                        ctl_events_set_clear(&handle_LaunchData,LAUNCHDATA_FLAG,0);//this should start the data logging 
                        //P2IES |= BIT6;// switch interrupt pin for toggle switch LOW TO HIGH TRANSISTION
                        //P2IE|=BIT6;//TURN BACK ON 
                       }
                     else 
                      { 
                        //need to have a setteling time for the voltage regulator
                        testing_start=0;
                        switch_is_on=0;//turn off, so code will exit out of launch_data_log(
                        //P2IES &= ~BIT6;// switch interrupt pin for toggle switch 
                        //P2IE|=BIT6;//TURN BACK ON 
                        checking_for_launch=0;//this stops the data logging loop 
                      }

        }
  else if (P1IFG&BIT5){
  P1IFG=0;
  }
  else if (P1IFG&BIT4){
  P1IFG=0;
  }
  else if (P1IFG&BIT3){
  P1IFG=0;
  }
  else if (P1IFG&BIT2){
  P1IFG=0;
   }
  else if (P1IFG&BIT1){
  P1IFG=0;
  }
  else  {
  P1IFG=0;
   }
 
}
/*
//Code will verify if there is a launch 
void VerifyLaunchDetect(void *p){

 float checkacc[6],xavg,yavg,zavg;
 float totalacc;
 unsigned int data;
 int i=0,accel_count_for_launch_test=0,checking_launch=0;

 float acc18gconversion = 0.057; // volts per g
 float adcconversion = 0.0008057; // volts per adc reference 0 to 4096
 float go_launch_value=0.00731;//this value is when a launch occures 0.0855^2
 unsigned e;
printf("\rLEDL launch test. Press s to stop. \r\n>");// 
adcsetup();
checking_launch=1;
 while(checking_launch)
 {
   unsigned checking_for_launch;
   checking_for_launch=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_LaunchDetect,(1<<0),CTL_TIMEOUT_NONE,0);
   while(checking_for_launch&(0x01))
   //if(checking_for_launch&LAUNCH_DETECT_FLAG)
   {
   //P7OUT = BIT7;
   //P7DIR = BIT7;

   //Code to check for acceleration, just pulled code from log_data file
   ctl_events_set_clear(&handle_LaunchDetect,0, 0x01);
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_adc,(1<<0),CTL_TIMEOUT_NONE,0);
      if(e&(0x01))
      {
        volatile unsigned int *arr = &ADC12MEM0;// creates a pointer, looks at the memory of mem0, checks adc register 
            
           
                for (i=0; i<6; ++i)// look at first 6 adc measurements of acceleration  
                {
                  checkacc[i]=arr[i];
                  //data is represented as a number from 0 to 4096 ( 0 V to 3.3 V), 0.0008057; volts per adc reference 0 to 4096
                  //a voltage conversion for the accelerometers
                  //   ADXL321 18 g accelerometer   57.0 mV per g 
                  //   ADXL001 70 g accelerometer   16 mV per g
                  checkacc[i]*=adcconversion;
                  checkacc[i]-=1.65;
                  checkacc[i]=fabsf(checkacc[i]);
                  
                }
                  accel_count_for_launch_test++;
                  if (accel_count_for_launch_test==1)
                  {
                  xavg=checkacc[5];//keep a running average on the samples
             //     printf("Xavg = %f",xavg);
                  yavg=checkacc[4];//keep a running average on the samples
             //     printf("Yavg = %f",yavg);
                  zavg=checkacc[3];//keep a running average on the samples. 
              //    printf("Zavg = %f",zavg);
                  }
                  else
                  {
                  xavg=(xavg+checkacc[5])/2;//keep a running average on the samples
                 // printf("Xavg = %f",xavg);
                  yavg=(xavg+checkacc[4])/2;//keep a running average on the samples 
                 // printf("Yavg = %f",yavg);
                  zavg=(xavg+checkacc[3])/2;//keep a running average on the samples. 
                //  printf("Zavg = %f",zavg);
                  if (accel_count_for_launch_test==4100)
                    {         
                      totalacc=xavg*xavg+yavg*yavg+zavg*zavg;//find the total squared acceleration vector
                        if (totalacc>go_launch_value)
                        {
                        //begin logging data
                         switch_is_on=1; 
                         checking_launch=0;
                         printf("WE HAVE LAUNCH");
                         accel_count_for_launch_test=0;
                         P4OUT^=BIT5;
                        
                         ctl_events_set_clear(&handle_LaunchData,LAUNCHDATA_FLAG,0);//this should start the data logging
                        }
                        else{
                        //go back to sleep 
                        checking_launch=0;
                        printf("NO Launch, return to sleep");
                        accel_count_for_launch_test=0;
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







