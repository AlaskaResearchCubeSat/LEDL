                                                                                             #include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "I2C_sensor.h"
#include "SensorsOnOff.h"
#include <commandLib.h>
#include <I2C.h>
#include <ARCbus.h>






/////magnetometer code taken from SPB test code from Z drive, these are based off of commands, changed for use in 

 //Defines for set reset pin
  #define MAG_SR_PIN         BIT4
  #define MAG_SR_OUT         P8OUT
  #define MAG_SR_DIR         P8DIR
  #define MAG_SR_SEL         P8SEL
  #define MAG_SR_REN         P8REN

#define MAX_NUM_ADDR  10

  
//gain of magnetomitor amplifier
//#define AMP_GAIN    (2.49e6/5.1e3)    // V/V0
#define AMP_GAIN    (1)    // V/V
//#define AMP_GAIN    (5.11e6/5.1e3)    // V/V
//sensitivity of magnetomitor
#define MAG_SENS    (1e-3)            // mV/V/Gauss

extern float ADCtoV(long adc);
extern float ADCtoGauss(long adc);
extern adc16Val(unsigned char *dat);
extern adc_ready_time;



  #define MAG_A_CH        LTC24xx_CH_1
  #define MAG_B_CH        LTC24xx_CH_0


extern unsigned short mag_ADC_gain;

//take a reading from the magnetometer ADC
extern short single_sample(unsigned short addr,long *dest);
extern gain_mag(char **argv,unsigned short argc);

//run magnetometer
//read data from the magnetometer and print to terminal
int mag_take_data(char **argv,unsigned short argc){
  unsigned short single=0,gauss=0,addr=0x14;
  unsigned char c,mag_addr=0x14;
  long result[2];
  float time=0;
  int i,res;
  MAGon();
  //parse arguments
  for(i=1;i<=argc;i++){
    if(!strcmp("single",argv[i])){
      single=1;
    }else if(!strcmp("gauss",argv[i])){
      gauss=1;
    }else if((addr=getI2C_addr(argv[i],0,magAddrSym))!=0xFF){
      mag_addr=addr;
    }else{
      
      printf("Error Unknown argument \'%s\'.\r\n",argv[i]);
      return -1;
    }
  }
  //run until abort is detected
  do{
    res=single_sample(mag_addr,result);
    if(res!=0){
      printf("Error encountered. Aborting\r\n");
      break;
    }
    if(gauss){
      printf("%f %f\r\n",ADCtoGauss(result[0])/2,ADCtoGauss(result[1])/2);
    }else{
      printf("%li %li\r\n",result[0],result[1]);
    }
    c=async_CheckKey();
  }while(!(c==0x03  || c=='Q' || c=='q' || single||c==' '));
  return 0;
  MAGoff();
}
extern adc_mag(char **argv, unsigned short argc);


extern const char *(addr_name_mag[]);
extern const char name_addrs_mag[];

extern int SR_mag(char **argv, unsigned short argc);

//// end of SPB test code 
////begin of temperature data code 


void Temp_I2C_sensor(int *array)
{

int i;
short check_value_tx;
short check_value_rx;
unsigned char tmp[2];
const unsigned char temparray[1]={REG_TEMP_VALUE};// const due to not changing the value, char its a one byte element 
// this is so I can have my address in memory and create a pointer to point to it for the function
const unsigned char all_temp_sensors[TEMP_SENSORS]={TEMP_X_PLUS, TEMP_X_MINUS,TEMP_Y_PLUS, TEMP_Y_MINUS, TEMP_Z_PLUS, TEMP_Z_MINUS, TEMP_L_BOARD};


// Measure all temperature sensors at once. 
for (i=0;i<TEMP_SENSORS;i++)
  {
  check_value_tx = i2c_tx(all_temp_sensors[i],temparray,1);//place the address I want to talk to, 
  //place the pointer to the array that stores my address, specify the number of bytes being sent
   //printf("\r\nSensor %i %s",i,I2C_error_str(check_value_tx));
    if (check_value_tx==1)
      {
      check_value_rx = i2c_rx(all_temp_sensors[i],tmp,2);//place the address I want to talk to, 
      //the array that stores my data, number of bytes being sent
      array[i]=tmp[0];
      //printf("\r\nT%i = %i\r\n",i,tmp[0]);
      ctl_timeout_wait(ctl_get_current_time()+1000);
      
      }
    else if (check_value_tx==-1)//returns a -1 if there is no connection to the I2C device
      {
      array[i]=500;//
      printf("\r\nT%i = %i\r\n",i,tmp[0]);
      }
    else if (check_value_tx==0)//returns a zero if 
      {
      array[i]=1111;//
      }

  }

}

/////////////////begin of eps code///////////////////////////////////////////////////////////


const char name_addrs_clyde[]={EPS_Y_PLUS_CURRENT_ADDR,
                               EPS_Y_MINUS_CURRENT_ADDR,
                               EPS_Y_PLUS_VOLTAGE_ADDR,
                               EPS_X_PLUS_CURRENT_ADDR,
                               EPS_X_MINUS_CURRENT_ADDR,
                               EPS_X_PLUS_VOLTAGE_ADDR,
                               EPS_Z_MINUS_CURRENT_ADDR,
                               EPS_Z_MINUS_VOLTAGE_ADDR,
                               EPS_BATTBUS_CURRENT,
                               EPS_5BUS_CURRENT,
                               EPS_3_3BUS_CURRENT,
                               EPS_BATT0_CURRENT,
                               EPS_BATT1_CURRENT,
                               EPS_BATT0_VOLTAGE,
                               EPS_BATT1_VOLTAGE,
                               EPS_BATT_1_TEMP,
                               EPS_BATT_0_TEMP,
                               EPS_BATT_0_CURRENT_DIR,
                               EPS_BATT_1_CURRENT_DIR,
                               };


//read write cmd for CLYDE
int clyde_take_data(int *array){
  int res,i,found=0;
  unsigned char tx[2],rx[2];
  unsigned short rez;

  //Set
  //printf("sent cmd \r\n");
  //send cmd\
  //take first 21 adc measurements for beacon 
  for(i=0;i<20;i++){
    tx[0]=EPS_ADC_COMMAND;
    tx[1]= name_addrs_clyde[i];
    res=i2c_tx(clyde_sensors,tx,2);
    //error msg or success
    if(res<0){
      //printf("tx returned = %s\r\n",I2C_error_str(res));
    }
    //wait 1.2 ms (300)
    ctl_timeout_wait(ctl_get_current_time()+5);
    //read cmd
    res=i2c_rx(clyde_sensors,rx,2);
    //error msg or success
    if (res<0){
      //printf("rx returned = %s\r\n",I2C_error_str(res));
    }
    rez=rx[1];
    rez|=rx[0]<<8;
    rez&=0x3FF;
    //printf("rez = %i\r\n",rez);
    array[i]=(int)rez;
    //printf("EPS data = %i, array # = %i\r\n",array[i],i);
  }

  //take status packet for beacon
  tx[0]=EPS_STATUS_COMMAND;
  tx[1]=0;//THIS DOESNT MATTER WHAT IT IS 
  res=i2c_tx(clyde_sensors,tx,2);
  //error msg or success
  //printf("%s\r\n",I2C_error_str(res));
  //wait 1.2 ms (300)
  ctl_timeout_wait(ctl_get_current_time()+5);
  //read cmd
  res=i2c_rx(clyde_sensors,rx,2);
  //error msg or success
  if(res!=0){
    //printf("%s\r\n",I2C_error_str(res));
  }
  rez=rx[1];
  rez|=rx[0]<<8;

  printf("rez = 0x%04X\r\n",rez);
  array[i]=(int)rez;
  //return success
  return RET_SUCCESS;
}
