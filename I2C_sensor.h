#ifndef __I2C_SENSOR_H
  #define __I2C_SENSOR_H


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

// Define Clyde Board Address
#define clyde_sensors     0x01


////////using the old clyde user guide for the following addresses, assumming Y1 is Y+ and Y2 is Y-, new eps has pair votlage, i am just choosing + side voltage 
///except for z which used the Z-

  #define EPS_Y_PLUS_CURRENT_ADDR     1
  #define EPS_Y_MINUS_CURRENT_ADDR    13
  #define EPS_Y_VOLTAGE_ADDR          0
  #define EPS_X_PLUS_CURRENT_ADDR     7
  #define EPS_X_MINUS_CURRENT_ADDR    4
  #define EPS_X_VOLTAGE_ADDR          6
  #define EPS_Z_MINUS_CURRENT_ADDR    31
  #define EPS_Z_VOLTAGE_ADDR          15
  #define EPS_BATTBUS_CURRENT         17
  #define EPS_5BUS_CURRENT            26
  #define EPS_3_3BUS_CURRENT          27
  #define EPS_BATT0_CURRENT           29
  #define EPS_BATT1_CURRENT           22
  #define EPS_BATT0_VOLTAGE           24
  #define EPS_BATT1_VOLTAGE           19               
  #define EPS_BATT_1_TEMP             18
  #define EPS_BATT_0_TEMP             23
  #define EPS_BATT_0_CURRENT_DIR      28
  #define EPS_BATT_1_CURRENT_DIR      21
 
  #define EPS_ADC_COMMAND             0
  #define EPS_STATUS_COMMAND          1
  #define EPS_GPIO_COMMAND            2
  #define EPS_PULSE_COMMAND           3
  #define EPS_VERSION_COMMAND         4
  #define EPS_HEATER_COMMAND          5
  #define EPS_GPIO_STATUS_COMMAND     6
  #define EPS_WATCHDOG_COMMAND        128



void Temp_I2C_sensor(int *array);
int clyde_take_data(int *array);






#endif
