#include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "uart.h"
#include "ARCbus.h"
#include <terminal.h>
#include "log_data.h"
#include <SDlib.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <commandLib.h>

//this code is a receptacle for getting commands from the user. 
void commands(void);

int SUB_parseCmd(unsigned char src,unsigned char cmd, unsigned char *dat, unsigned short len){
return ERR_UNKNOWN_CMD;
}

//////////////////////////////////////////////////////////////////////////////////////////
int logdataCmd(char **argv, unsigned short argc){
 
   launch_data_log();
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
    printf(INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR INTERGER_STR UNSIGNED_STR "\r\n",
    buffer[i*26+j*256],buffer[i*26+j*256+1],buffer[i*26+j*256+2],buffer[i*26+j*256+3],buffer[i*26+j*256+4],buffer[i*26+j*256+5],buffer[i*26+j*256+6],buffer[i*26+j*256+7],buffer[i*26+j*256+8],buffer[i*26+j*256+9],buffer[i*26+j*256+10],
    buffer[i*26+j*256+11],buffer[i*26+j*256+12],buffer[i*26+j*256+13],buffer[i*26+j*256+14],buffer[i*26+j*256+15],buffer[i*26+j*256+16],buffer[i*26+j*256+17],buffer[i*26+j*256+18],buffer[i*26+j*256+19],buffer[i*26+j*256+20],
    buffer[i*26+j*256+21],buffer[i*26+j*256+22],buffer[i*26+j*256+23],buffer[i*26+j*256+24],buffer[i*26+j*256+25]);
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

////////////////////////////////////////////////////////////////////////////////////////////////
int testlaunchCmd(char **argv, unsigned short argc){
 
   VerifyLaunchDetect();
   }

/////////////////////////////////////////////////////////////////////////////////////////////////


//created table for help commands
//table of commands with help
const CMD_SPEC cmd_tbl[]={{"help"," [command]\r\n\t""get a list of commands or help on a spesific command.",helpCmd},
                          {"logdata"," [command]\r\n\t""start to log data",logdataCmd},
                          {"print"," [command]\r\n\t""start to log data",printCmd},
                          {"printmulti"," [command]\r\n\t""start to log data",printmultiCmd}, 
                          {"printgyro"," [command]\r\n\t""start to log data",printGyroCmd},
                          {"mmczero"," [command]\r\n\t""return memeory card to address zero",mmc_address_zeroCmd},
                          {"testlaunch","[command]\r\n\t""check to see if interrupt works",testlaunchCmd},
                          MMC_COMMANDS,CTL_COMMANDS,
                         //end of list
                         {NULL,NULL,NULL}};


//code that associates with commands and what needs to be executed 







//Error Code Section 


char *err_decode(char buf[150], unsigned short source,int err, unsigned short argument){

  sprintf(buf,"source = %i, error = %i, argument = %i",source,err,argument);
  return buf;
}


