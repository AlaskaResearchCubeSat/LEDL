#ifndef __I2C_sensor
  #define __I2C_sensor 


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

// Defined Magnetometer addresses
#define MAG_SENSORS       6
#define MAG_64_GAIN       0x85
#define MAG_1_GAIN        0x80
#define MAG_X_PLUS_ADDR        0x14
#define MAG_X_MINUS_ADDR       0x16
#define MAG_Y_PLUS_ADDR       0x26
#define MAG_Y_MINUS_ADDR       0x34
#define MAG_Z_PLUS_ADDR        0x25
#define MAG_Z_MINUS_ADDR       0x24

// Define Clyde Board Address
#define clyde_sensors     0x01

#ifndef __LTC24XX_H
#define __LTC24XX_H

#define LTC24XX_EN      0x20
#define LTC24XX_PRE     0x80
#define LTC24xx_SGL     0x10
#define LTC24xx_SIGN    0x08
#define LTC24xx_CH_0    0x00
#define LTC24xx_CH_1    0x01
#define LTC24xx_EN2     0x80
#define LTC24xx_IM      0x40
#define LTC24xx_FA      0x20
#define LTC24xx_FB      0x10
#define LTC24xx_SPD     0x08
#define LTC24xx_GS2     0x04
#define LTC24xx_GS1     0x02
#define LTC24xx_GS0     0x01

#define LTC24xx_GAIN1       (0)
#define LTC24xx_GAIN4       (LTC24xx_GS0)
#define LTC24xx_GAIN8       (LTC24xx_GS1)
#define LTC24xx_GAIN16      (LTC24xx_GS1|LTC24xx_GS0)
#define LTC24xx_GAIN32      (LTC24xx_GS2)
#define LTC24xx_GAIN64      (LTC24xx_GS2|LTC24xx_GS0)
#define LTC24xx_GAIN128     (LTC24xx_GS2|LTC24xx_GS1)
#define LTC24xx_GAIN264     (LTC24xx_GS2|LTC24xx_GS1|LTC24xx_GS0)

//address for all LTC24XX ADC's
#define LTC24XX_GLOBAL_ADDR 0x77

  //CA1    CA0    ADDRESS
  //LOW    LOW    0X14
  //LOW    HIGH   0X16
  //LOW    FLOAT  0x15
  //HIGH   LOW    0x26
  //HIGH   HIGH   0x34
  //HIGH   FLOAT  0x27
  //FLOAT  LOW    0x17
  //FLOAT  HIGH   0x25
  //FLOAT  FLOAT  0x24


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



#endif




void Temp_I2C_sensor(int *array);
int clyde_take_data(int *array);






#endif
