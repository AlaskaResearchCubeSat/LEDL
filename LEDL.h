#ifndef __LEDL_H
  #define __LEDL_H
  
    typedef struct{
      unsigned long SDaddress; 
      //unsigned int gyro_data[GYRO_ARRAY_SIZE];
      char temp_data[TEMP_ARRAY_SIZE];
      char clyde_ttc;
      char clyde_array_two_char[2];
      int clyde_array_two_int[2];
  }LEDL_STAT;

  
  typedef struct {
  int clyde_array_fifteen[15];
  }CLYDE_STAT;
  



  extern int clyde_data[CLYDE_ARRAY_SIZE];
  extern int temp_measure[TEMP_ARRAY_SIZE];
  
  extern SD_block_addr SD_read_addr;

  extern arr;
  extern switch_is_on;
  extern mmcReturnValue;
  
  
 void sub_events(void *p);

#endif
