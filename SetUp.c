#include <ctl.h>
#include <msp430.h>
#include <stdlib.h>
#include <terminal.h>
#include <SDlib.h>
#include <string.h>
#include <stdio.h>
#include <ARCbus.h>
#include "SensorsOnOff.h"
#include "SetUp.h"

void Periferial_SetUp(void *p) __toplevel {
 int mmcReturnValue;
    //for final version this vreg cant be here for final testing, it keeps the voltage regulator on. 
    VREGon();//turn on voltage regulator in case the voltage on batteries drop, SD card can operate at 2.7 V, but the batteries will not stay there long
    ctl_timeout_wait(ctl_get_current_time()+5);//4.88 MS give time for voltage to stabalize 
    SENSORSon();//turn on sensors 
    //initalize the SD card 
    ctl_timeout_wait(ctl_get_current_time()+100);//wait for voltage on sd card to stabalize 
    initCLK();//SD card expects the 16 MHz clock 
    mmc_pins_on();

    ctl_timeout_wait(ctl_get_current_time()+5);
    while (!async_isOpen())
    {
    ctl_timeout_wait(ctl_get_current_time()+1024);
    }
    ctl_timeout_wait(ctl_get_current_time()+100);

  terminal(p);
}
