#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include <ctl.h>
#include <ARCbus.h>
#include "log_data.h"
#include <SDlib.h>
#include <Error.h>
#include "SensorsOnOff.h"
#include "I2C_sensor.h"
#include "LEDL.h"

  void CLYDE_STAT_LED_toggle(void){
  P4OUT^=BIT3;
  }

  void LEDL_STAT_LED_toggle(void){
  P4OUT^=BIT2;
  }

  void ERR_LED_on(void)
 {
 P4OUT^=BIT5;
 }
 
void sub_events(void *p) __toplevel{
  unsigned int e,len;
  int i,resp;
  extern CTL_TASK_t tasks[3];
  extern startup_mode;
  LEDL_STAT ledl_status;
  CLYDE_STAT clyde_status;
 
  unsigned char buf[BUS_I2C_HDR_LEN+sizeof(LEDL_STAT)+BUS_I2C_CRC_LEN],*ptr;
  if (P1IN&BIT0)
  {
  ctl_events_set_clear(&SUB_events,SUB_EV_INT_0,0);
  }
  for(;;){
    e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&SUB_events,SUB_EV_ALL,CTL_TIMEOUT_NONE,0);
    if(e&SUB_EV_INT_0){
      //CDH is on
      //switch has just been selected, switch_is_on is checked for initial conditions and enters this code to begin taking data. 
                    volatile unsigned int *arr = &ADC12MEM8;// creates a pointer, looks at the memory of mem0
                    int resp; 
                    int *buffer=NULL;//changed from char to int instead of each char being 1 byte their are now two bytes, resulting in being half as many things to index 
                    unsigned long SDaddr_for_total_blocks_used=0;
                    LEDL_TEST_LAUNCH *launch_detect_data;
                   int mmcReturnValue;
                   startup_mode=MODE_ORBIT;
                   switch_is_on=0;//turn off launch code
                   
                   //turn up the voltage
                   //NEED TO BLOW FUSE CODE 
                  // LEDL_BLOW_FUSE();
                   //SWITCH TO CLYDE POWER
                   LEDL_SWITCH_TO_EPS();
                  //need to have a setteling time for the voltage regulator
                  //then initalize the 16 Mhz clock 
                  ctl_timeout_wait(ctl_get_current_time()+5);//4.88 MS
                  SENSORSon();//turn on sensor
                  //turn on sensors here 
                  MAGon();//turn on magnetometers 
                  GyroWakeUp();
                  initCLK();
                  ADC12CTL0&=~ENC;
                  ADC12CTL1|=SHS_0|CONSEQ_1;//sets adc to sample and hold souce select on adc12sc, and conseq choses sequence of channels
                  ADC12CTL0|=ENC;//enc enables the adc, and ADC12sc starts the sample and conversion
                  ADC12IFG=0;//CLEAR ALL FLAGS 
                  ADC12IE = BIT8|BITA; // ENABLE THE INTERUPTS FOR IE7

                  //initalize the SD card 
                  ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 
                  mmc_pins_on();
                  mmcReturnValue=mmcInit_card();
                  if (mmcReturnValue==MMC_SUCCESS){
                  printf("\rCard initalized Sucessfully\r\n");
                  
                  }
                  else {
                  printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
                  } 
                     
  //need to take data so that LEDL is ready to send data when cdh requests it 
           

           ADC12CTL0&=~ENC;     //disable ADC
           ADC12CTL1&=~CSTARTADD_15;   //clear CSTARTADD
           ADC12CTL1|= CSTARTADD_8;    // CSTARTADD
           ADC12CTL0|=ADC12SC|ENC;//need another comment 
          clyde_take_data(clyde_data);
          Temp_I2C_sensor(temp_measure);

          
           //ledl_status.gyro_data[0]=arr[0];
           //ledl_status.gyro_data[1]=arr[1];
           //ledl_status.gyro_data[2]=arr[2];
        
           for (i=0;i<7;i++)
           {//convert temperature to 8 bit value, do we want to lose the quarter degree resolution 
           }
           //get how many blocks have been written to the SD card
          //get buffer, set a timeout of 2 secconds
           
           //get buffer, set a timeout of 2 secconds
            buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
            launch_detect_data=(LEDL_TEST_LAUNCH*)buffer;
            //check for error
            if(buffer==NULL){
              printf("Error : Timeout while waiting for buffer.\r\n");
              //TODO: SPECIFY A ERROR FUNCTIION FOR THIS 
              //return -1; 
            }else{
              //read from SD card
              resp=mmcReadBlock(SDaddr_for_total_blocks_used,(unsigned char*)buffer);
              //store the last SDcard location that is stored in memory, this will either be by the end of logging data when Clyde turns on, or if ledl resets if 
              //it has decided that logging data has happened or if ledl has had a restart
              ledl_status.SDaddress=launch_detect_data->dat.detect_dat.SD_last_address;
              //print response from SD card
              printf("%s\r\n",SD_error_str(resp));
              BUS_free_buffer();
            }

                   //if we get ride of the quarter degree resolution than the structure has to be changed to support 8 bit value, or be clever. 2 data in one slot... 
                   for(i=0;i<7;i++){
                   ledl_status.temp_data[i]=temp_measure[i];
                   }
                   for(i=0;i<2;i++)
                   {
                   ledl_status.clyde_array_two_int[i]=clyde_data[i+15];
                   ledl_status.clyde_array_two_char[i]=clyde_data[i+17];//need to verify this
                   }
                   ledl_status.clyde_ttc=clyde_data[19];//need to verify this
                   for(i=0;i<15;i++)
                   {
                   clyde_status.clyde_array_fifteen[i]=clyde_data[i];
                   }    
      

    }
    if(e&SUB_EV_PWR_OFF){
        //print message
        puts("System Powering Down\r\n");
    }
    if(e&SUB_EV_PWR_ON){
        //print message
        puts("System Powering Up\r\n");
       
    }
    if((e&SUB_EV_SEND_STAT&&(startup_mode==MODE_ORBIT))){
           //set up stuff to take data agian 
           
          //retrive data for status 
          volatile unsigned int *arr = &ADC12MEM8;// creates a pointer, looks at the memory of mem0
            int resp; 
            int *buffer=NULL;//changed from char to int instead of each char being 1 byte their are now two bytes, resulting in being half as many things to index 
            
            LEDL_TEST_LAUNCH *launch_detect_data;

           ADC12CTL0&=~ENC;     //disable ADC
           ADC12CTL1&=~CSTARTADD_15;   //clear CSTARTADD
           ADC12CTL1|= CSTARTADD_8;    // CSTARTADD
           ADC12CTL0|=ADC12SC|ENC;//need another comment 
           //TURN ON I2C LINE FOR CLYDE 
           P7DIR |= BIT4;
           P7OUT |= BIT4;
           
           //send ledl_status
                  puts("Sending ledl_status\r\n");
                  //get ledl_status data
                  //setup packet 
                  ptr=BUS_cmd_init(buf,CMD_LEDL_STAT);
                  //fill in telemitry data
                  for(i=0;i<sizeof(LEDL_STAT);i++){
                    ptr[i]=((unsigned char*)(&ledl_status))[i];
                   }
                
                //wait to avoid conflicts
                ctl_timeout_wait(ctl_get_current_time()+50);
                //send command
                //resp=BUS_cmd_tx(BUS_ADDR_CDH,buf,sizeof(LEDL_STAT),0,BUS_I2C_SEND_FOREGROUND);
                resp=BUS_cmd_tx(BUS_ADDR_CDH,buf,18,0,BUS_I2C_SEND_FOREGROUND);
                if(resp!=RET_SUCCESS){
                  //report error
                  //report_error(ERR_LEV_ERROR,LEDL_ERR_SRC_SUBSYSTEM,LEDL_ERR_SUB_STAT_TX,resp);
                  //turn on error LED
                  ERR_LED_on();
                }else{
                  LEDL_STAT_LED_toggle();
                }
                  //send clyde_status
                  puts("Sending clyde_status\r\n");
                  //get clyde_status data
                  //setup packet 
                  ptr=BUS_cmd_init(buf,CMD_EPS_STAT);
                  //fill in telemitry data
                  for(i=0;i<sizeof(LEDL_STAT);i++){
                    ptr[i]=((unsigned char*)(&clyde_status))[i];
                  }
                     //wait to avoid conflicts
                ctl_timeout_wait(ctl_get_current_time()+50);
                //send command
                resp=BUS_cmd_tx(BUS_ADDR_CDH,buf,sizeof(CLYDE_STAT),0,BUS_I2C_SEND_FOREGROUND);
                if(resp!=RET_SUCCESS){
                  //report error
                  //report_error(ERR_LEV_ERROR,LEDL_ERR_SRC_SUBSYSTEM,LEDL_ERR_SUB_STAT_TX,resp);
                  //turn on error LED
                  ERR_LED_on();
                }else{
                  CLYDE_STAT_LED_toggle();
                }

          clyde_take_data(clyde_data);
          Temp_I2C_sensor(temp_measure);

          
           //ledl_status.gyro_data[0]=arr[0];
           //ledl_status.gyro_data[1]=arr[1];
           //ledl_status.gyro_data[2]=arr[2];
        
           for (i=0;i<7;i++)
           {//convert temperature to 8 bit value, do we want to lose the quarter degree resolution 
           }
           //get how many blocks have been written to the SD card
          //get buffer, set a timeout of 2 secconds
           
           //get buffer, set a timeout of 2 secconds
            buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
            launch_detect_data=(LEDL_TEST_LAUNCH*)buffer;
            //check for error
            if(buffer==NULL){
              printf("Error : Timeout while waiting for buffer.\r\n");
              //TODO: SPECIFY A ERROR FUNCTIION FOR THIS 
              //return -1; 
            }else{
              //read from SD card
              resp=mmcReadBlock(SD_BECON_DATA ,(unsigned char*)buffer);
              //store the last SDcard location that is stored in memory, this will either be by the end of logging data when Clyde turns on, or if ledl resets if 
              //it has decided that logging data has happened or if ledl has had a restart
              ledl_status.SDaddress=launch_detect_data->dat.detect_dat.SD_last_address;
              //print response from SD card
              printf("%s\r\n",SD_error_str(resp));
              BUS_free_buffer();
            }

                   //if we get ride of the quarter degree resolution than the structure has to be changed to support 8 bit value, or be clever. 2 data in one slot... 
                   for(i=0;i<7;i++){
                   ledl_status.temp_data[i]=temp_measure[i];
                   }
                   for(i=0;i<2;i++)
                   {
                   ledl_status.clyde_array_two_int[i]=clyde_data[i+15];
                   ledl_status.clyde_array_two_char[i]=clyde_data[i+17];//need to verify this
                   }
                   ledl_status.clyde_ttc=clyde_data[21];//need to verify this
                   for(i=0;i<15;i++)
                   {
                   clyde_status.clyde_array_fifteen[i]=clyde_data[i];
                   }    
      

                  

    }

    if(e&SUB_EV_SPI_DAT){
      puts("SPI data recived:\r\n");
      //free buffer
      BUS_free_buffer_from_event();
    }
    if(e&SUB_EV_SPI_ERR_CRC){
        puts("SPI bad CRC\r\n");
    }
    if(e&SUB_EV_SPI_ERR_BUSY){
        puts("SPI packet lost\r\n");
    }
    
  }
}


