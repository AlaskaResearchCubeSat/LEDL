#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "ARCbus.h"
#include <terminal.h>
#include "log_data.h"
#include <SDlib.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <commandLib.h>
#include <I2C.h>
#include "I2C_sensor.h"
#include <ctl.h>
#include "SensorsOnOff.h"
#include "sensor-interface.h"
#include "Z:\Software\ADCS\ACDS-flight\SensorDataInterface.h"
#include "LaunchDetect.h"








//this code is a receptacle for getting commands from the user. 
void commands(void);

int SUB_parseCmd(unsigned char src,unsigned char cmd, unsigned char *dat, unsigned short len){
int i;
  unsigned short time,count;
  switch(cmd){
    case CMD_MAG_SAMPLE_CONFIG:
      //check command
      switch(*dat){
        case MAG_SAMPLE_START:
          if(len!=4){
            return ERR_PK_LEN;
          }
          //get time parameter
          time=dat[1]<<8;
          time|=((unsigned short)dat[2]);
          count=dat[3];
          //run sensor collection
          run_sensors(time,count);
          //success!
          return RET_SUCCESS;
        case MAG_SAMPLE_STOP:
          //check packet length
          if(len!=1){
            //incorrect length
            return ERR_PK_LEN;
          }
          //stop sensors
          stop_sensors();
          //success!
          return RET_SUCCESS;
        case MAG_SINGLE_SAMPLE:
          //check packet length
          if(len!=1){
            //incorrect length
            return ERR_PK_LEN;
          }
          sensors_single_sample();
          //success!
          return RET_SUCCESS;
        case MAG_TEST_MODE_ON:
          //check packet length
          if(len!=1){
            //incorrect length
            return ERR_PK_LEN;
          }
          mag_tx_addr=BUS_ADDR_CDH;
          return RET_SUCCESS;  
        case MAG_TEST_MODE_OFF:
          //check packet length
          if(len!=1){
            //incorrect length
            return ERR_PK_LEN;
          }
          mag_tx_addr=BUS_ADDR_ACDS;
          return RET_SUCCESS;
      }
  }
  //Return Error
  return ERR_UNKNOWN_CMD;
}

//////////////////////////////////////////////////////////////////////////////////////////
int logdataCmd(char **argv, unsigned short argc){
 
  // launch_data_log();//comment this out when using as a task and I am not calling it as a function 
   }
//////////////////////////////////////////////////////////////////////////////////////////
//define printf formats
#define HEXOUT_STR    "%02X "
#define INTERGER_STR  "%d "
#define ASCIIOUT_STR  "%c "
#define UNSIGNED_STR  "%u "

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
/////////////////////////////////////////////////////////////////////////////////////////////
int mmc_eraseCmd(char **argv, unsigned short argc){
  unsigned long start,end;
  int resp;
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

int printCmd(char **argv, unsigned short argc){//copied print mmcdump command to change the format of how the data is presented in tera term. 
  int resp; 
  int *buffer=NULL;//changed from char to int instead of each char being 1 byte their are now two bytes, resulting in being half as many things to index 
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
  for(i=0;i<256/26;i++){//changed the 512 to 256 which is a result of changing CHAR TO INT

  if(i<8){
    printf(INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR "\r\n",
    buffer[i*26],buffer[i*26+1],buffer[i*26+2],buffer[i*26+3],buffer[i*26+4],buffer[i*26+5],buffer[i*26+6],buffer[i*26+7],buffer[i*26+8],buffer[i*26+9],buffer[i*26+10],
    buffer[i*26+11],buffer[i*26+12],buffer[i*26+13],buffer[i*26+14],buffer[i*26+15],buffer[i*26+16],buffer[i*26+17],buffer[i*26+18],buffer[i*26+19],buffer[i*26+20],
    buffer[i*26+21],buffer[i*26+22],buffer[i*26+23],buffer[i*26+24],buffer[i*26+25]);
    }

  else{
    printf(INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR UNSIGNED_STR UNSIGNED_STR "\r\n",
    buffer[i*26],buffer[i*26+1],buffer[i*26+2],buffer[i*26+3],buffer[i*26+4],buffer[i*26+5],buffer[i*26+6],buffer[i*26+7],buffer[i*26+8],buffer[i*26+9],buffer[i*26+10],
    buffer[i*26+11],buffer[i*26+12],buffer[i*26+13],buffer[i*26+14],buffer[i*26+15],buffer[i*26+16],buffer[i*26+17],buffer[i*26+18],buffer[i*26+19],buffer[i*26+20],
    buffer[i*26+21],buffer[i*26+22],buffer[i*26+23],buffer[i*26+24],buffer[i*26+25],buffer[i*26+26]);
    }
   ctl_timeout_wait(ctl_get_current_time()+10); 
  }
  //free buffer
  BUS_free_buffer();
  
  return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
int printGyroCmd(char **argv, unsigned short argc){//copied print mmcdump command to change the format of how the data is presented in tera term. 
  int resp; 
  int *buffer=NULL;//changed from char to int instead of each char being 1 byte their are now two bytes, resulting in being half as many things to index 
  unsigned long sector=0;
  int i;
  //set up gryo offsets 
  int gyroxindex=24;
  int gyrozindex=50;
  int gyroyindex=76;
  int gyroincrease=26;
  int gyrox[3];
  int gyroz[3];
  int gyroy[3];
  

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
  

  //store all gryo data into 3 different arrays (G1=x, G2=z, G3=y) 

  //get gyro data from buffer 
  for(i=0;i<3;i++)
  {
  gyrox[i]=buffer[gyroxindex+gyroincrease*i*3];
  gyroz[i]=buffer[gyrozindex+gyroincrease*i*3];
  gyroy[i]=buffer[gyroyindex+gyroincrease*i*3];
  }
  
  //print gryo data received from buffer 
  
   printf("gyro x \r\n" INTERGER_STR INTERGER_STR INTERGER_STR"\r\n",gyrox[0],gyrox[1],gyrox[2]);
   printf("gyro z \r\n" INTERGER_STR INTERGER_STR INTERGER_STR"\r\n",gyroz[0],gyroz[1],gyroz[2]);
   printf("gyro y \r\n" INTERGER_STR INTERGER_STR INTERGER_STR"\r\n",gyroy[0],gyroy[1],gyroy[2]);
/*
  if(i<7){
    printf(INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR "\r\n",
    buffer[i*26],buffer[i*26+1],buffer[i*26+2],buffer[i*26+3],buffer[i*26+4],buffer[i*26+5],buffer[i*26+6],buffer[i*26+7],buffer[i*26+8],buffer[i*26+9],buffer[i*26+10],
    buffer[i*26+11],buffer[i*26+12],buffer[i*26+13],buffer[i*26+14],buffer[i*26+15],buffer[i*26+16],buffer[i*26+17],buffer[i*26+18],buffer[i*26+19],buffer[i*26+20],
    buffer[i*26+21],buffer[i*26+22],buffer[i*26+23],buffer[i*26+24],buffer[i*26+25]);
    }

  else{
    printf(INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR UNSIGNED_STR "\r\n",
    buffer[i*26],buffer[i*26+1],buffer[i*26+2],buffer[i*26+3],buffer[i*26+4],buffer[i*26+5],buffer[i*26+6],buffer[i*26+7],buffer[i*26+8],buffer[i*26+9],buffer[i*26+10],
    buffer[i*26+11],buffer[i*26+12],buffer[i*26+13],buffer[i*26+14],buffer[i*26+15],buffer[i*26+16],buffer[i*26+17],buffer[i*26+18],buffer[i*26+19],buffer[i*26+20],
    buffer[i*26+21],buffer[i*26+22],buffer[i*26+23],buffer[i*26+24],buffer[i*26+25]);
    }
    */
   ctl_timeout_wait(ctl_get_current_time()+10); 
  
  //free buffer
  BUS_free_buffer();
  
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////

int printmultiCmd(char **argv, unsigned short argc){//copied print mmcdump command to change the format of how the data is presented in tera term. 
  int resp; 
  int *buffer=NULL;//changed from char to int instead of each char being 1 byte their are now two bytes, resulting in being half as many things to index 
  unsigned long sector=0;
  int i;
  int j;
  int available;
 

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
  //find out how many data points can be sampled
  available=BUS_get_buffer_size()/512;//BUS_get_buffer_size returns the size of the memory block in bytes divided by the 
  //size of a memory block, this will provide the number of blocks that will be read from memory. 
  //read from SD card
  resp=mmcReadBlocks(sector,available,(unsigned char*)buffer);
  //print response from SD card
  printf("%s\r\n",SD_error_str(resp));
  
  

  for(j=0;j<available;j++)
  {
 
  for(i=0;i<256/26;i++){//changed the 512 to 256 which is a result of changing CHAR TO INT

  if(i<7){
    printf(INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR "\r\n",
    buffer[i*26+j*256],buffer[i*26+j*256+1],buffer[i*26+j*256+2],buffer[i*26+j*256+3],buffer[i*26+j*256+4],buffer[i*26+j*256+5],buffer[i*26+j*256+6],buffer[i*26+j*256+7],buffer[i*26+j*256+8],buffer[i*26+j*256+9],buffer[i*26+j*256+10],
    buffer[i*26+j*256+11],buffer[i*26+j*256+12],buffer[i*26+j*256+13],buffer[i*26+j*256+14],buffer[i*26+j*256+15],buffer[i*26+j*256+16],buffer[i*26+j*256+17],buffer[i*26+j*256+18],buffer[i*26+j*256+19],buffer[i*26+j*256+20],
    buffer[i*26+j*256+21],buffer[i*26+j*256+22],buffer[i*26+j*256+23],buffer[i*26+j*256+24],buffer[i*26+j*256+25]);
    }

  else{
    printf(INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR UNSIGNED_STR UNSIGNED_STR"\r\n",
    buffer[i*26+j*256],buffer[i*26+j*256+1],buffer[i*26+j*256+2],buffer[i*26+j*256+3],buffer[i*26+j*256+4],buffer[i*26+j*256+5],buffer[i*26+j*256+6],buffer[i*26+j*256+7],buffer[i*26+j*256+8],buffer[i*26+j*256+9],buffer[i*26+j*256+10],
    buffer[i*26+j*256+11],buffer[i*26+j*256+12],buffer[i*26+j*256+13],buffer[i*26+j*256+14],buffer[i*26+j*256+15],buffer[i*26+j*256+16],buffer[i*26+j*256+17],buffer[i*26+j*256+18],buffer[i*26+j*256+19],buffer[i*26+j*256+20],
    buffer[i*26+j*256+21],buffer[i*26+j*256+22],buffer[i*26+j*256+23],buffer[i*26+j*256+24],buffer[i*26+j*256+25],buffer[i*26+j*256+26]);
    }
    
   ctl_timeout_wait(ctl_get_current_time()+10); 
  }
  }
  printf("The number of Memory Blocks printed was %d",available);
  //free buffer
  BUS_free_buffer();
  
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
//return memory card to address zero
extern int SDaddr;
int mmc_address_zeroCmd(char **argv, unsigned short argc){
SDaddr=0;
if (SDaddr==0)
{printf("sucess memory starting at Zero");}
ctl_timeout_wait(ctl_get_current_time()+10); 
} 
/////////////////////////////////////////////////////////////////////////////////////////////////
//return memory card address to be read 
extern int SDaddr;
int mmc_address_Cmd(char **argv, unsigned short argc){
{printf("Memory is at %d",SDaddr);}
ctl_timeout_wait(ctl_get_current_time()+10); 
} 

////////////////////////////////////////////////////////////////////////////////////////////////
int testlaunchCmd(char **argv, unsigned short argc){
 
   VerifyLaunchDetect();
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
          mmcInit_msp();
          mmcReturnValue=mmcInit_card();
          if (mmcReturnValue==MMC_SUCCESS){
          printf("\rCard initalized Sucessfully\r\n");
          }
          else {
          printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
          }

}
////////////////////////////////////////////////////////////////////////////////////////////////
int turnoffsdcardCmd(char **argv, unsigned short argc){
       mmcInit_msp_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWR LINE 
       SENSORSoff();
       initCLK_lv();//Reduce clock speed for low voltage application
       ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 
       VREGoff();//turn Voltage regulator off for low power application
}
////////////////////////////////////////////////////////////////////////////////////////////////
 //EPS telemetry
  int battempcmd(char **argv,unsigned short arg){
  
}
////////////////////////////////////////////////////////////////////////////////////////////////

const struct{
  float scale,offset;
  const char *name,*units;
} clyde_tbl[32]={
//New Clyde Conversions
/*
{1,0,"GND",""},
{-.5,515.7,"Array +Y Current","mA"},
{-.163,110.338,"Array +Y Temperature","\xF8""C"},
{-.0086,8.81,"Array +y Temperature","V"},
{-.5,515.7,"Array pair Y voltage","mA"},
{-.163,110.338,"Array -X temperature","\xF8""C"},
{-.0086,8.81,"Array pair X voltage","V"},
{-.5,515.7,"Array +X current","mA"},
{-.163,110.338,"Array +x temperature","\xF8""C"},
{-.0086,8.81,"Array pair Z voltage","V"},
{-.5,515.7,"Array +Z current","mA"},
{-.163,110.338,"Array +z temprature","\xF8""C"},
{1,0,"GND",""},
{-.5,515.7,"Array -Y current","mA"},
{-.163,110.338,"Array -Y temperature","\xF8""C"},
{1,0,"GND",""},
{1,0,"GND",""},
{-3.153,3250.815,"Battery Bus Current","mA"},
{-.163,110.835,"BAT1 Temperature","\xF8""C"},
{-.0939,9.791,"BAT1 Full Voltage","V"},
{1,0,"GND",""},
{1,0,"BAT1 Current Direction","Chg/Dis NEEDS SCALE FACTORS"},
{-3.2,2926.22,"BAT1 Current","mA"},
{-.163,110.835,"BAT0 Temperature","\xF8""C"},
{-.0939,9.791,"BAT0 Full Voltage","V"},
{1,0,"GND",""},
{-3.5,3611.509,"5V Bus Current","mA"},
{-4.039,4155.271,"3.3V Bus Current","mA"},
{1,0,"BAT0 Current Direction","Chg/Dis NEEDS SCALE FACROTS"},
{-3.2,2926.22,"BAT0 Currnet Direction","mA"},
{-.163,110.338,"Array -Z Temperature","\xF8""C"},
{-.5,515.7,"Array -Z Current","mA",},
*/
//Old Clyde Conversions
{-.009,8.073,"Panel Y1 Voltage","V"},
{-.486,502.524,"Panel Y1 Current","mA"},
{-.1619,110.119,"Panel Y1 Temperature","\xF8""C"},
{-.009,8.703,"Panel X2 voltage","V"},
{-.486,502.524,"Panel X2 current","mA"},
{-.1619,110.119,"Panel X2 temperature","\xF8""C"},
{-.009,8.703,"Panel X1 Voltage","V"},
{-.486,502.524,"Panel X1 current","mA"},
{-.1619,110.119,"Panel X1 temperature","\xF8""C"},
{-.009,8.703,"Panel Z1 Voltage","V"},
{-.486,502.524,"Panel Z1 current","mA"},
{-.1619,110.119,"Panel Z1 temperature","\xF8""C"},
{-.009,8.703,"Panel Y2 Voltage","V"},
{-.486,502.524,"Panel Y2 Current","mA"},
{-.1619,110.119,"Panel Y2 Temperature","\xF8""C"},
{-.009,8.703,"Panel Z2 Voltage","V"},
{1,0,"GND",""},
{-2.13551,2422.004,"Battery Bus Current","mA"},//17
{-.163,110.7,"BAT2 Temperature","\xF8""C"},
{-.00939,9.791,"BAT2 Voltage","V"},
{-.00438,4.753,"CELL2 Voltage","V"},
{1,0,"BAT2 Current Direction","Chg/Dis NEEDS SCALE FACROTS"},
{-3.2,2926.22,"BAT2 Current","mA"},
{-.163,110.7,"BAT1 Temperature","\xF8""C"},
{-.00939,9.791,"BAT1 Voltage","V"},//24
{-.00438,4.753,"CELL1 Voltage","V"},//25
{-1.2009,1239.284,"5V Bus Current","mA"},//26
{-1.3009,1334.687,"3.3V Bus Current","mA"},//27
{1,0,"BAT1 Current Direction","Chg/Dis NEEDS SCALE FACROTS"},
{-3.2,2926.22,"BAT1 Current","mA"},
{-.1619,110.119,"Panel Z1 temperature","\xF8""C"},
{-.486,502.524,"Panel Z1 current","mA"},
};



//read write cmd for CLYDE
int clydecmd(char **argv,unsigned short argc){
int res,i,found=0;
unsigned char tx[2]={0x00,19}, rx[2];
unsigned short rez;
//TURN ON I2C LINE FOR CLYDE 
P7DIR |= BIT4;
P7OUT |= BIT4;
//Set

ctl_timeout_wait(ctl_get_current_time()+3);
// check num of arguments  
if (argc!=1){
  printf("Enter an ADC address from 1 to 31 \r\n");
  return 1;
}
// convert **argv string to int
tx[1]=atoi(argv[1]);
// check to make sure ADC addressing value is btwn 0 and 31
if(tx[1]>31){
printf("Error enterd channel value was %d\r\n Enter an ADC address from 0 to 31.\r\n",tx[1]);
return 2;}

printf("sent cmd \r\n");
//send cmd\

res=i2c_tx(0x01,tx,2);
//error msg or success
printf("%s\r\n",I2C_error_str(res));
//wait 1.2 ms (300)
ctl_timeout_wait(ctl_get_current_time()+5);
//read cmd
res=i2c_rx(0x01,rx,2);
//error msg or success
printf("%s\r\n",I2C_error_str(res));
rez=rx[1];
rez|=rx[0]<<8;
rez&=0x3FF;
printf("rez = %i\r\n",rez);
// ADC channel signal conversion equations
printf("%s = %f %s\r\n",clyde_tbl[tx[1]].name,clyde_tbl[tx[1]].scale*rez+clyde_tbl[tx[1]].offset,clyde_tbl[tx[1]].units);
//turn LED off
ctl_timeout_wait(ctl_get_current_time()+3);

//TURN OFF I2C LINE FOR CLYDE 
P7OUT&=~BIT4;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
int pdmcmd(char **argv,unsigned short argc){
unsigned char tx[2]={0x02,BIT2};
int res;

//send cmd
res=i2c_tx(0x01,tx,2);  
printf("PDM reset \r\n");
// LED off

}
int versioncmd(char **argv,unsigned short argc){
unsigned char tx[2]={0x02,BIT2},rx[2];
int res;

//send cmd
res=i2c_tx(0x01,tx,2);
//wait a bit
ctl_timeout_wait(ctl_get_current_time()+3);
//read cmd
res=i2c_rx(0x01,rx,2);
//convert string to int
//res=atoi(argv[1]);
printf("version number is %d\n\r",res);

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
int tempcmd(char **argv,unsigned short argc){
int temp_measure[7];



Temp_I2C_sensor(temp_measure);
ctl_timeout_wait(ctl_get_current_time()+2000);

}


////////////////////////////////////////////////////////////////////////////////////////////////

//created table for help commands
//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]\r\n\t""get a list of commands or help on a spesific command.",helpCmd},
                          {"logdata"," [command]\r\n\t""start to log data",logdataCmd},
                          {"print"," [command]\r\n\t""start to log data",printCmd},
                          {"printmulti"," [command]\r\n\t""start to log data",printmultiCmd}, 
                          {"printgyro"," [command]\r\n\t""start to log data",printGyroCmd},
                          {"mmczero"," [command]\r\n\t""return memeory card to address zero",mmc_address_zeroCmd},
                          {"mmcaddr"," [command]\r\n\t""return memeory card to address zero",mmc_address_Cmd},
                          {"testlaunch","[command]\r\n\t""check to see if interrupt works",testlaunchCmd},
                          {"SDon","[command]\r\n\t""Turn on SD card",turnonsdcardCmd},
                          {"SDoff","[command]\r\n\t""Turn on SD card",turnoffsdcardCmd},
                          {"battemp","addr num [data0]\r\n\t gets CLYDE battery temperature.",battempcmd},
                          {"clyde","channel \r\n\t""gets data from Clyde ADC channels.\n\r\t",clydecmd},
                          {"pdm","Hard reset switches power busses off.\r\n\t",pdmcmd},
                          {"version","provieds firmware version number of the CLYDE board\n\r\t",versioncmd},
                          {"temp","provides temperature measurement\n\r\t",tempcmd},
                          MMC_COMMANDS,CTL_COMMANDS,I2C_COMMANDS,
                         //end of list
                         {NULL,NULL,NULL}};


//code that associates with commands and what needs to be executed 







//Error Code Section 


char *err_decode(char buf[150], unsigned short source,int err, unsigned short argument){

  sprintf(buf,"source = %i, error = %i, argument = %i",source,err,argument);
  return buf;
}


