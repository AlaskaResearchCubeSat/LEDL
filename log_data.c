#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include <ctl.h>
#include <ARCbus.h>
#include "ADC_setup.h"
#include "I2C_sensor.h"
#include "timer_setup.h"
#include "log_data.h"
#include <SDlib.h>
#include "SensorsOnOff.h"
#include "sensor-interface.h"
#include <crc.h>


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


//static int launch_data[LAUNCH_DATA_SIZE];
unsigned long SDaddr=0;
static int *launch_data1;
static int *launch_data2;
int temp_measure[TEMP_ARRAY_SIZE];
int clyde_data[CLYDE_ARRAY_SIZE];
int clyde_measure;
int mag_measure;


unsigned stack4[1+100+1];//stacks need to be a globals 
 


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
      SDaddr+=1;
    
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
   event1=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_LaunchData,LaunchData_flag,CTL_TIMEOUT_NONE,0);
    if(event1&(LaunchData_flag))
    {
      int mmcReturnValue;
     //turn up the voltage
      VREGon();
      
      //need to have a setteling time for the voltage regulator
      //then initalize the 16 Mhz clock 
      ctl_timeout_wait(ctl_get_current_time()+5);//4.88 MS
      SENSORSon();//turn on sensors
     
      buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);

      if (buffer==NULL){
      printf("unable to use bus buffer \r\n");
      return;
      }//closes if(buffer==NULL) statement 
      
      launch_data1=(int*)buffer;
      launch_data2=(int*)(buffer+512);
      launch_data=(int*)launch_data1;
      //turn on sensors here 
      ACCon();//turn on acclerometers 
      GyroWakeUp();
      
          //initalize the SD card   
          ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 

          mmcInit_msp();
          mmcReturnValue=mmcInit_card();
          if (mmcReturnValue==MMC_SUCCESS){
          printf("\rCard initalized Sucessfully\r\n");
          }
          else {
          printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
          
          }
          ctl_task_run(&SD_card,3,(void(*)(void*))writedatatoSDcard,NULL,"writedatatoSDcard",sizeof(stack4)/sizeof(stack4[0])-2,stack4+1,0);//set up task to run SD card, 
          //this gets started after I begin taking data and stops when I am finished taking data, this is done here to prevent the SD card lines from turning on and off from main 
         initCLK();
         ctl_events_set_clear(&handle_LaunchData,0, LaunchData_flag);
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
              launch_data[data]=LEDL_ID;
              data++;
              launch_data[data]=SDaddr;
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
                              ctl_events_set_clear(&handle_get_I2C_data,get_I2C_data_flag,0);  
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
                              data=0;
                              if (launch_data==launch_data1)
                              {
                              ctl_events_set_clear(&handle_SDcard,SDcardwriteflag1,0);
                              launch_data=launch_data2;
                              }//closes if(launch_data==launch_data1)
                              else 
                              {
                              launch_data=launch_data1;
                              ctl_events_set_clear(&handle_SDcard,SDcardwriteflag2,0);  
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
       frame=0;
       ACCoff();
       GyroSleep();
       ctl_events_set_clear(&handle_SDcard,SDcardDieflag,0);//this sends the flag to allow for the SD card to shut down 
       ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_SDcard,SDcardfinishedflag,CTL_TIMEOUT_NONE,0);//this waits for SD card to finish writing the last block so that it can shutdown 
       mmcInit_msp_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWR LINE 
       SENSORSoff();
       BUS_free_buffer();
       initCLK_lv();//Reduce clock speed for low voltage application
       VREGoff();//turn Voltage regulator off for low power application
       WDT_STOP();
       //printf("TEST POINT");//this is the spot for temperature data

       }//closes  if(event1&(LaunchData_flag))
     }//closes  while()
   }//closes function  launch_data_log(void *p)
   

// Created event to store data to sd card 
void writedatatoSDcard(void)
{
  unsigned countforLED=0; 
  unsigned long SD_card_write_time;
  unsigned SD_card_write;
  int result;
  
  for(;;){//I may want to add code that looks for a new spot to put data if there is data in code(SD card code). 
      SD_card_write=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_SDcard,7,CTL_TIMEOUT_NONE,0);//the 7 means your looking at all 3 flags 
         if(SD_card_write&(SDcardwriteflag1))
           {  //SD_card_write_time=get_ticker_time();  
             // printf("SD card write start %u\t",get_ticker_time());
              result = mmcWriteBlock(SDaddr,(unsigned char*) launch_data1); //(unsigned char*) casting my pointer(array) as a char 
              SDaddr+=1;//memory card is block address
              //printf("SD card #1 returned, %i\r\n",result);
              //SD_card_write_time=get_ticker_time();  
              //printf("SD card write finish %u\t",get_ticker_time());
              countforLED++;
              if (countforLED==250)
              {
              SD_LED();
              countforLED=0;
              }
              }
      if(SD_card_write&(SDcardwriteflag2))
           {
              result= mmcWriteBlock(SDaddr,(unsigned char*) launch_data2); //(unsigned char*) casting my pointer(array) as a char 
              SDaddr+=1;//memory card is block address
           //   printf("SD card #2 returned, %i\r\n",result);
              countforLED++;
              if (countforLED==250)
              {
              SD_LED();
              countforLED=0;
              }
            }
      if(SD_card_write&(SDcardDieflag))
         {
         int SDaddr_for_total_blocks_used=0;
         launch_data2[0]=SDaddr;
         SD_LED_OFF();
         result= mmcWriteBlock(SDaddr_for_total_blocks_used,(unsigned char*) launch_data2); //(unsigned char*) casting my pointer(array) as a char   
         ctl_events_set_clear(&handle_SDcard,SDcardfinishedflag,0);//this is to tell that the SD card is finished writing and can be shut down if necessary 
        //  printf("SD card shutdown \r\n");
         return;
         }
          }
 }
//create an event to take I2C data 
void takeI2Cdata(void)
{
int k=0;
int l=0;
unsigned get_I2C_data;
for(;;){//take temperature data 
        get_I2C_data=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_get_I2C_data,get_I2C_data_flag,CTL_TIMEOUT_NONE,0);
        if (get_I2C_data&(get_I2C_data_flag))
        {
        Temp_I2C_sensor(temp_measure);

        }
       }
      }


//create an event to take clyde data 
void takeclydedata(void)
{
unsigned get_clyde_data;
for(;;){//take eps data 
        get_clyde_data=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_get_clyde_data,get_clyde_data_flag,CTL_TIMEOUT_NONE,0);
        if (get_clyde_data&(get_clyde_data_flag))
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

       }//closes  if(event1&(LaunchData_flag))
     }//closes  while()
   }//closes function  Orbit_data_log(void *p)











     



*/