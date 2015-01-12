#include <__cross_studio_io.h>
#include <MSP430.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <InilitizeClock.h>
#include <ctl.h>//this allows the use of tasking
#include <terminal.h>//this echos what is written into the terminal screen, this allows for commands to be used and looked at in terra term
#include <string.h>
#include "uart.h"
#include "MSPtest.h"
#include "OscillatorTest.h"
#include "SensorsOnOff.h"
#include "ADC_setup.h"
#include "log_data.h"//quotes for files in a project
#include <i2c.h>//brackets for files in a library include this file when ever the i2c functions are used. 
#include "I2C_sensor.h"
#include "ARCbus.h"
#include "timer_setup.h"
#include <terminal.h>
#include <SDlib.h>
#include "SetUp.h"
#include "LaunchDetect.h"
#include "LEDL.h"



//To enable use of the tera Term communication this defines _putchar so that the uart files are usable
int __putchar(int ch){
return async_TxChar(ch);//This is used for sending UART characters to ARCbus This is initialized from second board in stack 
//return TxChar(ch);//This is used for sending UART characters to mini USB drive. Dont forget to initialize this below if it is being used 
}

int __getchar(void){
return async_Getc();
}

//Creating task structure 
CTL_TASK_t mainTask;
CTL_TASK_t Perif_SetUp; 
CTL_TASK_t I2C;
CTL_TASK_t LaunchDetect;
CTL_TASK_t LaunchData;
CTL_TASK_t LEDL_events;


//Create size of stack necessary for storage of task information during task change times
unsigned stack1[1+256+1];   
unsigned stack2[1+400+1];
unsigned stack3[1+100+1];
unsigned stack6[1+200+1];

//unsigned stack5[1+100+1];


void main(void){



//Initialize the clock to run the microprocessor
ARC_setup_lv();//sets up initializes the clocks and should be called at the beginning of main
//initCLK();//this is now set up in ARC_setup 
//Initialize the uart to talk with terra term 

VREGinit();//INITALIZE THE VOLTAGE REGULATOR 
//initUART();//initalize when using TxChar(ch)
//setup I2C for use of UCB1 
initI2C();
 
//set up timer
init_timerA(); // some of the set up is now started in ARC_setup 
mmcInit_msp();
setup_launch_detect();
//setup_orbit_start();
UnusedPinSetup();//drive all unused pins to outputs 
SENSORSoff();
Gyroinit();
MAGoff();
ACCoff();
mmcInit_msp_off();
RESET_LED();

//Initialize the main task 
initARCbus(BUS_ADDR_LEDL); 
//ctl_task_init(&mainTask, 255, "main");//When you start the tasking you start the interrupts //this is taken care of in ARC_bus 
//set up timerA
//timersetup();
//test print

P4DIR |= BIT1|BIT2|BIT3|BIT5|BIT6|BIT7;//LIGHT UP LED'S AS OUTPUTS 
P4OUT &= ~(BIT1|BIT2|BIT3|BIT5|BIT6|BIT7);





//run MSP test
/*
MSPtest();
//RUN Oscillator Test
OscillatorTest();
//Turn temperature sensor/gyro/SD card on
SENSORSon();
//Turn temperature sensor/gyro/SD card off
 SENSORSoff();
 //Turn accelerometers on
 ACCon();
 //turn accelerometers off
 ACCoff();
 //Turn magnetometers on
 MAGon();
 //Turn magnetometers off
 MAGoff();
 */
 //Test the ADC functionality
 //ADC_test();
 //logging data for launch 
 //launch_data_log();

 //test  finished
 //printf("YOU ARE FINISHED!!!\n\r");

memset(stack1,0xcd,sizeof(stack1));//function memset, sets all values(array stack1, everything equals this value, 
//size of array so everything is changed)
stack1[0]=stack1[sizeof(stack1)/sizeof(stack1[0])-1]=0xfeed;//put marker values at the words before/after the stack. 

memset(stack2,0xcd,sizeof(stack2));  // write known values into the stack
stack2[0]=stack2[sizeof(stack2)/sizeof(stack2[0])-1]=0xfeed; // put marker values at the words before/after the stack

memset(stack3,0xcd,sizeof(stack3));  // write known values into the stack
stack3[0]=stack3[sizeof(stack3)/sizeof(stack3[0])-1]=0xfeed; // put marker values at the words before/after the stack

//memset(stack5,0xcd,sizeof(stack5));  // write known values into the stack
//stack5[0]=stack5[sizeof(stack5)/sizeof(stack5[0])-1]=0xfeed; // put marker values at the words before/after the stack

memset(stack6,0xcd,sizeof(stack6));  // write known values into the stack
stack6[0]=stack6[sizeof(stack6)/sizeof(stack6[0])-1]=0xfeed; // put marker values at the words before/after the stack

ctl_events_init(&handle_SDcard, 0);


//start timer A (taken from Jesse's code so that I can have an interrupt for my timer)
//start_timerA();
ctl_task_run(&Perif_SetUp,2,(void(*)(void*))Periferial_SetUp,"ARC Bus Test Program","Periferial_SetUp",sizeof(stack2)/sizeof(stack2[0])-2,stack2+1,0);//side note, the termainal can be used in two ways by either passing the uart functin or the async function 
ctl_task_run(&I2C,2,(void(*)(void*))takeI2Cdata,NULL,"takeI2Cdata",sizeof(stack3)/sizeof(stack3[0])-2,stack3+1,0);
ctl_task_run(&LaunchData,2,launch_data_log,NULL,"launch_data_log",sizeof(stack1)/sizeof(stack1[0])-2,stack1+1,0);//&LaunchData takes the address
ctl_task_run(&LEDL_events,2,sub_events,NULL,"sub_events",sizeof(stack6)/sizeof(stack6[0])-2,stack6+1,0);//this is to run orbit code
//ctl_task_run(&LaunchDetect,4,VerifyLaunchDetect,NULL,"VerifyLaunchDetect",sizeof(stack5)/sizeof(stack5[0])-2,stack5+1,0);
//of the variable which is the task structure is ,2 is the priority,launch_data_log is the function I want to run,"launch_data_log" is 
//the name when I look at the threads window to identify the task,the size of the memory stack minus the guard bits,
//first location where data is stored second element in array (first element is guard bit), the zero is a placeholder
//since the MSP doesn't support this function. 

//Use I2C sensor function to receive data


 //Stay in an infinite loop
//for(;;){// taken care of in main loop
//P5SEL |= BIT6;//OUTPUT aclk
//P5DIR |= BIT6;//output aclk
//mainLoop_lp();
mainLoop();

}


//==============[task library error function]==============

//something went seriously wrong
//perhaps should try to recover/log error
/*void ctl_handle_error(CTL_ERROR_CODE_t e){
  switch(e){
    case CTL_ERROR_NO_TASKS_TO_RUN: 
      __no_operation();
      //puts("Error: No Tasks to Run\r");
    break;
    case CTL_UNSUPPORTED_CALL_FROM_ISR: 
      __no_operation();
      //puts("Error: Wait called from ISR\r");
    break;
    case CTL_UNSPECIFIED_ERROR:
      __no_operation();
      //puts("Error: Unspesified Error\r");
    break;
    default:
      __no_operation();
      //printf("Error: Unknown error code %i\r\n",e);
  }
  //something went wrong, reset
  WDTCTL=0;
}*/

//I need code to do what? Testing document
//Start up code, how do I want to make sure the MSP is working properly 
  // when in start up mode what does the MSP do. 
  //check pins? how would I do that. 

//make a test to make sure the oscillator is functioning properly, perhaps no code needs to be written, probe crystal?

//Need UART Terra Term Communication, this is already written, need to understand it better

//sensor on off test
  //This may not need a task but may be a function 
   //In my LEDL board controls a PMOS, when high, sensors are off, when low, sensors are on. 
   //P7.7 Sensors on/off    //P8.6 ACC on/off    //P8.7 MAG on/off
   //perhaps in addition to measuring the voltage on the pins I could flash my LEDS for each pin individually 

//measure ADC and verify it is correct, see if I can take at 4000 Hz. 
  //I already have code set up to measure the ACD. Perhaps I can set it up into two different tasks, one for launch (Mode 1 ADC) and one for orbit (Mode 2 ADC). 
  //Mode 1 ADC: measure ACCEL and GYRO and place into buffer. Where is this buffer currently, 
  //Mode 2 ADC: measure GYRO and place into buffer. Same buffer as above. 

//Sensor I2C test
  // I have no code for this, I think Jordan got code to work for the temperature sensors
  // It would be nice to see these temperature sensors in the Mode 1 Matrix 

//Memory Card Test
  //I believe there is code for this 
  //test code would to be to write something into memory, it probably will be a 512 byte block and read out the block and verify it is the same.
    
   //After success in first SD card code, move data blocks from sampled data into SD card. This code should be written. 

//Gyro rotation test, I want to output measured values from ADC so I can compare to the rotation table. 
  //this may be an additional function, I can't do this test till I have a Board, cannot solder gyros on house boards. 
  //a function that saves the data to memory, extract memory for document. 

//Solar Panel test, test integration off all SPB boards 
  // this would include measuring accel, gyro, magnetometers, and temperature sensors and output the measured values to compare. 

//Piezo electric test, see if LEDL would turn on if vibrations or a voltage at a pin. 

//Clyde integration test, see if I can get Clyde data 

//CDH test, test sending packets and packages over the SPI 
