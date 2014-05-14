                                                                                             #include <msp430.h>
#include <string.h>
#include <stdio.h>
#include "I2C_sensor.h"
#include <i2c.h>

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
      printf("\r\nT%i = %i\r\n",i,tmp[0]);
      }
    else if (check_value_tx==-1)//returns a -1 if there is no connection to the I2C device
      {
      array[i]=500;//
      }
    else if (check_value_tx==0)//returns a zero if 
      {
      array[i]=1111;//
      }

  }

}
