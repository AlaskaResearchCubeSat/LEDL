#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include <ctl.h>
#include <ARCbus.h>
#include "ADC_setup.h"
#include "I2C_sensor.h"
#include "log_data.h"
#include <SDlib.h>
#include "SensorsOnOff.h"
#include "sensor-interface.h"
#include <crc.h>
#include <math.h>
#include <i2c.h>
#include "LEDL.h"
#include <Error.h>
#include "Error_src.h"


/*
void ADC_test(void){

 
 unsigned long test=0;//counter for rows also called SFID
  int accel_count=0;//counter to display last two rows of data
  int gyro_count=0;//counter to display gyro
  int frame_100=0;//counter to display temperature 
  int temp_display=0;//counter to display zeros or temp
  int i; //counter to cycle through six adc measurements 
  int signal_mux=0;// making signal that will cause mux to switch between gyro 2 (G2) when signal_mux=0 and gyro 3 (G3) when signal_mux=1 
  int k; //counter for loops of reading data 
  int ADCread;
 

printf("\rLEDL ADC test\r\n");// 
printf("\rmeasure voltage with the MSP ADC.\r\n");
printf("\rtest#\tA0\tA1\tA2\tA3\tA4\tA5\tA6\tA7\r\n");//gives a column title
  timersetup4kHz();//start timer
 // timersetup();//start timer
  adcsetup();//adc setup
   __enable_interrupt();// THIS IS A PREDEFINED GLOBAL INTERRUPT ENABLE BIT IN THE MSP LIBRARY 
  

  //for(;;)
  for(k=0;k<2;k++)//make two loops to makes measurements 
  {
    volatile unsigned int *arr = &ADC12MEM0;// creates a pointer, looks at the memory of mem0
    //while(handle_adc!=0)//in adc interrupt handle_adc is set high, so this code can become initialized
    {
      handle_adc = 0;//reset my personal flag to zero
      if (accel_count==0)// only when accel_count=0 will the row number be displayed
      {
      printf("%d\t", test);//prints row number
      test++;//add count to test 
      }  
        for (i=0; i<8; ++i)// print first 6 adc measurements  
         {
          printf("%d\t", arr[i]);
         }  

         printf("\r\nPlace the results in Document ARC1-LEDL-TST-LEDLPROTO-R01 section ADC Test.\r\n");
         printf("When finished press the space bar to continue.\r\n");
         while(ADCread!=' '){//continues to check to see if the space bar was hit.  
         ADCread = getchar();//We expect the ascii key for return to be in the return value.  
         }ADCread=0;
     }
    }
    }
*/
CTL_EVENT_SET_t handle_SDcard;
CTL_EVENT_SET_t handle_get_I2C_data;
CTL_EVENT_SET_t handle_get_mag_data;
CTL_EVENT_SET_t handle_get_clyde_data;
CTL_TASK_t SD_card;
CTL_TASK_t mag;
CTL_TASK_t clyde;
extern CTL_EVENT_SET_t handle_LaunchDetect;


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


unsigned stack4[1+300+1];//stacks need to be a globals 
int startup_mode;

extern vib_tab_count;

unsigned checking_for_launch;

void launch_data_log(void *p){// used to set up code USE THIS ONE WITH THE TASK LIBRARY 
//void launch_data_log(void){// used to set up code // USE THIS ONE FOR JUST USING IT AS A FUNCITON 
  extern int switch_is_on;
  unsigned long frame = 0;
  unsigned long frameforbitshift = 0;
  unsigned long row=0;//counter for rows also called SFID
  ticker ticker_time;//ticker time gained from the ARC bus clock ticker is a specified timer specefier for this 
  unsigned short time1;
  unsigned short time2;
  unsigned char *buffer;
  int accel_count=0;//counter to display last two rows of data
  int gyro_count=0;//counter to display gyro
  int frame_100=0;//counter to display temperature 
  int temp_display=0;//counter to display zeros or temp
  int i; //counter to cycle through six adc measurements 
  int data=0;//counter to cycle though all data
  int signal_mux=0;// making signal that will cause mux to switch between gyro 2 (G2) when signal_mux=0 and gyro 3 (G3) when signal_mux=1 
  unsigned event1;
  int *launch_data;
  int crc_check;
  //SDaddr+=1; //address already starts at 65

  memset(stack4,0xcd,sizeof(stack4));  // write known values into the stack
  stack4[0]=stack4[sizeof(stack4)/sizeof(stack4[0])-1]=0xfeed; // put marker values at the words before/after the stack

  //P7OUT &= ~(BIT7);
  //P8OUT &= ~(BIT6);

  adcsetup();//adc setup 
  ctl_events_init(&handle_SDcard, 0);
  ctl_events_init(&handle_get_I2C_data, 0);//is this supposed to be for the I2C initalization//code is the same on github
  //I changed it to and I2C command 
  //timersetup();//start timer //using jesse's timer in main

  /*while(!async_isOpen()){
  ctl_timeout_wait(ctl_get_current_time()+1000);
  }*/

  // __enable_interrupt();// THIS IS A PREDEFINED GLOBAL INTERRUPT ENABLE BIT IN THE MSP LIBRARY 
  printf("\rLEDL Sensor read test. Press s to stop. \r\n>");// 

  //printf("\rrow\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA6\tA7\r\n>");//gives columns titles 


  //while(async_CheckKey()!='s')//this statement for use with tera term 
  //for(;;)
 


  while(1)//stay in an endless loop while taking data
  //order of opperations -> wait for event to be set ->get buffer, turn on sensors and SD card -> take data -> turn off sensors and SD card
    {
        //put piezo wait event here 
        int mmcReturnValue;
        int count_error; 
        int check_for_free_address;
        long temperory_SDaddr;
        int checking_launch;
        LEDL_TEST_LAUNCH *launch_detect_data;//call variables by calling launch_detect_data.dat.detect_dat.SDaddress
        //make the structer launch_detect_data point to the buffer
         launch_detect_data=(LEDL_TEST_LAUNCH*)buffer;
/*
         //this gets started after I begin taking data and stops when I am finished taking data, this is done here to prevent the SD card lines from turning on and off from main 
          //check to see if launch has happened previously
          //first i have to turn on the regulator followed by everything else to make sd card work
           //turn up the voltage
          VREGon();
          //need to have a setteling time for the voltage regulator
          ctl_timeout_wait(ctl_get_current_time()+100);//4.88 MS IS 5
          //then initalize the 16 Mhz clock 
          initCLK();

          //get buffer to use sd card

          buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
          if (buffer==NULL){
          printf("unable to use bus buffer \r\n");
          return;
          }//closes if(buffer==NULL) statement
         
          //turn on sensors here 
          //have to turn on the SD card to check this out first
         
          ctl_timeout_wait(ctl_get_current_time()+100);//4.88 MS IS 5
          SENSORSon();//turn on sensor
          ctl_timeout_wait(ctl_get_current_time()+200);//4.88 MS IS 
          mmc_pins_on();
          mmcReturnValue=mmcInit_card();
          if (mmcReturnValue==MMC_SUCCESS){
            printf("\rCard initalized Sucessfully\r\n");
            }
          else {
          printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
               } 
         
          //read data from SD card to determine mode
          
          //give mode status to variable to look at now and later 
          
        
           // startup_mode=MODE_LAUNCH;//added this line for sensor testing 
            //if mode is launch mode, than find free data space 
            //if(startup_mode==MODE_LAUNCH)
           
           //I want to comment everything out from above this besides finding a new location from the sd card to store data below this is all the storage data infomation 
           //comment this out due to having this a function call 
           //WAIT FOR EVENT TO START 
           BUS_free_buffer();
           SENSORSoff();
           initCLK_lv();//Reduce clock speed for low voltage application
           VREGoff();//turn Voltage regulator off for low power application
   */        

           WDT_STOP();
           BUS_lp_mode=ML_LPM0;
           
           //Code that checks the launch detect
           checking_for_launch=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_LaunchData,LAUNCHDATA_FLAG,CTL_TIMEOUT_NONE,0);
           while(checking_for_launch&LAUNCHDATA_FLAG)
           {
             unsigned e;
             //start sd card task 
             ctl_task_run(&SD_card,BUS_PRI_EXTRA_HIGH,(void(*)(void*))writedatatoSDcard,NULL,"writedatatoSDcard",sizeof(stack4)/sizeof(stack4[0])-2,stack4+1,0);//set up task to run SD card, 
             
            //begin to log data from launch 
            
                      
              //turn up the voltage
              VREGon();
              //need to have a setteling time for the voltage regulator
              ctl_timeout_wait(ctl_get_current_time()+100);//4.88 MS IS 5
              //then initalize the 16 Mhz clock 
              
              LED_LAUNCH_DATA_ON();
              buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);

              if (buffer==NULL){
              printf("unable to use bus buffer \r\n");
              return;
              }//closes if(buffer==NULL) statement 
      
              launch_data1=(int*)buffer;
              launch_data2=(int*)(buffer+512);
              launch_data=(int*)launch_data1;
              launch_detect_data=(LEDL_TEST_LAUNCH*)buffer;
              //turn on sensors here 
              ACCon();//turn on acclerometers 
              ctl_timeout_wait(ctl_get_current_time()+300);//4.88 MS IS 5
              GyroWakeUp();
              SENSORSon();
              //initalize the SD card   
              mmc_pins_on();
              mmcReturnValue=mmcInit_card();
              if (mmcReturnValue==MMC_SUCCESS){
              printf("\rCard initalized Sucessfully\r\n");
              LED_6_ON();
              }
              else {
              printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
                   }
              //check to see what last stored data location is on SD card
              result=mmcReadBlock(SD_BECON_DATA,(unsigned char*)buffer);
              //store data into SDaddr
              SDaddr=launch_detect_data->dat.detect_dat.SD_last_address;
              //check to see that the value saved is greater than 100, if it is zero than there is no data saved, if it is less than 100 than it needs to start at 100. 
              if(SDaddr<1000){SDaddr=SD_LAUNCH_DATA_START;}
              //even though we may have the last address used, just in case the code will check that is is empty. 
              check_for_free_address=1;
              //SDaddr=launch_detect_data->dat.detect_dat.SD_last_address;
               LED_1_ON();
               //ctl_timeout_wait(ctl_get_current_time()+500);//4.88 MS IS 5

               initCLK();
               while(check_for_free_address)
                {
                     result=mmcReadBlock(SDaddr,(unsigned char*)buffer);
                     //check to see if current sd card address is equal to the input LEDL_DATA_ID
                     //NOTE this only checks to see if address is 0, does not match software chapter 
                     if(result<0){count_error++;
                       if(count_error==3)
                       {
                          SENSORSon();
                          //initalize the SD card   
                          mmc_pins_on();
                          mmcReturnValue=mmcInit_card();
                          if (mmcReturnValue==MMC_SUCCESS){
                          printf("\rCard initalized Sucessfully\r\n");
                          LED_6_ON();
                          count_error=0;
                          }

                       }
                     }
                      if(launch_detect_data->ledl_address!=LEDL_DATA_ID)
                      {
                      //this value starts at either 100 or at the first empty spot found in increments of 1000 
                      SDaddr=SDaddr+1000;
                      //ctl_timeout_wait(ctl_get_current_time()+500);//4.88 MS IS 5
                      }
                      else
                      {
                      //NOTE:this checking_launch may not be used, is this supposed to be checking_for_launch?
                      int checking_launch=1;
                      check_for_free_address=0;
                      LED_1_OFF();
                      //have data start, by acting like interrput has first happened
                      //only use this ctl event if Launch Detect is used as a ctl in main 
                      //ctl_events_set_clear(&handle_LaunchDetect,1<<0,0);//this is for use during analysis of if there is a current launch happening                     
                      //switch_is_on=1; 
                        
                      }
                    
                }
       



            while(switch_is_on)//this statement is for use with external switch or can be left during actual launch and we will have to make sure it is include for P2.7 interrupt for piezo tab
            {
           
              //start adc event 
              e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_adc,(1<<0),CTL_TIMEOUT_NONE,0);
                if(e&(0x01))
                {
                  //if (handle_adc)//in adc interrupt handle_adc is set high, so this code become initialized
                  //handle_adc = 0;//reset my personal flag to zero //I do not use this with events
                      //LAUNCH_LED_ON(); //LIGHT UP LED WHEN TAKING DATA 
                      extern adctemparray1[6]; 
                      
                      if (data==0)
                      {
                      launch_data[data]=LEDL_DATA_ID;
                      data++;
                      temperory_SDaddr=SDaddr;
                      launch_data[data]=temperory_SDaddr;
                      data++;
                      launch_data[data]=temperory_SDaddr>>16;
                      data++;

                      }
                      if (accel_count==0)// only when accel_count=0 will the row number be displayed
                      {
                      //printf("%d\t", row);//prints row number
                      launch_data[data]=frame;
                      data++;
                      launch_data[data]=row;
                      data++;
                      row++;//add count to rows this is also SFID
                      } //closes if(accel_count==0)
              
                        for (i=0; i<6; ++i)// print first 6 adc measurements  
                         {
                          //printf("%d\t", arr[i]);
                          launch_data[data]=adctemparray1[i];
                          data++;
                          accel_count++;//add a count to accel_count,
                         }  //closes for (i=0; i<6; ++i)// print first 6 adc measurements
              
                          if (accel_count==24)// when accel_count is 24 it print last two columns
                          {
                          switch (gyro_count)
                              {
                              case 0:
                                     //NOTE GYRO IS NOW A I2C DEVICE 
                                     //printf("%d(G1)\t", arr[6]);
                                     //launch_data[data]=adctemparray1[6];
                                     data++;
                                     gyro_count++;
                                     break;
                              case 1:
                                     //printf("%d(G2)\t", arr[7]);
                                     //launch_data[data]=arr[7];
                                     data++;
                                     gyro_count++;
                                     GyroMuxon();//selecting G3 on mux for next pass
                                     //GyroSelfTeston();//turning on ST to see if there is a change
                                     break;
                              case 2:
                                     //printf("%d(G3)\t", arr[7]);
                                     //launch_data[data]=arr[7];
                                     data++;
                                     gyro_count=0;
                                     GyroMuxoff();;//selecting G2 on mux for next pass
                                    // GyroSelfTestoff();//turning off the self test to see if there is a change 
                                     break;
                              }//closes switch (gyro_count)
   
           
                                switch (temp_display)
                                    {


                                    case 0: 
                                      if (frame_100==100)
                                      {
                                     // Temp_I2C_sensor(temp_measure);//here is the first temperature measurement passing an array 7 length long //at some point I will need to measure temperature sensors as a 
                                      //different task so that as I am waiting for the the return values I can be taking my other measurements. 
                                      //printf("%d\t",temp_measure[0]);//this is the spot for temperature data
                                      launch_data[data]=temp_measure[0];
                                      data++;
                                      }//closes if (frame_100==100)
                                      else if (frame_100==0)
                                      {
                                      ctl_events_set_clear(&handle_get_I2C_data,LEDL_EV_GET_TEMP_DATA,0);  
                                      data++;
                                      }//closes else if (frame_100==0)
                                      else
                                      {
                                      //printf("0000\t");
                                      launch_data[data]=0000;
                                      data++;
                                      }//closes else
                                      temp_display++;
                                      break;
                                    case 1:
                                      if (frame_100==100)
                                      {
                                      //printf("%d\t",temp_measure[1]);//this is the spot for temperature data
                                      launch_data[data]=temp_measure[1];
                                      data++;
                                      }// closes if (frame_100==100)
                                      else
                                      {
                                      //printf("0000\t");
                                      launch_data[data]=0000;
                                      data++;
                                      }//closes else 
                                      temp_display++;
                                      break;
                                    case 2:
                                      if (frame_100==100)
                                      {
                                      //printf("%d\t",temp_measure[2]);//this is the spot for temperature data
                                      launch_data[data]=temp_measure[2];
                                      data++;
                                      }//closes if (frame_100==100)
                                      else
                                      {
                                      //printf("0000\t");
                                      launch_data[data]=0000;
                                      data++;
                                      }//closes else 
                                      temp_display++;
                                      break;
                                    case 3:
                                      if (frame_100==100)
                                      {
                                      //printf("%d\t",temp_measure[3]);//this is the spot for temperature data
                                      launch_data[data]=temp_measure[3];
                                      data++;
                                      }//closes if (frame_100==100)
                                      else
                                      {
                                      //printf("0000\t");
                                      launch_data[data]=0000;
                                      data++;
                                      }//closes else
                                      temp_display++;
                                      break;
                                    case 4:
                                      if (frame_100==100)
                                      {
                                      //printf("%d\t",temp_measure[4]);//this is the spot for temperature data
                                      launch_data[data]=temp_measure[4];
                                      data++;
                                      }//closes if (frame_100==100)
                                      else
                                      {
                                      //printf("0000\t");
                                      launch_data[data]=0000;
                                      data++;
                                      }//closes else
                                      temp_display++;
                                      break;
                                    case 5:
                                      if (frame_100==100)
                                      {
                                      //printf("%d\t",temp_measure[5]);//this is the spot for temperature data
                                      launch_data[data]=temp_measure[5];
                                      data++;
                                      }//closes if (frame_100==100)
                                      else
                                      {
                                      //printf("0000\t");
                                      launch_data[data]=0000;
                                      data++;
                                      }//closes else
                                      temp_display++;
                                      break;
                                    case 6:
                                      if (frame_100==100)
                                      {
                                     //printf("%d\t",temp_measure[6]);//this is the spot for temperature data
                                     launch_data[data]=temp_measure[6];
                                      data++;
                                      frame_100=0;
                                      }// closes if (frame_100==100)
                                      else
                                      {
                                      //printf("0000\t");
                                      launch_data[data]=0000;
                                      data++;
                                      frame_100++;
                                      }//closes else 
                                      temp_display++;
                                      break;
                                    case 7:
                                      ticker_time=get_ticker_time();                 
                                      time2=ticker_time;
                                      time1=ticker_time>>16;

                                      //printf("%u\t",time1);//this is the spot for time1 data
                                      launch_data[data]=time1;
                                      data++;
                                      temp_display++;
                                      break;
                                    case 8: 
                                      //printf("%u\t",time2);
                                      launch_data[data]=time2;
                                      data++;
                                      //store vib tab data to data matrix to first gyro data point 
                                      launch_data[29]=vib_tab_count;
                                      
                                      //store crc into data array 
                                      crc_check=crc16(launch_data,510);
                                      launch_data[255]=crc_check;//put crc in last spot of data
                                      frame++;
                                      row=0;
                                      data=0;
                                      if (launch_data==launch_data1)
                                      {
                                      ctl_events_set_clear(&handle_SDcard,SD_EV_WRITE_1,0);
                                      //result = mmcWriteBlock(SDaddr,(unsigned char*) launch_data1); //(unsigned char*) casting my pointer(array) as a char 
                                      //SD_LED();
                                      launch_data=launch_data2;
                                      }//closes if(launch_data==launch_data1)
                                      else 
                                      {
                                      
                                       ctl_events_set_clear(&handle_SDcard,SD_EV_WRITE_2,0); 
                                       //result= mmcWriteBlock(SDaddr,(unsigned char*) launch_data2); //(unsigned char*) casting my pointer(array) as a char 
                                       //SD_LED(); 
                                       launch_data=launch_data1;
                                      }//closes else                         
                                      temp_display=0;
                                     break; 
                                      //add a count for incrementing frame number
                                    }//closes switch (temp_display) 
                        
                                //printf("\r\n");
                                //rest accel_count to 0 so the loop can start all over again
                                accel_count=0;
                                //reset the vib_tab_count to zero so that we can count in the next block 
                                vib_tab_count=0;
                           }//closes  if (accel_count==24)
                         }//closes  if(e&(0x01)) adc if loop 
         
                     }//closes while(switch_is_on==1) 
                   // printf("%d/t",launch_data);
                  //turn off sensors here if this is where the empty while loop 
               //LAUNCH_LED_OFF(); //Turn off LED that shows data is no longer taking data 
               //before everything gets turned off, store the number of sd card sectors used and the last address stored 
               launch_detect_data->ledl_address=LEDL_DETECT_ID;
               launch_detect_data->dat.detect_dat.SDaddress=SDaddr;
               launch_detect_data->dat.detect_dat.SD_last_address=SDaddr;
               launch_detect_data->dat.detect_dat.accel_from_launch[0]=0;
               launch_detect_data->dat.detect_dat.accel_from_launch[1]=0;
               launch_detect_data->dat.detect_dat.accel_from_launch[2]=0;
               launch_detect_data->dat.detect_dat.accel_from_launch[3]=0;
               launch_detect_data->dat.detect_dat.max_and_min_from_launch[0]=0;
               launch_detect_data->dat.detect_dat.max_and_min_from_launch[1]=0;
               launch_detect_data->dat.detect_dat.max_and_min_from_launch[2]=0;
               launch_detect_data->dat.detect_dat.max_and_min_from_launch[3]=0;
               launch_detect_data->dat.detect_dat.max_and_min_from_launch[4]=0;
               launch_detect_data->dat.detect_dat.max_and_min_from_launch[5]=0;
               launch_detect_data->dat.detect_dat.number_of_detect=wake_up_attempt;
               launch_detect_data->dat.detect_dat.mode_status=MODE_ORBIT;
               crc_check=crc16(launch_data,510);                          
               launch_detect_data->crc=crc_check;//put crc in last spot of data
               result = mmcWriteBlock(SD_BECON_DATA ,(unsigned char*) &launch_detect_data); //(unsigned char*) casting my pointer(array) as a char 
               frame=0;
               LED_LAUNCH_DATA_OFF();
               ACCoff();
               GyroSleep();
               ctl_events_set_clear(&handle_SDcard,SD_EV_DIE,0);//this sends the flag to allow for the SD card to shut down 
               ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_SDcard,SD_EV_FINISHED,CTL_TIMEOUT_NONE,0);//this waits for SD card to finish writing the last block so that it can shutdown 
               mmc_pins_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWR LINE 
               LED_6_OFF();
               LED_5_OFF();
               SENSORSoff();
               BUS_free_buffer();
               initCLK_lv();//Reduce clock speed for low voltage application
               VREGoff();//turn Voltage regulator off for low power application
               WDT_STOP();
               //printf("TEST POINT");//this is the spot for temperature data

         
            }//closes launchlog  
    }//closes  while(1)
         
}//closes function  launch_data_log(void *p)




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

//data for EPS commands sent over I2C
unsigned char remote_EPS_cmd[2];

//create an event to take I2C data 
void takeI2Cdata(void){
  int k=0;
  int l=0;
  int resp;
  unsigned get_I2C_data;
  unsigned char *buffer;
  for(;;){ 
    get_I2C_data=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_get_I2C_data,LEDL_EV_ALL,CTL_TIMEOUT_NONE,0);
    if (get_I2C_data&(LEDL_EV_GET_TEMP_DATA)){
      //take temperature data
      Temp_I2C_sensor(temp_measure);
    }
    if(get_I2C_data&(LEDL_EV_EPS_CMD)){
      //lock mutex
      if(ctl_mutex_lock(&EPS_mutex,CTL_TIMEOUT_DELAY,1000)){
        //send command to EPS
        i2c_tx(EPS_I2C_ADDRESS,remote_EPS_cmd,2);
        //unlock mutex
        ctl_mutex_unlock(&EPS_mutex);
      }else{
        //TODO: report error
        //trigger event again            
        ctl_events_set_clear(&handle_get_I2C_data,LEDL_EV_EPS_CMD,0);
        //TODO : timeout?
      }
    }
    if(get_I2C_data&(LEDL_EV_SEND_DAT)){
      buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,500);
      if(buffer!=NULL){
        //set block 
        buffer[0]=SPI_LEDL_DAT;
        buffer[1]=BUS_ADDR_LEDL;
        //read block into buffer
        resp=mmcReadBlock(SD_read_addr,buffer+2);
        //check response
        if(resp==RET_SUCCESS){
          //send data to COMM
          resp=BUS_SPI_txrx(BUS_ADDR_COMM,buffer,NULL,512 + 2);
          //check result
          if(resp!=RET_SUCCESS){
            //TODO: handle error
          }
        }else{
          //TODO: handle error
        }
        //done with buffer, free it
        BUS_free_buffer();
      }
    }
    if(get_I2C_data&(LEDL_EV_BLOW_FUSE)){
      LEDL_BLOW_FUSE();
    }
    if(get_I2C_data&(LEDL_EV_STATUS_TIMEOUT)){
      reset(ERR_LEV_INFO,ERR_SRC_LEDL,STAT_TIMEOUT,0);
    }
  }
}


//create an event to take clyde data 
void takeclydedata(void)
{
unsigned get_clyde_data;
for(;;){//take eps data 
        get_clyde_data=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_get_clyde_data,CLYDE_EV_GET_DATA,CTL_TIMEOUT_NONE,0);
        if (get_clyde_data&(CLYDE_EV_GET_DATA))
        {
        printf("made it to clyde command");
        clyde_take_data(clyde_data);
        }
        }
       }




////////////orbit data////////////////////////////////
//in orbit LEDL takes magnetometer data, temperature data, gyro data, and clyde data. All data needs to be put into a packet for CDH
// The LEDL data packet looks like this 
//name size definiation 
// ledl_total_sd_data 32 bits total data in SD card   temp_measure[0]
// ledl_temp_x_plus 8 bits temperature x+ face        temp_measure[1]
// ledl_temp_x_minus 8 bits temperuate x- face        temp_measure[2]
// ledl_temp_y_plus 8 bits temperature y+ face        temp_measure[3]
// ledl_temp_y_minus 8 bits temperature y- face       temp_measure[4]
// ledl_temp_z_plus 8 bits temperuate Z+ face         temp_measure[5]
// ledl_temp_z_minus 8 bits temperature z- face       temp_measure[6]
// ledl_temp_board 8 bits temperature ledl board      temp_measure[7]
// eps_stat_ttc 8 bits eps ttc status                   clyde_data[15]
// eps_batt_1_temp 16 bits battery 1 temperature        clyde_data[16]
// eps_batt_0_temp 16 bots battery 0 temperature        clyde_data[17]
// eps_batt_0_current_dir 8 bits battery 0 current direcition hig if bat charge, low if battery discharge clyde_data[18]
// eps_batt_1_current_dir 8 bits battery 1 current direcition hig if bat charge, low if battery discharge clyde_data[19]

//the cldye data packet looks like this 
//name size 
//eps_yplus_current 16 bits y+current     clyde_data[0]
//eps_yminus_current 16 bits              clyde_data[1]
//eps_y_voltage 16 bits                   clyde_data[2]
//eps_xplus_current 16 bits               clyde_data[3]
//eps_xminus_current 16 bits              clyde_data[4]
//eps_x_voltage 16 bits                   clyde_data[5]
//eps_zminus_current 16 bits              clyde_data[6]
//eps_z_voltage 16 bits                   clyde_data[7]
//eps_battbus_current 16 bits             clyde_data[8]
//eps_5bus_current 16 bits                clyde_data[9]
//eps_3_3bus_current 16 bits              clyde_data[10]
//eps_batt0_current 16 bits               clyde_data[11]
//eps_batt1_current 16 bits               clyde_data[12]
//eps_batt0_voltage 16 bits               clyde_data[13]
//eps_batt1_volatage 16 bits              clyde_data[14]

//when taking the eps data the clyde data  packets will be sampled first, the remainder of clyde data from ledl packet will be sampled at end

/*
void orbit_data_log(void *p){
//need to, temperate sensors, gyros, clyde, take magnetometer data at request of ACDS, and store important information into sd card
//can resuse temperate and gyro logging data, use the same SD card code, // set code to sample 0-15 samples a second for magnetometer depending on ACDS request
//ADC timer has to be changed. 




//void launch_data_log(void){// used to set up code // USE THIS ONE FOR JUST USING IT AS A FUNCITON 
extern int orbit_switch_is_on;
int i; //counter to cycle through six adc measurements 
unsigned event1;
     
    
    memset(stack4,0xcd,sizeof(stack4));  // write known values into the stack
    stack4[0]=stack4[sizeof(stack4)/sizeof(stack4[0])-1]=0xfeed; // put marker values at the words before/after the stack
    memset(stack5,0xcd,sizeof(stack5));  // write known values into the stack
    stack5[0]=stack5[sizeof(stack5)/sizeof(stack5[0])-1]=0xfeed; // put marker values at the words before/after the stack
    memset(stack6,0xcd,sizeof(stack6));  // write known values into the stack
    stack6[0]=stack6[sizeof(stack6)/sizeof(stack6[0])-1]=0xfeed; // put marker values at the words before/after the stack
  
      //P7OUT &= ~(BIT7);
      //P8OUT &= ~(BIT6);

      adcsetup();//orbit uses the same adc, the timer needs to be dependant on the mode that LEDL is in though, saved in flash
      ctl_events_init(&handle_SDcard, 0);
      ctl_events_init(&handle_get_I2C_data, 0);//is this supposed to be for the I2C initalization//code is the same on github
      ctl_events_init(&handle_get_mag_data,0);//this is for mag initializtion 
      ctl_events_init(&handle_get_clyde_data,0);//this is for clyde initialization 
      
      //I changed it to and I2C command 
      //timersetup();//start timer //using jesse's timer in main
  
      /*while(!async_isOpen()){
        ctl_timeout_wait(ctl_get_current_time()+1000);
      }*/
     
      // __enable_interrupt();// THIS IS A PREDEFINED GLOBAL INTERRUPT ENABLE BIT IN THE MSP LIBRARY 
     // printf("\rLEDL orbit read test\r\n>");// 
      
      //printf("\rrow\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA6\tA7\r\n>");//gives columns titles 
  
  
      //while(async_CheckKey()!='s')//this statement for use with tera term 
      //for(;;)

/*
  while(1)//stay in an endless loop while taking data
  {
   event1=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_OrbitData,OrbitData_flag,CTL_TIMEOUT_NONE,0);
    if(event1&(OrbitData_flag))
    {
      int mmcReturnValue;
      int gyro_count=0;//counter to display gyro
      int signal_mux=0;// making signal that will cause mux to switch between gyro 2 (G2) when signal_mux=0 and gyro 3 (G3) when signal_mux=1 
      int data=0;
     //turn up the voltage
      VREGon();
      
      //need to have a setteling time for the voltage regulator
      //then initalize the 16 Mhz clock 
      ctl_timeout_wait(ctl_get_current_time()+5);//4.88 MS
      SENSORSon();//turn on sensor
      //turn on sensors here 
      MAGon();//turn on magnetometers 
      GyroWakeUp();
      
       //initalize the SD card   
       ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 
      // mmcInit_msp();
       //mmcReturnValue=mmcInit_card();
      // if (mmcReturnValue==MMC_SUCCESS){
      // printf("\rCard initalized Sucessfully\r\n");
      // }
      // else {
      // printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue)); 
      // }
      // ctl_task_run(&SD_card,3,writedatatoSDcard,NULL,"writedatatoSDcard",sizeof(stack4)/sizeof(stack4[0])-2,stack4+1,0);//set up task to run SD card,
     ctl_task_run(&mag,15,ACDS_sensor_interface,NULL,"ACDS_sensor",sizeof(stack5)/sizeof(stack5[0])-2,stack5+1,0);
     ctl_task_run(&clyde,2,takeclydedata,NULL,"takeclydedata",sizeof(stack6)/sizeof(stack6[0])-2,stack6+1,0);//set up task to run SD card,
       //this gets started after I begin taking data and stops when I am finished taking data, this is done here to prevent the SD card lines from turning on and off from main 
       initCLK();
       ctl_events_set_clear(&handle_OrbitData,0, OrbitData_flag);
    while(orbit_switch_is_on)//this statement is for use with external switch
    {
      unsigned e;
      e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_adc,(1<<0),CTL_TIMEOUT_NONE,0);
        if(e&(0x01))
        {
          volatile unsigned int *arr = &ADC12MEM0;// creates a pointer, looks at the memory of mem0
          //if (handle_adc)//in adc interrupt handle_adc is set high, so this code become initialized
          //handle_adc = 0;//reset my personal flag to zero //I do not use this with events
              P4OUT |= BIT2; //LIGHT UP LED WHEN TAKING DATA 
                  //first take gyro data 

                         //take temperature data 
                              ctl_events_set_clear(&handle_get_I2C_data,get_I2C_data_flag,0); 
                              //take clyde data
                              ctl_events_set_clear(&handle_get_clyde_data,get_clyde_data_flag,0);
                             
                             switch (gyro_count)
                             {
                               case 0:
                               {
                                 //printf("%d(G1)\t", arr[6]);
                                 ledl_packet_to_acds.gyro_data[data]=arr[6];
                                 gyro_count++;
                                 data++;
                                 //printf("%d(G2)\t", arr[7]);
                                 ledl_packet_to_acds.gyro_data[data]=arr[7];
                                 data++;
                                 GyroMuxon();//selecting G3 on mux for next pass
                                 //GyroSelfTeston();//turning on ST to see if there is a change
                                 break;
                               }
                               case 1:
                                 //printf("%d(G3)\t", arr[7]);
                                 ledl_packet_to_acds.gyro_data[data]=arr[7];
                                 data=0;
                                 gyro_count=0;
                                 GyroMuxoff();;//selecting G2 on mux for next pass
                                // GyroSelfTestoff();//turning off the self test to see if there is a change 

                                 //organize data packets for each system 
                                 //temperature data has to be converted to 8 bit value 
                                 for (i=0;i<7;i++)
                                 {//convert temperature to 8 bit value, do we want to lose the quarter degree resolution 
                                 }
                                 // ledl data to cdh is tempdata and some of clyde data
                                 ledl_packet_to_cdh.SDaddress=SDaddr; 
                                 //if we get ride of the quarter degree resolution than the structure has to be changed to support 8 bit value, or be clever. 2 data in one slot... 
                                 for(i=0;i<7;i++){
                                 ledl_packet_to_cdh.temp_data[i]=temp_measure[i];
                                 }
                                 for(i=0;i<5;i++)
                                 {
                                 ledl_packet_to_cdh.clyde_array_five[i]=clyde_data[i+15];
                                 }
                                 for(i=0;i<15;i++)
                                 {
                                 clyde_packet_to_cdh.clyde_array_fifteen[i]=clyde_data[i];
                                 }    
                               break;
                             }//closes switch 
                 }//closes  if(e&(0x01))
         
             }//closes while(switch_is_on==1) 
       //turn off sensors here if this is where the empty while loop 
       P4OUT &= ~(BIT2|BIT4); //Turn off LED that shows data is no longer taking data 
       GyroSleep();
       MAGoff();
       mmcInit_msp_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWR LINE 
       SENSORSoff();
       initCLK_lv();//Reduce clock speed for low voltage application
       VREGoff();//turn Voltage regulator off for low power application
       WDT_STOP();
       //printf("TEST POINT");//this is the spot for temperature data

       }//closes  if(event1&(LAUNCHDATA_FLAG))
     }//closes  while()
   }//closes function  Orbit_data_log(void *p)



//code from original

/*

void launch_data_log(void *p){// used to set up code USE THIS ONE WITH THE TASK LIBRARY 
//void launch_data_log(void){// used to set up code // USE THIS ONE FOR JUST USING IT AS A FUNCITON 
  extern int switch_is_on;
  unsigned long frame = 0;
  unsigned long frameforbitshift = 0;
  unsigned long row=0;//counter for rows also called SFID
  ticker ticker_time;//ticker time gained from the ARC bus clock ticker is a specified timer specefier for this 
  unsigned short time1;
  unsigned short time2;
  unsigned char *buffer;
  int accel_count=0;//counter to display last two rows of data
  int gyro_count=0;//counter to display gyro
  int frame_100=0;//counter to display temperature 
  int temp_display=0;//counter to display zeros or temp
  int i; //counter to cycle through six adc measurements 
  int data=0;//counter to cycle though all data
  int signal_mux=0;// making signal that will cause mux to switch between gyro 2 (G2) when signal_mux=0 and gyro 3 (G3) when signal_mux=1 
  unsigned event1;
  int *launch_data;
  int crc_check;
  //SDaddr+=1; //address already starts at 65

  memset(stack4,0xcd,sizeof(stack4));  // write known values into the stack
  stack4[0]=stack4[sizeof(stack4)/sizeof(stack4[0])-1]=0xfeed; // put marker values at the words before/after the stack

  //P7OUT &= ~(BIT7);
  //P8OUT &= ~(BIT6);

  adcsetup();//adc setup 
  ctl_events_init(&handle_SDcard, 0);
  ctl_events_init(&handle_get_I2C_data, 0);//is this supposed to be for the I2C initalization//code is the same on github
  //I changed it to and I2C command 
  //timersetup();//start timer //using jesse's timer in main

  //while(!async_isOpen()){ctl_timeout_wait(ctl_get_current_time()+1000);}

  // __enable_interrupt();// THIS IS A PREDEFINED GLOBAL INTERRUPT ENABLE BIT IN THE MSP LIBRARY 
  printf("\rLEDL Sensor read test. Press s to stop. \r\n>");// 

  //printf("\rrow\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA6\tA7\r\n>");//gives columns titles 


  //while(async_CheckKey()!='s')//this statement for use with tera term 
  //for(;;)
 


  while(1)//stay in an endless loop while taking data
  //order of opperations -> wait for event to be set ->get buffer, turn on sensors and SD card -> take data -> turn off sensors and SD card
    {
        //put piezo wait event here 
        int mmcReturnValue;
        unsigned checking_for_launch;
        int check_for_free_address;
       
        int checking_launch;
        LEDL_TEST_LAUNCH *launch_detect_data;//call variables by calling launch_detect_data.dat.detect_dat.SDaddress
         //this gets started after I begin taking data and stops when I am finished taking data, this is done here to prevent the SD card lines from turning on and off from main 
          //check to see if launch has happened previously
          //first i have to turn on the regulator followed by everything else to make sd card work
           //turn up the voltage
          VREGon();
          //need to have a setteling time for the voltage regulator
          //then initalize the 16 Mhz clock 
          ctl_timeout_wait(ctl_get_current_time()+100);//4.88 MS IS 5
          //get buffer to use sd card
          buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
          if (buffer==NULL){
          printf("unable to use bus buffer \r\n");
          return;
          }//closes if(buffer==NULL) statement
          //make the structer launch_detect_data point to the buffer
          launch_detect_data=(LEDL_TEST_LAUNCH*)buffer;
          //turn on sensors here 
          //have to turn on the SD card to check this out first

          SENSORSon();//turn on sensor
          mmc_pins_on();
          mmcReturnValue=mmcInit_card();
          if (mmcReturnValue==MMC_SUCCESS){
            printf("\rCard initalized Sucessfully\r\n");
            result=mmcReadBlock(SD_BECON_DATA ,(unsigned char*)buffer);
            crc_check=crc16(launch_detect_data,510);
            if(crc_check==launch_detect_data->crc)
            {startup_mode=launch_detect_data->dat.detect_dat.mode_status;}
            }
          else {
          printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
      
               } 
           
          //read data from SD card to determine mode
          
          //give mode status to variable to look at now and later 
          
        
            startup_mode=MODE_LAUNCH;//added this line for sensor testing 
            //if mode is launch mode, than find free data space 
            if(startup_mode==MODE_LAUNCH)
            {//search for first spot
              check_for_free_address=1;
              SDaddr=launch_detect_data->dat.detect_dat.SD_last_address;
               LED_1_ON();
               //ctl_timeout_wait(ctl_get_current_time()+500);//4.88 MS IS 5
               while(check_for_free_address)
                {
                   result=mmcReadBlock(SDaddr,(unsigned char*)buffer);
                     //check to see if current sd card address is equal to the input LEDL_DATA_ID
                      if((launch_detect_data->ledl_address==LEDL_DATA_ID)||(launch_detect_data->ledl_address==LEDL_DETECT_ID))
                      {
                      //this value starts at 65 and begins looking for a free spot in memory 
                      SDaddr++;
                      //ctl_timeout_wait(ctl_get_current_time()+500);//4.88 MS IS 5
                      }
                      else
                      {
                      int checking_launch=1;
                      check_for_free_address=0;
                      LED_1_OFF();
                      //have data start, by acting like interrput has first happened
                      ctl_events_set_clear(&handle_LaunchDetect,1<<0,0);//this is for use during analysis of if there is a current launch happening                     
                      switch_is_on=1; 
                        
                      }
                    
                }
            }
            else
            {//we turned on sd card to check for launch, now it needs to be turned back off since the system determined there was not a launch
                if(P1IN&BIT0)
                {
                //in obrit mode do not turn off sd card stuff 
                //TURN REGULATOR OFF 
                VREGoff();
                BUS_free_buffer();
                }
                 else{
                 mmc_pins_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWER LINE 
                 SENSORSoff();
                 BUS_free_buffer();
                 initCLK_lv();//Reduce clock speed for low voltage application
                 VREGoff();//turn Voltage regulator off for low power application
                 WDT_STOP();
                }
             
            }
         //waiting for event, return to low power mode  
         BUS_lp_mode=ML_LPM4;
        
        //Code that checks the launch detect
        checking_for_launch=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_LaunchDetect,(1<<0),CTL_TIMEOUT_NONE,0);
        if(checking_for_launch&(0x01))
        //if(checking_for_launch&LAUNCH_DETECT_FLAG)
        {
          int mmcReturnValue;
          unsigned e;
          float checkacc[6],xavg,yavg,zavg,maxx,maxy,maxz,minx,miny,minz;
          float totalacc;
          unsigned int data,accel_count_for_launch_test;
          int i=0;
          long temperory_SDaddr;
          float acc18gconversionx = 0.0625; // volts per g
          float acc18gconversiony = 0.0633;
          float acc18gconversionz = 0.0633;
          float adcconversion = 0.0008057; // volts per adc reference 0 to 4096 for 3.3 V source 
          float go_launch_value=0.0156;//this value is when a launch occures 0.0855^2
          LEDL_TEST_LAUNCH *launch_detect_data;//call variables by calling launch_detect_data.dat.detect_dat.SDaddress
          BUS_lp_mode=ML_LPM0;
          wake_up_attempt++;//increase the wake up attempt
         //turn up the voltage
          VREGon();
      
          //need to have a setteling time for the voltage regulator
          //then initalize the 16 Mhz clock 
          ctl_timeout_wait(ctl_get_current_time()+100);//4.88 MS IS 5
        
          buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);

          if (buffer==NULL){
          printf("unable to use bus buffer \r\n");
          return;
          }//closes if(buffer==NULL) statement 
      
          launch_data1=(int*)buffer;
          launch_data2=(int*)(buffer+512);
          launch_data=(int*)launch_data1;
          launch_detect_data=(LEDL_TEST_LAUNCH*)buffer;
          //turn on sensors here 
          ACCon();//turn on acclerometers 
          ctl_timeout_wait(ctl_get_current_time()+300);//4.88 MS IS 5
          GyroWakeUp();
          SENSORSon();
          //initalize the SD card   
          mmc_pins_on();
          mmcReturnValue=mmcInit_card();
          if (mmcReturnValue==MMC_SUCCESS){
          printf("\rCard initalized Sucessfully\r\n");
          }
          else {
          printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
      
          }
                     
            
      
          
          ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 

          initCLK();
          
          ADC12CTL0|=ADC12SC;//adc control bits 
              //check to see if there is launch 
               for(accel_count_for_launch_test=0;accel_count_for_launch_test<4100;++accel_count_for_launch_test)
                    {
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
                                    if(i==3)
                                    {//zaxis 
                                    checkacc[i]-=1.6623;//should be average Z axis 
                                    checkacc[i]=fabsf(checkacc[i]);
                                    }
                                    else if (i==4)
                                    {//yaxis 
                                    checkacc[i]-=1.6735;//should be average Y axis 
                                    checkacc[i]=fabsf(checkacc[i]);
                                    }
                                    else if (i==5)
                                    {//xaxis 
                                    checkacc[i]-=1.6683;//should be average X axis
                                    checkacc[i]=fabsf(checkacc[i]);
                                    }
                                    else
                                    {}//do nothing with other data
                                    
                                  }//closes for loop for taking accelerometer data
                                    if (accel_count_for_launch_test==1)
                                    {
                                    xavg=checkacc[5];//keep a running average on the samples
                                    maxx=checkacc[5];
                                    minx=checkacc[5];
                               //     printf("Xavg = %f",xavg);
                                    yavg=checkacc[4];//keep a running average on the samples
                                    maxy=checkacc[4];
                                    miny=checkacc[4];
                               //     printf("Yavg = %f",yavg);
                                    zavg=checkacc[3];//keep a running average on the samples. 
                                    maxz=checkacc[3];
                                    minz=checkacc[3];
                                //    printf("Zavg = %f",zavg);
                                    }//closes if statment for accel_count_for_test
                                    else
                                    {
                                      xavg=(xavg+checkacc[5])/2;//keep a running average on the samples
                                        if(checkacc[5]>maxx)
                                        {
                                        maxx=checkacc[5];
                                        }
                                        else
                                        {
                                        minx=checkacc[5];
                                        }
                                      // printf("Xavg = %f",xavg);
                                      yavg=(yavg+checkacc[4])/2;//keep a running average on the samples
                                        if(checkacc[4]>maxy)
                                        {
                                        maxy=checkacc[4];
                                        }
                                        else
                                        {
                                        miny=checkacc[4];
                                        }
                                      // printf("Yavg = %f",yavg);
                                      zavg=(zavg+checkacc[3])/2;//keep a running average on the samples.
                                        if(checkacc[3]>maxz)
                                        {
                                        maxz=checkacc[3];
                                        }
                                        else
                                        {
                                        minz=checkacc[3];
                                        }
                                      //  printf("Zavg = %f",zavg);
                                      //kick watchdog
                                      WDT_KICK();
                                    }//closes else for accelcount>1 but still less than 4100
                            }//closes wait for adc to trigger 
                    }//finishes counting to 4100 to take all data
                  
          printf("Xavg = %f",xavg);
          printf("Yavg = %f",yavg);
          printf("Zavg = %f",zavg);
          totalacc=xavg*xavg+yavg*yavg+zavg*zavg;//find the total squared acceleration vector
          // store launch results to SD card
          launch_detect_data->ledl_address=LEDL_DETECT_ID;
          launch_detect_data->dat.detect_dat.SDaddress=SDaddr;
          launch_detect_data->dat.detect_dat.SD_last_address=SDaddr;
          launch_detect_data->dat.detect_dat.accel_from_launch[0]=xavg;
          launch_detect_data->dat.detect_dat.accel_from_launch[1]=yavg;
          launch_detect_data->dat.detect_dat.accel_from_launch[2]=zavg;
          launch_detect_data->dat.detect_dat.accel_from_launch[3]=totalacc;
          launch_detect_data->dat.detect_dat.number_of_detect=wake_up_attempt;
          launch_detect_data->dat.detect_dat.max_and_min_from_launch[0]=maxx;
          launch_detect_data->dat.detect_dat.max_and_min_from_launch[1]=minx;
          launch_detect_data->dat.detect_dat.max_and_min_from_launch[2]=maxy;
          launch_detect_data->dat.detect_dat.max_and_min_from_launch[3]=miny;
          launch_detect_data->dat.detect_dat.max_and_min_from_launch[4]=maxz;
          launch_detect_data->dat.detect_dat.max_and_min_from_launch[5]=minz;
          
              if(startup_mode==MODE_DETECT)
                {
                  if (totalacc>go_launch_value)
                     {
                     //confirmed launch
                     //begin logging data
                      startup_mode=MODE_LAUNCH;
                      launch_detect_data->dat.detect_dat.mode_status=startup_mode;
                      crc_check=crc16(launch_detect_data,510);                          
                      launch_detect_data->crc=crc_check;//put crc in last spot of data
                      data=0;
                      switch_is_on=1; 
                      checking_launch=0;
                      printf("WE HAVE LAUNCH");
                      accel_count_for_launch_test=0;
                      result = mmcWriteBlock(SDaddr, (unsigned char*) launch_detect_data); //(unsigned char*) casting my pointer(array) as a char 
                      launch_detect_data->dat.detect_dat.SDaddress=SD_BECON_DATA;//since we are storing this in location zero, the address should say zero 
                      crc_check=crc16(launch_detect_data,510);                          
                      launch_detect_data->crc=crc_check;//put crc in last spot of data;
                      result = mmcWriteBlock(SD_BECON_DATA ,(unsigned char*) launch_detect_data);//store that launch happened in the first spot
                      SDaddr+=1;//memory card is block address
                      //P4OUT^=BIT5; 
                     }
                  else{
                      //go back to sleep 
                      BUS_lp_mode=ML_LPM4;
                      checking_launch=0;
                      launch_detect_data->dat.detect_dat.mode_status=startup_mode;
                      crc_check=crc16(launch_detect_data,510);                          
                      launch_detect_data->crc=crc_check;//put crc in last spot of data
                      data=0;
                      printf("NO Launch, return to sleep");
                      accel_count_for_launch_test=0;
                      result = mmcWriteBlock(SDaddr,(unsigned char*) launch_detect_data); //(unsigned char*) casting my pointer(array) as a char 
                      SDaddr+=1;//memory card is block address
                      //LED_3_OFF();
                      ACCoff();
                      GyroSleep();
                      mmc_pins_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWR LINE 
                      SENSORSoff();
                      BUS_free_buffer();
                      initCLK_lv();//Reduce clock speed for low voltage application
                      VREGoff();//turn Voltage regulator off for low power application
                      WDT_STOP();
                      continue;
                      }//closes else for determining launch has not happened
                }//closes if checking mode if statment
            else{
                  //if the LEDL resets for some reason, when it comes back on and sees that launch has actually happened than it will restore the last number used by the data
                  //logger and restores it in the first data location for later use in packets, this will again be updated by the 
                      data=0;
                      switch_is_on=1; 
                      checking_launch=0;
                      printf("WE HAVE LAUNCH");
                      accel_count_for_launch_test=0;
                      startup_mode=MODE_LAUNCH;
                      launch_detect_data->dat.detect_dat.mode_status=startup_mode;
                      crc_check=crc16(launch_detect_data,510);                          
                      launch_detect_data->crc=crc_check;//put crc in last spot of data
                      result = mmcWriteBlock(SDaddr, (unsigned char*) launch_detect_data); //(unsigned char*) casting my pointer(array) as a char 
                      launch_detect_data->dat.detect_dat.SDaddress=SD_BECON_DATA;//since we are storing this in location zero, the address should say zero 
                      crc_check=crc16(launch_detect_data,510);                          
                      launch_detect_data->crc=crc_check;//put crc in last spot of data
                      //result=mmcErase(SD_BECON_DATA,SD_BECON_DATA);
                      ctl_timeout_wait(ctl_get_current_time()+100);//4.88 MS IS 5
                      result = mmcWriteBlock(SD_BECON_DATA ,(unsigned char*) launch_detect_data);//store that launch happened in the first spot
                      SDaddr+=1;//memory card is block address
                      //P4OUT^=BIT5; 
                }//closes else, launch has been verified before it died for some reason, the code will pick up where it left off in the count.  
   


             //I want to comment everything out from above this besides finding a new location from the sd card to store data below this is all the storage data infomation 
           

                 //start sd card task 
          ctl_task_run(&SD_card,BUS_PRI_EXTRA_HIGH,(void(*)(void*))writedatatoSDcard,NULL,"writedatatoSDcard",sizeof(stack4)/sizeof(stack4[0])-2,stack4+1,0);//set up task to run SD card, 
         
          //begin to log data from launch 
          while(switch_is_on)//this statement is for use with external switch
          {
            unsigned e;
            e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_adc,(1<<0),CTL_TIMEOUT_NONE,0);
              if(e&(0x01))
              {
                volatile unsigned int *arr = &ADC12MEM0;// creates a pointer, looks at the memory of mem0
                //if (handle_adc)//in adc interrupt handle_adc is set high, so this code become initialized
                //handle_adc = 0;//reset my personal flag to zero //I do not use this with events
                    //LAUNCH_LED_ON(); //LIGHT UP LED WHEN TAKING DATA 
                    if (data==0)
                    {
                    launch_data[data]=LEDL_DATA_ID;
                    data++;
                    temperory_SDaddr=SDaddr;
                    launch_data[data]=temperory_SDaddr;
                    data++;
                    launch_data[data]=temperory_SDaddr>>16;
                    data++;

                    }
                    if (accel_count==0)// only when accel_count=0 will the row number be displayed
                    {
                    //printf("%d\t", row);//prints row number
                    launch_data[data]=frame;
                    data++;
                    launch_data[data]=row;
                    data++;
                    row++;//add count to rows this is also SFID
                    } //closes if(accel_count==0)
              
                      for (i=0; i<6; ++i)// print first 6 adc measurements  
                       {
                        //printf("%d\t", arr[i]);
                        launch_data[data]=arr[i];
                        data++;
                        accel_count++;//add a count to accel_count,
                       }  //closes for (i=0; i<6; ++i)// print first 6 adc measurements
              
                        if (accel_count==24)// when accel_count is 24 it print last two columns
                        {
                        switch (gyro_count)
                            {
                            case 0:
                                   //printf("%d(G1)\t", arr[6]);
                                   launch_data[data]=arr[6];
                                   data++;
                                   gyro_count++;
                                   break;
                            case 1:
                                   //printf("%d(G2)\t", arr[7]);
                                   launch_data[data]=arr[7];
                                   data++;
                                   gyro_count++;
                                   GyroMuxon();//selecting G3 on mux for next pass
                                   //GyroSelfTeston();//turning on ST to see if there is a change
                                   break;
                            case 2:
                                   //printf("%d(G3)\t", arr[7]);
                                   launch_data[data]=arr[7];
                                   data++;
                                   gyro_count=0;
                                   GyroMuxoff();;//selecting G2 on mux for next pass
                                  // GyroSelfTestoff();//turning off the self test to see if there is a change 
                                   break;
                            }//closes switch (gyro_count)
   
           
                              switch (temp_display)
                                  {


                                  case 0: 
                                    if (frame_100==100)
                                    {
                                   // Temp_I2C_sensor(temp_measure);//here is the first temperature measurement passing an array 7 length long //at some point I will need to measure temperature sensors as a 
                                    //different task so that as I am waiting for the the return values I can be taking my other measurements. 
                                    //printf("%d\t",temp_measure[0]);//this is the spot for temperature data
                                    launch_data[data]=temp_measure[0];
                                    data++;
                                    }//closes if (frame_100==100)
                                    else if (frame_100==0)
                                    {
                                    ctl_events_set_clear(&handle_get_I2C_data,LEDL_EV_GET_TEMP_DATA,0);  
                                    data++;
                                    }//closes else if (frame_100==0)
                                    else
                                    {
                                    //printf("0000\t");
                                    launch_data[data]=0000;
                                    data++;
                                    }//closes else
                                    temp_display++;
                                    break;
                                  case 1:
                                    if (frame_100==100)
                                    {
                                    //printf("%d\t",temp_measure[1]);//this is the spot for temperature data
                                    launch_data[data]=temp_measure[1];
                                    data++;
                                    }// closes if (frame_100==100)
                                    else
                                    {
                                    //printf("0000\t");
                                    launch_data[data]=0000;
                                    data++;
                                    }//closes else 
                                    temp_display++;
                                    break;
                                  case 2:
                                    if (frame_100==100)
                                    {
                                    //printf("%d\t",temp_measure[2]);//this is the spot for temperature data
                                    launch_data[data]=temp_measure[2];
                                    data++;
                                    }//closes if (frame_100==100)
                                    else
                                    {
                                    //printf("0000\t");
                                    launch_data[data]=0000;
                                    data++;
                                    }//closes else 
                                    temp_display++;
                                    break;
                                  case 3:
                                    if (frame_100==100)
                                    {
                                    //printf("%d\t",temp_measure[3]);//this is the spot for temperature data
                                    launch_data[data]=temp_measure[3];
                                    data++;
                                    }//closes if (frame_100==100)
                                    else
                                    {
                                    //printf("0000\t");
                                    launch_data[data]=0000;
                                    data++;
                                    }//closes else
                                    temp_display++;
                                    break;
                                  case 4:
                                    if (frame_100==100)
                                    {
                                    //printf("%d\t",temp_measure[4]);//this is the spot for temperature data
                                    launch_data[data]=temp_measure[4];
                                    data++;
                                    }//closes if (frame_100==100)
                                    else
                                    {
                                    //printf("0000\t");
                                    launch_data[data]=0000;
                                    data++;
                                    }//closes else
                                    temp_display++;
                                    break;
                                  case 5:
                                    if (frame_100==100)
                                    {
                                    //printf("%d\t",temp_measure[5]);//this is the spot for temperature data
                                    launch_data[data]=temp_measure[5];
                                    data++;
                                    }//closes if (frame_100==100)
                                    else
                                    {
                                    //printf("0000\t");
                                    launch_data[data]=0000;
                                    data++;
                                    }//closes else
                                    temp_display++;
                                    break;
                                  case 6:
                                    if (frame_100==100)
                                    {
                                   //printf("%d\t",temp_measure[6]);//this is the spot for temperature data
                                   launch_data[data]=temp_measure[6];
                                    data++;
                                    frame_100=0;
                                    }// closes if (frame_100==100)
                                    else
                                    {
                                    //printf("0000\t");
                                    launch_data[data]=0000;
                                    data++;
                                    frame_100++;
                                    }//closes else 
                                    temp_display++;
                                    break;
                                  case 7:
                                    ticker_time=get_ticker_time();                 
                                    time2=ticker_time;
                                    time1=ticker_time>>16;

                                    //printf("%u\t",time1);//this is the spot for time1 data
                                    launch_data[data]=time1;
                                    data++;
                                    temp_display++;
                                    break;
                                  case 8: 
                                    //printf("%u\t",time2);
                                    launch_data[data]=time2;
                                    data++;
                                    crc_check=crc16(launch_data,510);
                                    launch_data[255]=crc_check;//put crc in last spot of data
                                    frame++;
                                    row=0;
                                    data=0;
                                    if (launch_data==launch_data1)
                                    {
                                    ctl_events_set_clear(&handle_SDcard,SD_EV_WRITE_1,0);
                                    launch_data=launch_data2;
                                    }//closes if(launch_data==launch_data1)
                                    else 
                                    {
                                    launch_data=launch_data1;
                                    ctl_events_set_clear(&handle_SDcard,SD_EV_WRITE_2,0);  
                                    }//closes else                         
                                    temp_display=0;
                                   break; 
                                    //add a count for incrementing frame number
                                  }//closes switch (temp_display) 
                        
                              //printf("\r\n");
                              accel_count=0;
                         }//closes  if (accel_count==24)
                       }//closes  if(e&(0x01))
         
                   }//closes while(switch_is_on==1) 
                 // printf("%d/t",launch_data);
                //turn off sensors here if this is where the empty while loop 
             //LAUNCH_LED_OFF(); //Turn off LED that shows data is no longer taking data 
             //before everything gets turned off, store the number of sd card sectors used and the last address stored 
             launch_detect_data->ledl_address=LEDL_DETECT_ID;
             launch_detect_data->dat.detect_dat.SDaddress=SDaddr;
             launch_detect_data->dat.detect_dat.SD_last_address=SDaddr;
             launch_detect_data->dat.detect_dat.accel_from_launch[0]=0;
             launch_detect_data->dat.detect_dat.accel_from_launch[1]=0;
             launch_detect_data->dat.detect_dat.accel_from_launch[2]=0;
             launch_detect_data->dat.detect_dat.accel_from_launch[3]=0;
             launch_detect_data->dat.detect_dat.max_and_min_from_launch[0]=0;
             launch_detect_data->dat.detect_dat.max_and_min_from_launch[1]=0;
             launch_detect_data->dat.detect_dat.max_and_min_from_launch[2]=0;
             launch_detect_data->dat.detect_dat.max_and_min_from_launch[3]=0;
             launch_detect_data->dat.detect_dat.max_and_min_from_launch[4]=0;
             launch_detect_data->dat.detect_dat.max_and_min_from_launch[5]=0;
             launch_detect_data->dat.detect_dat.number_of_detect=wake_up_attempt;
             launch_detect_data->dat.detect_dat.mode_status=MODE_ORBIT;
             crc_check=crc16(launch_data,510);                          
             launch_detect_data->crc=crc_check;//put crc in last spot of data
             result = mmcWriteBlock(SD_BECON_DATA ,(unsigned char*) &launch_detect_data); //(unsigned char*) casting my pointer(array) as a char 

             frame=0;
             ACCoff();
             GyroSleep();
             ctl_events_set_clear(&handle_SDcard,SD_EV_DIE,0);//this sends the flag to allow for the SD card to shut down 
             ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_SDcard,SD_EV_FINISHED,CTL_TIMEOUT_NONE,0);//this waits for SD card to finish writing the last block so that it can shutdown 
             mmc_pins_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWR LINE 
             SENSORSoff();
             BUS_free_buffer();
             initCLK_lv();//Reduce clock speed for low voltage application
             VREGoff();//turn Voltage regulator off for low power application
             WDT_STOP();
             //printf("TEST POINT");//this is the spot for temperature data

         
          }//closes checking for launch 
    }//closes  while(1)
         
}//closes function  launch_data_log(void *p)
  
*/





     



