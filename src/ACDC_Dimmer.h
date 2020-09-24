/*
  ACDCDIMMER.H - Library for Dimming AC and 12V loads with ESP12E wireless Dimmer module
  Created by Rob Recinella July 28, 2020.
*/
#ifndef ACDC_DIMMER_H
#define ACDC_DIMMER_H

#include <stdlib.h>

//#if   defined(ARDUINO_ARCH_ESP32)
//	#include "esp32/AC_Dimmer_ESP32.h"
#if defined(ARDUINO_ARCH_ESP8266)
	#include "esp8266/ACDC_Dimmer_ESP12E.h"
#else 
	#error "This library only supports boards with ESP8266 processor."
#endif

class ACDCDimmer
{         
	protected:
		#define STATE_OFF 0
		#define STATE_ON 1
		#define CURRENT 0
		#define PREVIOUS 1
		#define PWR_MIN 0
		#define PWR_MAX 100
		#define PWM_MIN 0
		#define PWM_MAX 100
		#define MQTT_PUB_CHANGE 0
		#define MQTT_PUB_STATUS_CHANGE 1
		#define MQTT_PUB_POWER_CHANGE 2
		#define MQTT_PUB_INCDEC_CHANGE 3
		#define MQTT_PUB_SET 0
		#define MQTT_PUB_CLR 1
		#define DIMMER_AC 0
		#define DIMMER_DC 1
		#define TIMER_MIN 10			// Max power
		#define TIMER_MAX 45000			// Min Power 9.ms
		#define PULSE_MAX_TIME 10060	// 10.060 ms means a pulse was missed
		#define TEN_MS_IN_MICROS 10060000	// 10.05ms means a pulse was missed
		

		int8_t _dimPower[2];
		//uint8_t _dimState[2];
		int _dimOutputPin;
		int8_t _pwmIncDec;
		uint8_t _dimmerType;
		const int DIMMER_UPDATE_MSECS = 50;                     // 50 ms
		long lastMillisDimmerUpdate = DIMMER_UPDATE_MSECS;
		long lastMicrosPulseSkipped;

		void port_init(void);
		void timer_init(void);
		void ext_int_init(void);
		bool chkPowerChange(void);
		bool chkStateChange(void);
		void updateOutput(int8_t setPower);

    public:   

		ACDCDimmer(int dimOutputPin, uint8_t pwmIncDec);
        ACDCDimmer(int dim_TRIAC_On_pin, int dim_ZC_pin, uint8_t pwmIncDec);
        void begin(bool initState, uint32_t initPower);
		uint8_t  getPower(void);
        bool getState(void);
        void setPower(int8_t power);
		void changeState(void);
		void setState(bool onOff);
		void incPower(void);
		void decPower(void);
		void update(uint8_t *MQTT_flags);
};

#endif
