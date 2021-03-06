#ifndef __I2C_SENSOR_H
  #define __I2C_SENSOR_H
  #include <ctl.h>

// Defined temperature addresses

#define TEMP_SENSORS      7
#define TEMP_X_PLUS       0x48
#define TEMP_X_MINUS      0x4A
#define TEMP_Y_PLUS       0x49
#define TEMP_Y_MINUS      0x4C
#define TEMP_Z_PLUS       0x4E
#define TEMP_Z_MINUS      0x4D
#define TEMP_L_BOARD      0x4F

// Defined temperature registers
#define REG_TEMP_VALUE    0x00
#define REG_CONFIG        0x01
#define REG_TEMP_HYST     0x02
#define REG_TEMP_OIT      0x03


//possible addresses for EPS
#define EPS_ADDR_V1     (0x01)
#define EPS_ADDR_V2     (0x2D)

// Define Clyde Board Address
#define EPS_I2C_ADDRESS    EPS_ADDR_V1


////////using the old clyde user guide for the following addresses, assumming Y1 is Y+ and Y2 is Y-, new eps has pair votlage, i am just choosing + side voltage 
///except for z which used the Z-
//New clyde data on the left, old clyde data addresses on right

  #define EPS_Y_PLUS_CURRENT_ADDR     /*1*/1
  #define EPS_Y_MINUS_CURRENT_ADDR    /*13*/13
  #define EPS_Y_PLUS_VOLTAGE_ADDR     /*3*/0
  #define EPS_Y_MINUS_VOLTAGE_ADDR    12
  #define EPS_X_PLUS_CURRENT_ADDR     /*7*/7
  #define EPS_X_MINUS_CURRENT_ADDR    /*4*/4
  #define EPS_X_PLUS_VOLTAGE_ADDR     /*6*/6
  #define EPS_X_MINUS_VOLTAGE_ADDR    3
  #define EPS_Z_MINUS_CURRENT_ADDR    /*31*/31
  #define EPS_Z_MINUS_VOLTAGE_ADDR     /*9*/15
  #define EPS_BATTBUS_CURRENT         /*17*/17
  #define EPS_5BUS_CURRENT            /*26*/26
  #define EPS_3_3BUS_CURRENT          /*27*/27
  #define EPS_BATT0_CURRENT           /*29*/29
  #define EPS_BATT1_CURRENT           /*22*/22
  #define EPS_BATT0_VOLTAGE           /*24*/24
  #define EPS_BATT1_VOLTAGE           /*19*/19               
  #define EPS_BATT_1_TEMP             /*18*/18
  #define EPS_BATT_0_TEMP             /*23*/23
  #define EPS_BATT_0_CURRENT_DIR      /*28*/28
  #define EPS_BATT_1_CURRENT_DIR      /*21*/21
 
 //below are command summary of the clyde eps, the commented out ones are not on the new eps
  #define EPS_ADC_COMMAND             0
  #define EPS_STATUS_COMMAND          1
  #define EPS_GPIO_COMMAND            2
  #define EPS_PDM_OFF_COMMAND         2
  #define EPS_PULSE_COMMAND           3
  #define EPS_VERSION_COMMAND         4
  #define EPS_HEATER_COMMAND          5
  #define EPS_GPIO_STATUS_COMMAND     6
  #define EPS_WATCHDOG_COMMAND        128

//mutex to lock control of the EPS
extern CTL_MUTEX_t EPS_mutex;


void Temp_I2C_sensor(int *array);
int clyde_take_data(int *array);






#endif
