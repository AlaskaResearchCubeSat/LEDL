#ifndef __LOG_DATA_H
  #define __LOG_DATA_H
  #include <Error.h>
  #define TEMP_ARRAY_SIZE       7 //array is 7 16 byte column 
  #define LAUNCH_DATA_SIZE      512
  #define CLYDE_ARRAY_SIZE      20//we are only sampling 20 of the possible clyde sensors 
  #define MAG_ARRAY_SIZE        6//there are 6 magnetometers
  #define GYRO_ARRAY_SIZE       3//there are 3 gyroscopes 
  #define LEDL_DATA_ID          0x3311//this is specified by the data block definitions (launch data)
  #define LEDL_DETECT_ID        0x2121//this is launch_detect data

  #define LEDL_PACKETSIZE_FOR_CDH   15//this is specified by becon each spot is different length,
  #define CLYDE_PACKETSIZE_FOR_CDH  15//this is specified by becon each spot is 16 bits 
  #define LEDL_PACKETSIZE_FOR_ACDS  9//each packet has 6 mag and 3 gyro measurements
 
  #define SD_EV_WRITE_1         0x01
  #define SD_EV_WRITE_2         0x02 
  #define SD_EV_DIE             0x04
  #define SD_EV_FINISHED        0x08
  
  #define SD_EV_ALL             (SD_EV_WRITE_1|SD_EV_WRITE_2|SD_EV_DIE|SD_EV_FINISHED)
  
  #define LEDL_EV_GET_TEMP_DATA  0x01
  #define LEDL_EV_EPS_CMD        0x02
  #define LEDL_EV_ALL            (LEDL_EV_EPS_CMD|LEDL_EV_GET_TEMP_DATA)

  #define CLYDE_EV_GET_DATA     0x01
  #define LaunchData_flag       0x01
  #define OrbitData_flag        0x01
  
  //LEDL data starts after error data
  #define SD_LAUNCH_DATA_START  (ERR_ADDR_END+36)//this starts at 100
  #define SD_BECON_DATA         (ERR_ADDR_END+1)//start our beacon data at 65
  #define launch_didNot_happened 0xAA
  #define launch_happened        0x22
  #define MODE_DETECT            0
  #define MODE_LAUNCH            1
  #define MODE_ORBIT             2

  extern CTL_EVENT_SET_t handle_adc;
  extern CTL_EVENT_SET_t handle_SDcard;
  extern CTL_EVENT_SET_t handle_get_I2C_data; 
  extern CTL_EVENT_SET_t handle_get_mag_data; 
  extern CTL_EVENT_SET_t handle_get_clyde_data; 
  extern CTL_EVENT_SET_t handle_LaunchData;
  extern CTL_EVENT_SET_t handle_OrbitData;

  extern unsigned char remote_EPS_cmd[2];


  void launch_data_log(void *p); //USE WHEN USED AS A TASK 
  //void launch_data_log(void); // USE WHEN USED AS A FUNCITON 
  void ADC_test(void);
  void writedatatoSDcard(void);
  void takeI2Cdata(void);
  void takemagdata(void);
  void takeclydedata(void);
  void orbit_data_log(void *p);

  typedef struct{
      int ledl_address;
      union{
        struct{
          long SDaddress; 
          float accel_from_launch[4];//if I want to add any other data here I just need to add it and the size will always be the same due to the union
          float max_and_min_from_launch[6];
          int number_of_detect;
          int mode_status; 
          long SD_last_address;
       }detect_dat;
        unsigned char pad[508];
      }dat;
      unsigned short crc;
  }LEDL_TEST_LAUNCH;//to use this specify what my structure name is and  call it name.dat.detect_dat.SDaddress

  #endif
