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
#define MAG_X_PLUS        0x14
#define MAG_X_MINUS       0x16
#define MAG_Y_PLUS        0x26
#define MAG_Y_MINUS       0x34
#define MAG_Z_PLUS        0x25
#define MAG_Z_MINUS       0x24

// Define Clyde Board Address
#define clyde_sensors     0x01

void Temp_I2C_sensor(int *array);

#endif
