#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include <ctl.h>
#include <ARCbus.h>
#include "uart.h"
#include "ADC_setup.h"
#include "I2C_sensor.h"
#include "timer_setup.h"
#include "log_data.h"
#include <SDlib.h>

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
         ADCread = Getc();//We expect the ascii key for return to be in the return value.  
         }ADCread=0;
     }
    }
    }
*/
CTL_EVENT_SET_t handle_SDcard;
CTL_EVENT_SET_t handle_get_I2C_data;
//static int launch_data[LAUNCH_DATA_SIZE];
int SDaddr;
static int *launch_data1;
static int *launch_data2;
int temp_measure[TEMP_ARRAY_SIZE];

//void launch_data_log(void *p){// used to set up code USE THIS ONE WITH THE TASK LIBRARY 
void launch_data_log(void){// used to set up code // USE THIS ONE FOR JUST USING IT AS A FUNCITON 
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
  
  int *launch_data;
  
  //P7OUT &= ~(BIT7);
  //P8OUT &= ~(BIT6);

  adcsetup();//adc setup 
  ctl_events_init(&handle_SDcard, 0);
  ctl_events_init(&handle_SDcard, 0);
  //timersetup();//start timer //using jesse's timer in main
  
  /*while(!async_isOpen()){
    ctl_timeout_wait(ctl_get_current_time()+1000);
  }*/
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  if (buffer==NULL){
  printf("unable to use bus buffer \r\n");
  return;
  }
  
  launch_data1=buffer;
  launch_data2=buffer+512;
  // __enable_interrupt();// THIS IS A PREDEFINED GLOBAL INTERRUPT ENABLE BIT IN THE MSP LIBRARY 
  printf("\rLEDL Sensor read test. Press s to stop. \r\n>");// 
  launch_data=launch_data1;
  //printf("\rrow\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA0\tA1\tA2\tA3\tA4\tA5\tA6\tA7\r\n>");//gives columns titles 
  while(async_CheckKey()!='s')
  //for(;;)
  {
  unsigned e;
  e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_adc,(1<<0),CTL_TIMEOUT_NONE,0);
    if(e&(0x01))
    {
      volatile unsigned int *arr = &ADC12MEM0;// creates a pointer, looks at the memory of mem0
      //if (handle_adc)//in adc interrupt handle_adc is set high, so this code become initialized
    
        {
        //handle_adc = 0;//reset my personal flag to zero //I do not use this with events
     

          if (accel_count==0)// only when accel_count=0 will the row number be displayed
          {
          //printf("%d\t", row);//prints row number
          row++;//add count to rows this is also SFID
          }  
            for (i=0; i<6; ++i)// print first 6 adc measurements  
             {
              //printf("%d\t", arr[i]);
              launch_data[data]=arr[i];
              data++;
              accel_count++;//add a count to accel_count,
             }  
              
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
                         signal_mux=1;//selecting G3 on mux for next pass
                         break;
                  case 2:
                         //printf("%d(G3)\t", arr[7]);
                         launch_data[data]=arr[7];
                         data++;
                         gyro_count=0;
                         signal_mux=0;//selecting G2 on mux for next pass
                         break;
                  }
/*
                   {

                        if (frame_100==0)
                            {
                            ctl_events_set_clear(&handle_get_I2C_data,get_I2C_data_flag,0); 
                            launch_data[data]=0000;
                            data++;
                            }
                        else if (frame_100==10)
                            {
								if (temp_display<=7)
								{
								// Temp_I2C_sensor(temp_measure);//here is the first temperature measurement passing an array 7 length long //at some point I will need to measure temperature sensors as a 
								//different task so that as I am waiting for the the return values I can be taking my other measurements. 
								//printf("%d\t",temp_measure[0]);//this is the spot for temperature data
								launch_data[data]=temp_measure[temp_display];
								data++;
								}
								else if (temp_display==7)
								{
								 ticker_time=get_ticker_time();                 
								 time2=ticker_time;
								 time1=ticker_time>>16;

								//printf("%u\t",time1);//this is the spot for time1 data
								launch_data[data]=time1;
								data++;
								}
								else 
								{
								launch_data[data]=time2;
								data=0;
									if (launch_data==launch_data1)
									{
									ctl_events_set_clear(&handle_SDcard,SDcardwriteflag1,0);
									launch_data=launch_data2;
									}
									else 
									{
									launch_data=launch_data1;
									ctl_events_set_clear(&handle_SDcard,SDcardwriteflag2,0);  
									}                         
								temp_display=0;
								}
							}
						else
							{
							  //printf("0000\t");
							  launch_data[data]=0000;
							  data++;
							}
                          temp_display++;
*/
           
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
                          }
                          else if (frame_100==0)
                          {
                          ctl_events_set_clear(&handle_get_I2C_data,get_I2C_data_flag,0);  
                          data++;
                          }
                          else
                          {
                          //printf("0000\t");
                          launch_data[data]=0000;
                          data++;
                          }
                          temp_display++;
                          break;
                        case 1:
                          if (frame_100==100)
                          {
                          //printf("%d\t",temp_measure[1]);//this is the spot for temperature data
                          launch_data[data]=temp_measure[1];
                          data++;
                          }
                          else
                          {
                          //printf("0000\t");
                          launch_data[data]=0000;
                          data++;
                          }
                          temp_display++;
                          break;
                        case 2:
                          if (frame_100==100)
                          {
                          //printf("%d\t",temp_measure[2]);//this is the spot for temperature data
                          launch_data[data]=temp_measure[2];
                          data++;
                          }
                          else
                          {
                          //printf("0000\t");
                          launch_data[data]=0000;
                          data++;
                          }
                          temp_display++;
                          break;
                        case 3:
                          if (frame_100==100)
                          {
                          //printf("%d\t",temp_measure[3]);//this is the spot for temperature data
                          launch_data[data]=temp_measure[3];
                          data++;
                          }
                          else
                          {
                          //printf("0000\t");
                          launch_data[data]=0000;
                          data++;
                          }
                          temp_display++;
                          break;
                        case 4:
                          if (frame_100==100)
                          {
                          //printf("%d\t",temp_measure[4]);//this is the spot for temperature data
                          launch_data[data]=temp_measure[4];
                          data++;
                          }
                          else
                          {
                          //printf("0000\t");
                          launch_data[data]=0000;
                          data++;
                          }
                          temp_display++;
                          break;
                        case 5:
                          if (frame_100==100)
                          {
                          //printf("%d\t",temp_measure[5]);//this is the spot for temperature data
                          launch_data[data]=temp_measure[5];
                          data++;
                          }
                          else
                          {
                          //printf("0000\t");
                          launch_data[data]=0000;
                          data++;
                          }
                          temp_display++;
                          break;
                        case 6:
                          if (frame_100==100)
                          {
                         //printf("%d\t",temp_measure[6]);//this is the spot for temperature data
                         launch_data[data]=temp_measure[6];
                          data++;
                          frame_100=0;
                          }
                          else
                          {
                          //printf("0000\t");
                          launch_data[data]=0000;
                          data++;
                          frame_100++;
                          }
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
                          data=0;
                          if (launch_data==launch_data1)
                          {
                          ctl_events_set_clear(&handle_SDcard,SDcardwriteflag1,0);
                          launch_data=launch_data2;
                          }
                          else 
                          {
                          launch_data=launch_data1;
                          ctl_events_set_clear(&handle_SDcard,SDcardwriteflag2,0);  
                          }                         
                          temp_display=0;
                         break; 
                          //add a count for incrementing frame number
                        } 
                        
                    //printf("\r\n");
                    accel_count=0;
               }
             }
         
         } 
        // printf("%d/t",launch_data);
      
   }
   BUS_free_buffer();
   }
   

// Created event to store data to sd card 
void writedatatoSDcard(void)
{
  
  unsigned long SD_card_write_time;
  unsigned SD_card_write;
  SDaddr=0;
  for(;;){//I may want to add code that looks for a new spot to put data if there is data in code(SD card code). 
      SD_card_write=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_SDcard,3,CTL_TIMEOUT_NONE,0);
         if(SD_card_write&(SDcardwriteflag1))
           {  //SD_card_write_time=get_ticker_time();  
             // printf("SD card write start %u\t",get_ticker_time());
              mmcWriteBlock(SDaddr,(unsigned char*) launch_data1); //(unsigned char*) casting my pointer(array) as a char 
              SDaddr+=1;//memory card is block address
              //SD_card_write_time=get_ticker_time();  
              //printf("SD card write finish %u\t",get_ticker_time());
             }
      if(SD_card_write&(SDcardwriteflag2))
           {
              mmcWriteBlock(SDaddr,(unsigned char*) launch_data2); //(unsigned char*) casting my pointer(array) as a char 
              SDaddr+=1;//memory card is block address
             
            }
          }
 }
//create an event to take I2C data 
void takeI2Cdata(void)
{

unsigned get_I2C_data;
for(;;){//take temperatue data
        get_I2C_data=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_get_I2C_data,get_I2C_data_flag,CTL_TIMEOUT_NONE,0);
        if (get_I2C_data&(get_I2C_data_flag))
        {
        Temp_I2C_sensor(temp_measure);
        }
       }
      }

void orbit_data_log(void){
//need to, temperate sensors, gyros, clyde, take magnetometer data at request of ACDS, and store important information into sd card
//can resuse temperate and gyro logging data, use the same SD card code, // set code to sample 0-15 samples a second for magnetometer depending on ACDS request
//ADC timer has to be changed. 
 adcsetup();//adc setup

}

//think about start up code, how ledl wakes from launch 
