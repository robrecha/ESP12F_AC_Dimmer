Library for Dimming AC and 12V loads with respective ESP12F wireless Dimmer modules
Created by Rob Recinella: July 28, 2020.

User Functions:
DC Only		ACDCDimmer(int dimOutputPin, uint8_t pwmIncDec) 
		Setup pin allocation
AC Only		ACDCDimmer(int dim_TRIAC_On_pin, int dim_ZC_pin, uint8_t pwmIncDec)
		Setup pin allocation
AC Only		void begin(bool initState, uint32_t initPower)
		Start timer & Interrupt
AC & DC		uint8_t  getPower(void)
		Returns dimmer output power (0 to 100) %
		bool getState(void)
AC & DC		void setPower(int8_t power)
		set output power (0 to 100) %
AC & DC		void changeState(void)
AC & DC		void setState(bool onOff)
AC & DC		void incPower(void)
AC & DC		void decPower(void)
AC & DC		void update(uint8_t *MQTT_flags)
