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
 
  terminal(p);
}
