//This file will cover all stack up events
#include <msp430.h>
#include <ctl.h>
#include <ARCbus.h>
#include <Error.h>
#include <SDlib.h>
#include "subsystem.h"

//static int launch_data[LAUNCH_DATA_SIZE];
unsigned long SDaddr=SD_LAUNCH_DATA_START;//STARTING SDaddr at 100
static int *launch_data1;
static int *launch_data2;
int temp_measure[TEMP_ARRAY_SIZE];
int clyde_data[CLYDE_ARRAY_SIZE];
int clyde_measure;
int mag_measure;
long wake_up_attempt=0;
int result;
CTL_TASK_t SD_card;
CTL_EVENT_SET_t SYS_evt; // This creates the event struct,change SYS to actual subsystem
CTL_EVENT_SET_t handle_SDcard;//sd card event structure 
unsigned stack4[1+300+1];//stacks need to be a globals 


//handle subsystem specific commands - this is for I2C commands on the BUS that are not SUB events, so system specific commands.
int SUB_parseCmd(unsigned char src,unsigned char cmd,unsigned char *dat,unsigned short len){
  switch(cmd){
    /*case 0x14:
      P7OUT=0xFF;
      P7DIR=0xFF;
      break;*/
    default:
      return ERR_UNKNOWN_CMD;
  }
}


//parse subsystem events
void sub_events(void *p) __toplevel{
  unsigned int e,len;
  int i;
  unsigned char buf[10],*ptr;
  extern unsigned char async_addr;
  memset(stack4,0xcd,sizeof(stack4));  // write known values into the stack
  stack4[0]=stack4[sizeof(stack4)/sizeof(stack4[0])-1]=0xfeed; // put marker values at the words before/after the stack

  for(;;){
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&SUB_events,SUB_EV_ALL|SUB_EV_ASYNC_OPEN|SUB_EV_ASYNC_CLOSE,CTL_TIMEOUT_NONE,0);
    if(e&SUB_EV_PWR_OFF){  //**************************************** Sent when the subsystem receives the power off command **********************************
      //print message
      puts("System Powering Down\r");
    }
    if(e&SUB_EV_PWR_ON){ //***************************************** Sent when the subsystem receives the power on command ************************************
      //print message
      puts("System Powering Up\r");
    }
    if(e&SUB_EV_SEND_STAT){ // ************************************* Sent when the subsystem receives the send status command  ********************************
      //send status
      puts("Sending status\r");
      //setup packet 
      //ptr=BUS_cmd_init(buf,CMD_SYS_STAT); // replace CMD_SYS_STAT with actual subsystem ie CMD_COMM_STAT
      //send command
      //BUS_cmd_tx(BUS_ADDR_CDH,buf,0,0); //sending status contained in "buf" CDH 

      // FOR TESTING SEND_I2C command!!

    }
    if(e&SUB_EV_SPI_DAT){ // ************************************** Sent when SPI data is received correctly **************************************************
      puts("SPI data rx\r");
    }
    if(e&SUB_EV_SPI_ERR_CRC){ // ********************************* Sent when SPI transaction was rejected because of busy buffer *****************************
      puts("SPI bad CRC\r");
    }
    if(e&SUB_EV_SPI_ERR_BUSY){
      puts("SPI Busy\r");
    }
    if(e&SUB_EV_ASYNC_OPEN){ // ********************************** An async connection was opened remotely **************************************************
      //close async connection, not supported
      puts("Async open called");
    }
    if(e&SUB_EV_ASYNC_CLOSE){ // ********************************** An async connection was closed remotely  *************************************************
     //close async connection, not supported
     puts("Async close called");
    }
    if(e&SUB_EV_INT_0){ // ************************************** Event on interrupt bus pin 0  *************************************************************  
    puts("EV 0 has been called.");
    }
  }
}



int adctemp; 
int adctempbuffer[6]; 
void sys_events(void *p) __toplevel{
  unsigned int e;
  int i=0; 
  ctl_task_run(&SD_card,BUS_PRI_EXTRA_HIGH,(void(*)(void*))writedatatoSDcard,NULL,"writedatatoSDcard",sizeof(stack4)/sizeof(stack4[0])-2,stack4+1,0);//set up task to run SD card,         
  ctl_events_init(&SYS_evt,0); //Initialize Event
  
  for(;;){
    // wait for events
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&SYS_evt,SYS_EVT_ALL,CTL_TIMEOUT_NONE,0);
//*************************************************** Events to be processed for system ****************************************************************
    if(e&SYS_EV_1){
 
    puts("SYS_EV_1 called\r\n");
    printf("A%d : %d\r\n",i,adctemp);
    adctempbuffer[i]=adctemp;

    i++;
      if (i==5){
      i=0;
     /*stop_ADC_sampeling();
     stop_timerA3();*/
      };
    }
    if(e&SYS_EV_2){
    puts("SYS_EV_2 called\r\n");
    }
    if(e&SYS_EV_3){
    puts("SYS_EV_3 called\r\n");
    }
    if(e&SYS_EV_4){
    puts("SYS_EV_4 called\r\n");
    }
    else{
     puts("ELSE event called\r\n");
    }
  }
}


// Created event to store data to sd card 
void writedatatoSDcard(void)
{
  unsigned countforLED=0; 
  unsigned long SD_card_write_time;
  unsigned SD_card_write;
  unsigned count_error; 
  int mmcReturnValue;
  
  for(;;){//I may want to add code that looks for a new spot to put data if there is data in code(SD card code). 
      SD_card_write=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_SDcard,SD_EV_ALL,CTL_TIMEOUT_NONE,0);//the 7 means your looking at all 3 flags 
         if(SD_card_write&(SD_EV_WRITE_1))
           {  //SD_card_write_time=get_ticker_time();  
             // printf("SD card write start %u\t",get_ticker_time());
              result = mmcWriteBlock(SDaddr,(unsigned char*) launch_data1); //(unsigned char*) casting my pointer(array) as a char 
              if(result<0){
              LED_5_ON();
                count_error++;
                 if(count_error==3)
                   {
                      SENSORSon();
                      //initalize the SD card   
                      mmc_pins_on();
                      mmcReturnValue=mmcInit_card();
                      if (mmcReturnValue==MMC_SUCCESS){
                      printf("\rCard initalized Sucessfully\r\n");
                      count_error=0;
                      }
                   }
              }
              else{
              LED_5_OFF();
              SDaddr+=1;//memory card is block address
              //printf("SD card #1 returned, %i\r\n",result);
              //SD_card_write_time=get_ticker_time();  
              //printf("SD card write finish %u\t",get_ticker_time());
              countforLED++;
              if (countforLED==100)
              {
              SD_LED();
              countforLED=0;
              }
              }
              
              }
      if(SD_card_write&(SD_EV_WRITE_2))
           {
              result= mmcWriteBlock(SDaddr,(unsigned char*) launch_data2); //(unsigned char*) casting my pointer(array) as a char 
              if(result<0){
              LED_5_ON();
               count_error++;
                 if(count_error==3)
                    {
                    SENSORSon();
                    //initalize the SD card   
                    mmc_pins_on();
                    mmcReturnValue=mmcInit_card();
                    if (mmcReturnValue==MMC_SUCCESS){
                    printf("\rCard initalized Sucessfully\r\n");
                    count_error=0;
                    }
                    }
              }
              else{
              LED_5_OFF();
              SDaddr+=1;//memory card is block address
           //   printf("SD card #2 returned, %i\r\n",result);
              countforLED++;
              if (countforLED==100)
              {
              SD_LED();
              countforLED=0;
              }
              }
              
            }
      if(SD_card_write&(SD_EV_DIE))
          {
           int SDaddr_for_total_blocks_used=0;
           launch_data2[0]=SDaddr;
           SD_LED_OFF();
           result= mmcWriteBlock(SDaddr_for_total_blocks_used,(unsigned char*) launch_data2); //(unsigned char*) casting my pointer(array) as a char   
           ctl_events_set_clear(&handle_SDcard,SD_EV_FINISHED,0);//this is to tell that the SD card is finished writing and can be shut down if necessary 
          //  printf("SD card shutdown \r\n");
           return;
           LED_5_OFF();
          }
         }
         
}
