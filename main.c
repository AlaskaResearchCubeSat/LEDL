#include <msp430.h>
#include <ctl.h>
#include <ARCbus.h>
#include <Error.h>
#include <terminal.h>         
#include <UCA2_uart.h>        // UART setup 
#include "pins.h"             // linked in bare_bones solution 
#include "subsystem.h"        // linked in bare_bones solution 
#include "log_data.h"

CTL_TASK_t terminal_task,sub_task,sys_task,LEDL_events, I2C,LaunchData; // name your task (first thing to do when setting up a new task (1))

//********************************************* allocate mem for tasks (2)
//stack for terminal
unsigned terminal_stack[2000];
//stack for subsystem events
unsigned sys_stack[1000];
//stack for bus events
unsigned sub_stack[1000];
//stack for LEDL Launch function 
unsigned stack6[1+200+1];
unsigned stack1[1+300+1];   
unsigned stack3[1+200+1];


//******************************************** redefine putchar and getchar 
//make printf and friends use async
int __putchar(int c){
  return UCA2_TxChar(c);
}

//make printf and friends use async
int __getchar(void){
  return UCA2_Getc();
}

//******************************************* Example_Bare_bones main loop
void main(void){
  //turn on LED's this will flash the LED's during startup
  P7OUT=0xFF;
  P7DIR=0xFF;
  //init complete turn on LED0 and all others off
  P7OUT=0x1F;
 

  
  //DO this first
  ARC_setup(); 

  //TESTING: set log level to report everything by default
  set_error_level(0);

  //initialize UART
  UCA2_init_UART(UART_PORT,UART_TX_PIN_NUM,UART_RX_PIN_NUM);

  //setup bus interface
  initARCbus(0x1F);   // Default addr for "SYS" subsystem, should be changed for specific subsystems.
  //set up timer 
  init_timerA3();
  //set up adc 
  adcsetup();


  // initialize stacks (3)
  memset(stack1,0xcd,sizeof(stack1));//function memset, sets all values(array stack1, everything equals this value, 
  //size of array so everything is changed)
  stack1[0]=stack1[sizeof(stack1)/sizeof(stack1[0])-1]=0xfeed;//put marker values at the words before/after the stack.
  memset(stack3,0xcd,sizeof(stack3));  // write known values into the stack
  stack3[0]=stack3[sizeof(stack3)/sizeof(stack3[0])-1]=0xfeed; // put marker values at the words before/after the stack
  memset(terminal_stack,0xcd,sizeof(terminal_stack));                                           //write known values into the stack 
  terminal_stack[0]=terminal_stack[sizeof(terminal_stack)/sizeof(terminal_stack[0])-1]=0xfeed;  //put marker values at the words before/after the stack
  memset(sys_stack,0xcd,sizeof(sys_stack));                                                     //write known values into the stack 
  sys_stack[0]=sys_stack[sizeof(sys_stack)/sizeof(sys_stack[0])-1]=0xfeed;                      //put marker values at the words before/after the stack
  memset(sub_stack,0xcd,sizeof(sub_stack));                                                     //write known values into the stack 
  sub_stack[0]=sub_stack[sizeof(sub_stack)/sizeof(sub_stack[0])-1]=0xfeed;                      //put marker values at the words before/after the stack
  memset(stack6,0xcd,sizeof(stack6));  // write known values into the stack
  stack6[0]=stack6[sizeof(stack6)/sizeof(stack6[0])-1]=0xfeed; // put marker values at the words before/after the stack

  ctl_events_init(&handle_SDcard, 0);


  // creating the tasks
  ctl_task_run(&terminal_task,BUS_PRI_LOW,terminal,"Example Bare Bones program ready","terminal",sizeof(terminal_stack)/sizeof(terminal_stack[0])-2,terminal_stack-1,0);
  ctl_task_run(&sys_task,BUS_PRI_NORMAL,sys_events,NULL,"SYS_events",sizeof(sys_stack)/sizeof(sys_stack[0])-2,sys_stack-1,0);
  ctl_task_run(&sub_task,BUS_PRI_HIGH,sub_events,NULL,"SUB_events",sizeof(sub_stack)/sizeof(sub_stack[0])-2,sub_stack-1,0);
  ctl_task_run(&LEDL_events,BUS_PRI_NORMAL+10,sub_events,NULL,"sub_events",sizeof(stack6)/sizeof(stack6[0])-2,stack6+1,0);//this is to run orbit code
  ctl_task_run(&I2C,BUS_PRI_NORMAL,(void(*)(void*))takeI2Cdata,NULL,"takeI2Cdata",sizeof(stack3)/sizeof(stack3[0])-2,stack3+1,0);
  ctl_task_run(&LaunchData,BUS_PRI_HIGH,launch_data_log,NULL,"launch_data_log",sizeof(stack1)/sizeof(stack1[0])-2,stack1+1,0);//&LaunchData takes the address
 
  //mainLoop_lp();
  //main loop <-- this is an ARCbus function 
  mainLoop(); 

}

//decode errors
char *err_decode(char buf[150], unsigned short source,int err, unsigned short argument){
  sprintf(buf,"source = %i, error = %i, argument = %i",source,err,argument);
  return buf;
}

