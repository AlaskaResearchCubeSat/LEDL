#ifndef __SENSORS_H
  #define __SENSORS_H

 //P7.7 Sensors on/off    //P8.6 ACC on/off    //P8.7 MAG on/off
 void init_timerA(void);
 void SENSORSon(void);
 void SENSORSoff(void);
 void ACCon(void);
 void ACCoff(void);
 void MAGon(void);
 void MAGoff(void);
 void VREGinit(void);
 void VREGon(void);
 void VREGoff(void);
 void GyroMuxon(void);
 void GyroMuxoff(void);
 void GyroSelfTeston(void);
 void GyroSelfTestoff(void);
 void GryoSleep(void);
 void GyroWakeUp(void);
 void Gyroinit(void);
 void UnusedPinSetup(void);
void SD_LED(void);
void RESET_LED(void);
void SD_LED_OFF(void);
void LEDL_SWITCH_TO_EPS(void);
void LEDL_BLOW_FUSE(void);
void GyroSleep(void);
void GyroWakeUp(void);


 #endif
