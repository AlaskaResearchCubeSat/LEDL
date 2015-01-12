#include <ctl.h>
#include <msp430.h>
#include <stdlib.h>
#include <terminal.h>
#include <SDlib.h>
#include <string.h>
#include <stdio.h>
#include "ARCbus.h"

void Periferial_SetUp(void *p) __toplevel {
 int mmcReturnValue;
 //for final version this vreg cant be here for final testing, it keeps the voltage regulator on. 
          VREGon();//turn on voltage regulator in case the voltage on batteries drop, SD card can operate at 2.7 V, but the batteries will not stay there long
          ctl_timeout_wait(ctl_get_current_time()+5);//4.88 MS give time for voltage to stabalize 
          SENSORSon();//turn on sensors 
          //initalize the SD card 
          ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 
          initCLK();//SD card expects the 16 MHz clock 
          mmcInit_msp();

    ctl_timeout_wait(ctl_get_current_time()+5);
    while (!async_isOpen())
    {
    ctl_timeout_wait(ctl_get_current_time()+1024);
    }
    ctl_timeout_wait(ctl_get_current_time()+100);
    mmcReturnValue=mmcInit_card();
    if (mmcReturnValue==MMC_SUCCESS){
    printf("\rCard initalized Sucessfully\r\n");
    }
    else {
    printf("\rERROR initalizing SD card""\r\n Response = %i\r\n %s", mmcReturnValue,SD_error_str(mmcReturnValue));
    }
    ///////////////////////Testing 
     printf("\rSD card is now off, use SDon to access SD card\r\n");
          mmcInit_msp_off();//SHUT DOWN THE SD CARD SO IT WONT PULL UP THE VOLTAGE LINE FOR THE SENSORS ON/OFF POWR LINE 
          SENSORSoff();
          initCLK_lv();//Reduce clock speed for low voltage application
          VREGoff();//turn Voltage regulator off for low power application
          WDT_STOP();
  terminal(p);
}
