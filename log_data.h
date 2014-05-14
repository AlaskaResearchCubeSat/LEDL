#ifndef __log_data_launch
  #define __log_data_launch
  #define TEMP_ARRAY_SIZE   7 //array is 7 16 byte column 
  #define LAUNCH_DATA_SIZE  512
  #define SDcardwriteflag1  0x01
  #define SDcardwriteflag2  0x02 
  #define get_I2C_data_flag 0x01

  extern CTL_EVENT_SET_t handle_adc;
  extern CTL_EVENT_SET_t handle_SDcard;
  extern CTL_EVENT_SET_t handle__get_I2C_data; 


  //void launch_data_log(void *p); //USE WHEN USED AS A TASK 
  void launch_data_log(void); // USE WHEN USED AS A FUNCITON 
  void ADC_test(void);
  void writedatatoSDcard(void);
  void takeI2Cdata(void);
  void orbit_data_log(void);

  #endif
