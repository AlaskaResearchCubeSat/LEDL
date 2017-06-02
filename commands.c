/**********************************************************************************************************************************************
The commands.c file is for commands that will be displayed through the serial terminal. 
In order to add a command you must create a function as seen below.
Then function must be added to the "const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd}" table at the end of the file.
**********************************************************************************************************************************************/
#include <msp430.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <terminal.h>
#include <commandLib.h>
#include <stdlib.h>
#include <ARCbus.h>
#include <SDlib.h>
#include <i2c.h>
#include "ADC_setup.h"
#include "log_data.h"
#include <commandLib.h>
#include <errno.h>

//define printf formats
#define HEXOUT_STR    "%02X "
#define INTERGER_STR  "%d "
#define ASCIIOUT_STR  "%c "
#define UNSIGNED_STR  "%u "
#define FLOAT_STR     "%f "

SD_block_addr SD_read_addr;

//*********************************************************** passing arguments over the terminal *********************************************
int example_command(char **argv,unsigned short argc){
  int i,j;
  //TODO replace printf with puts ? 
  printf("This is an example command that shows how arguments are passed to commands.\r\n""The values in the argv array are as follows : \r\n");
  for(i=0;i<=argc;i++){
    printf("argv[%i] = 0x%p\r\n\t""string = \"%s\"\r\n",i,argv[i],argv[i]);
    //print out the string as an array of hex values
    j=0;
    printf("\t""hex = {");
    do{
      //check if this is the first element
      if(j!='\0'){
        //print a space and a comma
        printf(", ");
      }
      //print out value
      printf("0x%02hhX",argv[i][j]);
    }while(argv[i][j++]!='\0');
    //print a closing bracket and couple of newlines
    printf("}\r\n\r\n");
  }
  return 0;
}


/*********************************************************** Using the Timer_A1 ***************************************************************
* DONT USE TIMER0_Ax_VECTOR !!! this interrupt is use in library code and will cause a collision 
* Use TIMERx_Ay_VECTOR x=2,3 & y=0,1
* TIMER0_Ax_VECTOR used in ARClib ? 
* TIMER1_Ax_VECTOR used in ????
**********************************************************************************************************************************************/
int example_timer_IR(char **argv,unsigned short argc){
  int timer_check;
  WDTCTL = WDTPW+WDTHOLD;                                   // Stop WDT
  P7DIR |= 0xFF;                                            // Setting port 7 to drive LED's (0xFF ==1111 1111)
  P7OUT = 0x00;                                             // Set all LED's on port 7 to start all off
//************************************ Set up clock [0] 
  TA2CTL |= TASSEL__ACLK | MC_2;                            // Setting Timer_A to ACLK(TASSEL_1) to continuous mode(MC_2)

//*********************************** Set timer interrupt enable [1] 
  TA2CCTL0 |= CCIE;                                          // Capture/compare interrupt enable #0
  TA2CCTL1 |= CCIE;                                          // Capture/compare interrupt enable #1

//*********************************** Set the timer count IR value [2] 
  TA2CCR0 = 10000;                                           // Timer0_A3 Capture/Compare @ 10000 counts
  TA2CCR1 = 1000;                                            // TA0IV_1 Capture/Compare @ 1000 counts

   while ((timer_check=getchar()) != EOF)                                                // poll in while loop until a key press
   {
    ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS,0, 1<<15, CTL_TIMEOUT_DELAY, 1000); // wait in loop 
   }
  reset(0,ERR_SRC_CMD,CMD_ERR_RESET,0);                     // reset clock registers 
  return 0;
}

// ********************************* Timer_A0 interrupt code 
void Timer_A2_A0(void)__interrupt[TIMER2_A0_VECTOR]{     // Timer A0 interrupt service routine TA0IV_TA0IFG. 
  P7OUT^=BIT0; // toggle LEDs when IR is called
}

void Timer_A2_A1(void)__interrupt[TIMER2_A1_VECTOR]{     // Timer A0 interrupt service routine for capture comp 1 and 2
        P7OUT^=BIT1; // light LEDs
}

//******************************************* Using the SD card *******************************************************************************

int SD_write(char **argv,unsigned short argc){
char buff[512];
int mmcReturnValue, result , i;
  mmcReturnValue=mmc_is_init(); 
  if (mmcReturnValue<0){
  mmcInit_msp();  // Sets up the interface for the card
  mmc_pins_on();  //Sets up MSP to talk to SD card
  mmcReturnValue=mmcInit_card();
  }
  if (mmcReturnValue==MMC_SUCCESS){ // check good initialization 
    printf("\rCard initalized Sucessfully\r\n");
  }
  else{
    printf("Check SD card.\r\nInitalized failed.\r\n Error %i\r\n",mmcReturnValue);
  }

//populate buffer block
  for(i=1;i<=argc;i++) {// ignore 0 *argv input (argv[0]--> "SD_write" )
    strcat(buff,argv[i]); // appends chars from one string to another
    strcat(buff,"|");     // separates strings with | as strcat eats NULL
  }

//write to SD card
  result= mmcWriteBlock(100,buff); //(unsigned char*) casting my pointer(array) as a char 
 
  if (result>=0){ // check SD write 
  printf("SD card write success.\r\n");
  }
  else{
    printf("SD card write failed.\r\nError %i\r\n",result);
  }
  return 0;
}

int SD_read(char **argv,unsigned short argc){
  #define ASCIIOUT_STR  "%c "
 char buffer[512];
 int mmcReturnValue,resp , i;

  mmcInit_msp();  // Sets up the interface for the card
  mmc_pins_on();  //Sets up MSP to talk to SD card
  mmcReturnValue=mmcInit_card();
  
  //read from SD card
 resp=mmcReadBlock(100,buffer);
  //print response from SD card
  printf("%s\r\n",SD_error_str(resp));

        for(i=0;i<9;i++){//changed the 512 to 256 which is a result of changing CHAR TO INT

        if(i<8){
          printf(HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }

        else{
          printf(HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }
        }
        return 0;
}

int printCmd(char **argv, unsigned short argc){//copied print mmcdump command to change the format of how the data is presented in tera term. 
  int resp; 
  int *buffer=NULL;//changed from char to int instead of each char being 1 byte their are now two bytes, resulting in being half as many things to index 
  LEDL_TEST_LAUNCH *launch_detect_data;


  unsigned long sector=0;
  int i;
  mmcInit_msp();
  mmcInit_card();
  if(argc!=0){
    if(1!=sscanf(argv[1],"%lu",&sector)){
      printf("Error with sector\r\n");
      return -1;
    }
  }

  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //below is done after get buffer
  launch_detect_data=(LEDL_TEST_LAUNCH*)buffer;
  //read from SD card
  resp=mmcReadBlock(sector,(unsigned char*)buffer);
  //print response from SD card
  printf("%s\r\n",SD_error_str(resp));
  //print out buffer

  //check to see what kind of data packet is being printed 
      if (buffer[0]==LEDL_DATA_ID  )//check to see if it is a Launch Packet 
      {
            //define printf formats
#define HEXOUT_STR    "%02X "
#define INTERGER_STR  "%d "
#define ASCIIOUT_STR  "%c "
#define UNSIGNED_STR  "%u "
#define FLOAT_STR     "%f "
#define LONG_STR      "%lu "
        //in printing all my data, i want to print out first the LEDL packet id, then the sd address that this is supposed to be saved to
      //the data will follow, then last by the crc 
         printf("LEDL PACKET="HEXOUT_STR"\r\n", buffer[0]);
         printf("SD_ADDRESS="LONG_STR"\r\n",*(unsigned long*)&buffer[1]);
         //print all the data 
        for(i=0;i<9;i++){//changed the 512 to 256 which is a result of changing CHAR TO INT

        if(i<8){
          printf(UNSIGNED_STR UNSIGNED_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }

        else{
          printf(UNSIGNED_STR UNSIGNED_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR UNSIGNED_STR UNSIGNED_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }
        }
        //last is print the crc 
        printf("CRC="UNSIGNED_STR"\r\n",buffer[255]);//last space in the data packet 
        ctl_timeout_wait(ctl_get_current_time()+10); 
     }
  else if(buffer[0]==LEDL_DETECT_ID )//check to see if it is a detect packet 
      {
       //below is after the transfer is done
      printf("LEDL PACKET="HEXOUT_STR" \r\n"
             "SD_ADDRESS="LONG_STR"\r\n"
             "LAST SD USED="LONG_STR"\r\n"
             "AVG ACCELEROMETER DATA X="FLOAT_STR"Y="FLOAT_STR"Z="FLOAT_STR"TOTAL ACC = "FLOAT_STR"\r\n"
             "MAX AND MIN ACCELEROMTER DATA\r\nXMAX="FLOAT_STR"XMIN="FLOAT_STR"\r\nYMAX="FLOAT_STR"YMIN="FLOAT_STR"\r\nZMAX="FLOAT_STR"ZMIN="FLOAT_STR"\r\n"
             "NUMBER OF DETECT="INTERGER_STR"\r\n"
             "LEDL MODE="UNSIGNED_STR"\r\n"
             "CRC = "UNSIGNED_STR"\r\n",
            launch_detect_data->ledl_address,//LEDL_DETECT_ID
            launch_detect_data->dat.detect_dat.SDaddress,//SDaddr
            launch_detect_data->dat.detect_dat.SDaddress,//Last SD addr used by logging data
            launch_detect_data->dat.detect_dat.accel_from_launch[0],//xavg
            launch_detect_data->dat.detect_dat.accel_from_launch[1],//yavg
            launch_detect_data->dat.detect_dat.accel_from_launch[2],//zavg
            launch_detect_data->dat.detect_dat.accel_from_launch[3],//totalacc
            launch_detect_data->dat.detect_dat.max_and_min_from_launch[0],//maxx
            launch_detect_data->dat.detect_dat.max_and_min_from_launch[1],//minx
            launch_detect_data->dat.detect_dat.max_and_min_from_launch[2],//maxy
            launch_detect_data->dat.detect_dat.max_and_min_from_launch[3],//miny
            launch_detect_data->dat.detect_dat.max_and_min_from_launch[4],//maxz
            launch_detect_data->dat.detect_dat.max_and_min_from_launch[5],//minz
            launch_detect_data->dat.detect_dat.number_of_detect,//wake_up_attempt
            launch_detect_data->dat.detect_dat.mode_status,//specify what mode ledl is in
            launch_detect_data->crc);//crc check 

      }
  else 
      {
      printf("Error : Packet address not found %i\r\n", buffer[0]);
      {
            //define printf formats
        #define HEXOUT_STR    "%02X "
        #define INTERGER_STR  "%d "
        #define ASCIIOUT_STR  "%c "
        #define UNSIGNED_STR  "%u "
        #define FLOAT_STR     "%f "
        #define LONG_STR      "%lu "
        //in printing all my data, i want to print out first the LEDL packet id, then the sd address that this is supposed to be saved to
      //the data will follow, then last by the crc 
         printf("LEDL PACKET="HEXOUT_STR"\r\n", buffer[0]);
         printf("SD_ADDRESS="LONG_STR"\r\n",*(unsigned long*)&buffer[1]);
         //print all the data 
        for(i=0;i<9;i++){//changed the 512 to 256 which is a result of changing CHAR TO INT

        if(i<8){
          printf(UNSIGNED_STR UNSIGNED_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }

        else{
          printf(UNSIGNED_STR UNSIGNED_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR UNSIGNED_STR UNSIGNED_STR "\r\n",
          buffer[i*28+3],buffer[i*28+4],buffer[i*28+5],buffer[i*28+6],buffer[i*28+7],buffer[i*28+8],buffer[i*28+9],buffer[i*28+10],buffer[i*28+11],buffer[i*28+12],buffer[i*28+13],
          buffer[i*28+14],buffer[i*28+15],buffer[i*28+16],buffer[i*28+17],buffer[i*28+18],buffer[i*28+19],buffer[i*28+20],buffer[i*28+21],buffer[i*28+22],buffer[i*28+23],
          buffer[i*28+24],buffer[i*28+25],buffer[i*28+26],buffer[i*28+27],buffer[i*28+28],buffer[i*28+29],buffer[i*28+30]);
          }
        }
        //last is print the crc 
        printf("CRC="UNSIGNED_STR"\r\n",buffer[255]);//last space in the data packet 
        ctl_timeout_wait(ctl_get_current_time()+10); 
     }
      }
  //free buffer
  BUS_free_buffer();
  
  return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////

//TODO FIX ZACKS CODE D:
int send_I2C(char **argv,unsigned short argc){
  unsigned short addr, dat, resp;

  // There should only be two passed arguments -- destination address and I2C command
  if (argc > 2) {
    printf("Too many arguments.\n");
    return -1;
  }

  // Get integer value of address from passed string
  addr=strtoul(argv[1], NULL, 0);

  // Get integer value of I2C command from passed string
  dat=strtoul(argv[2], NULL, 0);

  initI2C(4,1,0);
  // I2C_tx expects a char* for transmitted data, not an integer
  resp=i2c_tx(addr, (unsigned char*)&dat, 1);
  if (resp < 0){
    printf("I2C transaction error.\n");
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
int mmc_eraseCmd(char **argv, unsigned short argc){
  unsigned long start,end;
  int resp;
  
  mmcInit_msp();
  mmcInit_card();


  //check arguments
  if(argc!=2){
    printf("Error : %s requiors two arguments\r\n",argv[0]);
    return 1;
  }
  errno=0;
  start=strtoul(argv[1],NULL,0);
  end=strtoul(argv[2],NULL,0);
  if(errno){
    printf("Error : could not parse arguments\r\n");
    return 2;
  }
  printf("Erasing from %lu to %lu\r\n",start,end);
  //send erase command
  resp=mmcErase(start,end);
  printf("%s\r\n",SD_error_str(resp));
  return 0;
}


///////////////////////////////////////////
/*

//////////////////////////////////////////////////////////////////////////////////////////
int mmc_dump(char **argv, unsigned short argc){
  int resp; 
  char *buffer=NULL;
  unsigned long sector=0;
  int i;
  if(argc!=0){
    if(1!=sscanf(argv[1],"%ul",&sector)){
      printf("Error with sector\r\n");
      return -1;
    }
  }
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //read from SD card
  resp=mmcReadBlock(sector,(unsigned char*)buffer);
  //print response from SD card
  printf("%s\r\n",SD_error_str(resp));
  //print out buffer
  for(i=0;i<512/16;i++){
    printf(HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR "\r\n",
    buffer[i*16],buffer[i*16+1],buffer[i*16+2],buffer[i*16+3],buffer[i*16+4],buffer[i*16+5],buffer[i*16+6],buffer[i*16+7],buffer[i*16+8],buffer[i*16+9],buffer[i*16+10],
    buffer[i*16+11],buffer[i*16+12],buffer[i*16+13],buffer[i*16+14],buffer[i*16+15]);
  }
  //free buffer
  BUS_free_buffer();
  ctl_timeout_wait(ctl_get_current_time()+10);
  return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

int mmc_reset(char **argv, unsigned short argc){
  int resp;
  //setup the SD card
  resp=mmcGoIdle();
  //set some LEDs
  //P7OUT&=~(BIT7|BIT6);
  if(resp==MMC_SUCCESS){
  //  P7OUT|=BIT7;
  }else{
  //  P7OUT|=BIT6;
  }
  printf("Response = %i\r\n",SD_error_str(resp));
  return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////
int mmc_multiWTstCmd(char **argv, unsigned short argc){
  unsigned char *ptr;
  int stat;
  unsigned long i,start,end;
  unsigned short multi=1;
  if(argc<2){
    printf("Error : too few arguments\r\n");
    return -1;
  }
  if(argc>3){
    printf("Error : too many arguments\r\n");
    return -2;
  }
  //get start and end
  errno=0;
  start=strtoul(argv[1],NULL,0);
  end=strtoul(argv[2],NULL,0);
  if(errno){
    printf("Error : could not parse arguments\r\n");
    return 2;
  }
  if(argc>2){
    if(!strcmp("single",argv[3])){
      multi=0;
    }else if(!strcmp("multi",argv[3])){
      multi=1;
    }else{
      //unknown argument
      printf("Error : unknown argument \"%s\".\r\n",argv[3]);
      return -3;
    }
  }
  //TESTING: set line high
  P8OUT|=BIT0;
  if(!multi){
    //write each block in sequence
    for(i=start,ptr=NULL;i<end;i++,ptr+=512){
      if((stat=mmcWriteBlock(i,ptr))!=MMC_SUCCESS){
        printf("Error writing block %li. Aborting.\r\n",i);
        printf("%s\r\n",SD_error_str(stat));
        return 1;
      }
    }
  }else{
    //write all blocks with one command
    if((stat=mmcWriteMultiBlock(start,NULL,end-start))!=MMC_SUCCESS){
      printf("Error with write. %i\r\n",stat);
      printf("%s\r\n",SD_error_str(stat));
      return 1;
    }
  }
  //TESTING: set line low
  P8OUT&=~BIT0;
  printf("Data written sucussfully\r\n");
  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
int mmc_multiRTstCmd(char **argv, unsigned short argc){
  unsigned char *ptr,*buffer;
  int resp;
  unsigned long i,start,end;
  unsigned short multi=1;
  if(argc<2){
    printf("Error : too few arguments\r\n");
    return -1;
  }
  if(argc>3){
    printf("Error : too many arguments\r\n");
    return -2;
  }
  //get start and end
  errno=0;
  start=strtoul(argv[1],NULL,0);
  end=strtoul(argv[2],NULL,0);
  if(errno){
    printf("Error : could not parse arguments\r\n");
    return -4;
  }
  if((end-start)*512>BUS_get_buffer_size()){
    printf("Error : data size too large for buffer.\r\n");
    return -5;
  }
  if(argc>2){
    if(!strcmp("single",argv[3])){
      multi=0;
    }else if(!strcmp("multi",argv[3])){
      multi=1;
    }else{
      //unknown argument
      printf("Error : unknown argument \"%s\".\r\n",argv[3]);
      return -3;
    }
  }
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //TESTING: set line high
  P8OUT|=BIT0;
  if(!multi){
    //write each block in sequence
    for(i=start,ptr=buffer;i<end;i++,ptr+=512){
      if((resp=mmcReadBlock(i,ptr))!=MMC_SUCCESS){
        printf("Error reading block %li. Aborting.\r\n",i);     
        printf("%s\r\n",SD_error_str(resp));
        //free buffer
        BUS_free_buffer();
        return 1;   
      }
    }
  }else{
    //write all blocks with one command
    if((resp=mmcReadBlocks(start,end-start,buffer))!=MMC_SUCCESS){
      printf("Error with read.\r\n");
      printf("resp = 0x%04X\r\n%s\r\n",resp,SD_error_str(resp));
      //free buffer
      BUS_free_buffer();
      return 1;   
    }
  }
  //TESTING: set line low
  P8OUT&=~BIT0;
  //print out buffer
  for(i=0;i<(end-start)*512/16;i++){
    printf(HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR HEXOUT_STR "\r\n",
    buffer[i*16],buffer[i*16+1],buffer[i*16+2],buffer[i*16+3],buffer[i*16+4],buffer[i*16+5],buffer[i*16+6],buffer[i*16+7],buffer[i*16+8],buffer[i*16+9],buffer[i*16+10],
    buffer[i*16+11],buffer[i*16+12],buffer[i*16+13],buffer[i*16+14],buffer[i*16+15]);
  }
  printf("Data read sucussfully\r\n");
  //free buffer
  BUS_free_buffer();
  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
int mmc_write(char **argv, unsigned short argc){
  //pointer to buffer, pointer inside buffer, pointer to string
  unsigned char *buffer=NULL,*ptr=NULL,*string;
  //response from block write
  int resp;
  int i ;
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //clear all bytes in buffer
  memset(buffer,0,512);
  //concatinate arguments into one big string with spaces in between
  for(ptr=buffer,i=1; i<=argc; i++){
    string=(unsigned char*)argv[i];
    while(*string!=0){
      *ptr++=*string++;
    }
    *ptr++=' ';
  }
  //Terminate string
  *(ptr-1)=0;
  //write data
  resp=mmcWriteBlock(0,buffer);
  //check if write was successful
  if(resp==MMC_SUCCESS){
    printf("data written to memeory\r\n");
  }else{
    printf("resp = 0x%04X\r\n%s\r\n",resp,SD_error_str(resp));
  }
  //free buffer
  BUS_free_buffer();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int mmc_read(char **argv, unsigned short argc){
  char resp,*ptr=NULL,*buffer=NULL; 
   //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  //init buffer
  memset(buffer,0,513);
  //read from SD card
  resp=mmcReadBlock(0,(unsigned char*)buffer);
  //check for error
  if(resp!=MMC_SUCCESS){
    //print error from SD card
    printf("%s\r\n",SD_error_str(resp));
    return 1;
  }
  //force termination after last byte
  buffer[512]=0;
  //check for non printable chars
  for(ptr=(char*)buffer;ptr!=0;ptr++){
    if(!isprint(*ptr)){
      //check for null
      if(*ptr){
        printf("String prematurely terminated due to a non printable character.\r\n");
      }
      *ptr=0;
      break;
    }
  }
  //print out the string
  printf("Here is the string you wrote:\r\n\'%s\'\r\n",buffer);
  //free buffer
  BUS_free_buffer();
  return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/

extern int adcready;  
extern readvalue;
int timer_check;
extern switch_is_on;
int logdatacmd(char **argv,unsigned short argc){ 
int e; 
int i=6; 

//start timer
switch_is_on=1;
ctl_events_set_clear(&handle_LaunchData,LAUNCHDATA_FLAG,0);//this should start the data logging 
start_timerA3();
//while((timer_check=getchar()) != 0x0D){//return key 
   /*  while(i>0){
               e=ctl_events_wait(CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR,&handle_adc,(1<<0),CTL_TIMEOUT_NONE,0);
                 if(e&(0x01)){
                   printf("A%d : %d\r\n",i,ADC10MEM0); 
                   i--;
                   }
             
               
               }*/
//stop_timerA3();
}

int stopdatacmd(char **argv,unsigned short argc){
  //stop_ADC_sampeling();
  stop_timerA3();
  switch_is_on=0;
}
unsigned short SDaddrhigh;
unsigned short SDaddrlow;
extern SDaddr; 
int SDaddrcmd(char **argv,unsigned short argc){
SDaddrhigh=SDaddr>>16; 
SDaddrlow=SDaddr; 
printf("SDhigh=%i, SDlow=%i\r\n",SDaddrhigh,SDaddrlow); 
printf("SD_ADDRESS=%lu\r\n",SDaddr);
}

//check if SD card SPI uses DMA
int mmcDMA_Cmd(char **argv, unsigned short argc){
  if(SD_DMA_is_enabled()){
    printf("DMA is enabled\r\n");
  }else{
    printf("DMA is not enabled\r\n");
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
int turnonsdcardCmd(char **argv, unsigned short argc){
          int mmcReturnValue;
          VREGon();//turn on voltage regulator in case the voltage on batteries drop, SD card can operate at 2.7 V, but the batteries will not stay there long
          ctl_timeout_wait(ctl_get_current_time()+5);//4.88 MS give time for voltage to stabalize 
          SENSORSon();//turn on sensors 
          //initalize the SD card 
          ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 
          initCLK();//SD card expects the 16 MHz clock 
          mmc_pins_on();
          mmcReturnValue=mmcInit_card();
          if (mmcReturnValue==MMC_SUCCESS){
          printf("\rCard initalized Sucessfully\r\n");
          }
          else {
          printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
          }

}

int mmcdat_Cmd(char **argv, unsigned short argc){
  int resp; 
  char *buffer=NULL;
  unsigned long sector=0,len=1;
  unsigned int check;
  int i,j,k,count=0;
  //check how many sectors can fit in the buffer
  const unsigned short buffsector=(BUS_get_buffer_size()/512);
  //check if sector given
  if(argc!=0){
    //read sector
    if(1!=sscanf(argv[1],"%lu",&sector)){
      //print error
      printf("Error : failed to parse sector \"%s\"\r\n",argv[1]);
      return -1;
    }
  }else{
      printf("Error : %s requires one or two arguments\r\n",argv[0]);
      return -1;
  }
  if(argc==2){
     if(1!=sscanf(argv[2],"%lu",&len)){
      //print error
      printf("Error : failed to parse length \"%s\"\r\n",argv[1]);
      return -1;
    } 
  }
  //get buffer, set a timeout of 2 secconds
  buffer=BUS_get_buffer(CTL_TIMEOUT_DELAY,2048);
  //check for error
  if(buffer==NULL){
    printf("Error : Timeout while waiting for buffer.\r\n");
    return -1;
  }
  for(i=0;i<len;i+=count){
      count=len-i;
      count=count>buffsector?buffsector:count;
      if(count==1){
          //read from SD card
          resp=mmcReadBlock(sector+i,(unsigned char*)buffer);
      }else{
        resp=mmcReadBlocks(sector+i,count,(unsigned char*)buffer);
      }
      //check if command was successful
      if(resp){
          printf("%s\r\n",SD_error_str(resp));
          //free buffer
          BUS_free_buffer();
          //return
          return resp;
      }
      for(j=0;j<count;j++){
          //print sector 
          printf("Sending MMC block %lu\r\n",sector+i+j);
          //initialize check
          check=0;
          //print out buffer
          for(k=0;k<512;k++){
            putchar(buffer[k+512*j]);
            check=check+buffer[k+512*j];
          }
          printf("Check =  %u\r\n",check);
      }
  }
  //free buffer
  BUS_free_buffer();
  return 0;
}


//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]",helpCmd},
                   {"ex","[arg1] [arg2] ...\r\n\t""Example command to show how arguments are passed",example_command},
                   {"timer_IR","[time]...\r\n\tExample command to show how the timer can be used as an interupt",example_timer_IR},
                   {"SD_write","Writes given args to the SD card",SD_write},
                   {"SD_read","",SD_read},
                   {"send_I2C","Sends I2C command to subsystem",send_I2C},
                   {"logdata","start logging data",logdatacmd},
                   {"stop","stop logging data",stopdatacmd},
                   {"print"," [command]\r\n\t""start to log data",printCmd},
                   {"mmce","start end\r\n\t""erase sectors from start to end",mmc_eraseCmd},
                   {"SDaddr","print out current memory card address\r\t",SDaddrcmd},
                   {"DMA","\r\n\t""Check if DMA is enabled.",mmcDMA_Cmd},
                   {"SDon","[command]\r\n\t""Turn on SD card",turnonsdcardCmd},
                   {"mmcdat","[sector]\r\n\t""read a sector from MMC card and send out raw data. Best used with accompanying Matlab function.",mmcdat_Cmd},
                 /*  {"mmcr","\r\n\t""read string from mmc card.",mmc_read},
                   {"mmcdump","[sector]\r\n\t""dump a sector from MMC card.",mmc_dump},
                   {"mmcw","[data,..]\r\n\t""write data to mmc card.",mmc_write},
                   {"mmcsize","\r\n\t""get card size.",mmc_cardSize},
                   {"mmctst","start end [seed]\r\n\t""Test by writing to blocks from start to end.",mmc_TstCmd},
                   {"mmcmw","start end [single|multi]\r\n\t""Multi block write test.",mmc_multiWTstCmd},
                   {"mmcmr","start end [single|multi]\r\n\t""Multi block read test.",mmc_multiRTstCmd},
                   {"mmcreinit","\r\n\t""initialize the mmc card the mmc card.",mmc_reinit},
                   
                   {"mmcreg","[CID|CSD]\r\n\t""Read SD card registers.",mmcreg_Cmd},
                   {"mmcinitchk","\r\n\t""Check if the SD card is initialized",mmcInitChkCmd},*/
                   ARC_COMMANDS,CTL_COMMANDS,ERROR_COMMANDS,
                   //end of list
                   {NULL,NULL,NULL}};

int mmcdat_Cmd(char **argv, unsigned short argc);