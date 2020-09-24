#ifndef ACDC_DIMMERESP12E_H
#define ACDC_DIMMERESP12E_H

#include "Arduino.h"
#include "ACDC_Dimmer.h"
#include "pins_arduino.h"
#include <stdio.h>
#include "ets_sys.h"

#define MAX_INSTANCES 10
#define TIMER_RUNNING 1
#define TIMER_NOT_RUNNING 0
//#define TIMER_MAX 47500		// Min Power 9.5ms
//#define TIMER_MAX 49500		// Min Power 9.9ms
#define ON_PULSE 2000 			// 400us
//#define ON_PULSE 435 // 80us

void isr_ext(void);
void ICACHE_RAM_ATTR onTimerISR();

#endif
