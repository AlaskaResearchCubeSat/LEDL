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
    
  terminal(p);
}
