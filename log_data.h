#ifndef __LOG_DATA_H
  #define __LOG_DATA_H
  #define TEMP_ARRAY_SIZE       7 //array is 7 16 byte column 
  #define LAUNCH_DATA_SIZE      512
  #define CLYDE_ARRAY_SIZE      20//we are only sampling 20 of the possible clyde sensors 
  #define MAG_ARRAY_SIZE        6//there are 6 magnetometers
  #define GYRO_ARRAY_SIZE       3//there are 3 gyroscopes 
  #define LEDL_ID               0x3311//this is specified by the data block definitions

  #define LEDL_PACKETSIZE_FOR_CDH   15//this is specified by becon each spot is different length,
  #define CLYDE_PACKETSIZE_FOR_CDH  15//this is specified by becon each spot is 16 bits 
  #define LEDL_PACKETSIZE_FOR_ACDS  9//each packet has 6 mag and 3 gyro measurements
 
  #define SDcardwriteflag1      0x01
  #define SDcardwriteflag2      0x02 
  #define get_I2C_data_flag     0x01
  #define get_mag_data_flag     0x01
  #define get_clyde_data_flag   0x01
  #define LaunchData_flag       0x01
  #define OrbitData_flag        0x01
  #define SDcardDieflag         0x04
  #define SDcardfinishedflag    0x08

  extern CTL_EVENT_SET_t handle_adc;
  extern CTL_EVENT_SET_t handle_SDcard;
  extern CTL_EVENT_SET_t handle_get_I2C_data; 
  extern CTL_EVENT_SET_t handle_get_mag_data; 
  extern CTL_EVENT_SET_t handle_get_clyde_data; 
  extern CTL_EVENT_SET_t handle_LaunchData;
  extern CTL_EVENT_SET_t handle_OrbitData;


  void launch_data_log(void *p); //USE WHEN USED AS A TASK 
  //void launch_data_log(void); // USE WHEN USED AS A FUNCITON 
  void ADC_test(void);
  void writedatatoSDcard(void);
  void takeI2Cdata(void);
  void takemagdata(void);
  void takeclydedata(void);
  void orbit_data_log(void *p);

  #endif
