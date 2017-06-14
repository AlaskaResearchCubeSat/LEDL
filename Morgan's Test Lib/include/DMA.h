//Exported on 02/17/2017 at 16:07:49
// version : v2.0-242-g7474509
#ifndef __DMA_H
#define __DMA_H

//include ctl for event set stuff
#include <ctl.h>

//flags for DMA
enum{DMA_EV_SD_SPI=1<<0,DMA_EV_USER=1<<1};

//variable for DMA events
extern CTL_EVENT_SET_t DMA_events;

#endif
