#if defined(ARDUINO_ARCH_ESP8266)

#include "ACDC_Dimmer_ESP12E.h"

// Global Variables requied for Timer & Ext interrupt (Used for AC dimmer)
#define STATE_OFF 0
#define STATE_ON 1
#define CURRENT 0
#define PREVIOUS 1

bool dimState[2], ACmainTimerRunning;
int ACdimOutputPin, ACZCSkipCount = 0;
uint8_t ACdimZCPin[2];
uint32_t ACdimTimerVal;
unsigned long MicrosCheck[2];

// For DC Dimmer
ACDCDimmer::ACDCDimmer(int dimOutputPin, uint8_t pwmIncDec)
{
	_dimOutputPin = dimOutputPin; 					// Store Pin
	_pwmIncDec = pwmIncDec;							// Store Inc/Dec Val
	dimState[CURRENT] = STATE_OFF;					// Initialise state 
	dimState[PREVIOUS] = STATE_OFF;					// Initialise state 
	_dimPower[CURRENT] = PWR_MIN;					// Initialise state 
	_dimPower[PREVIOUS] = PWR_MIN;					// Initialise state 
	_dimmerType = DIMMER_DC;						// Set Type as DC
	pinMode(_dimOutputPin, OUTPUT);
}

// For AC Dimmer
ACDCDimmer::ACDCDimmer(int dim_TRIAC_On_pin, int dim_ZC_pin, uint8_t pwmIncDec)
{
	ACdimOutputPin = dim_TRIAC_On_pin;		// Set global variable from Class Instance initialise
	ACdimZCPin[CURRENT] = dim_ZC_pin;		// Set global variable from Class Instance initialise
	_pwmIncDec = pwmIncDec;					// Store Inc/Dec Val
	ACmainTimerRunning = STATE_OFF;			// Initialise global variable 
	dimState[CURRENT] = STATE_OFF;			// Initialise global variable 
	dimState[PREVIOUS] = STATE_OFF;			// Initialise global variable 
	_dimPower[CURRENT] = PWR_MIN;			// Initialise state 
	_dimPower[PREVIOUS] = PWR_MIN;			// Initialise state 
	ACdimTimerVal = 1;						// Initialise global variable
	_dimmerType = DIMMER_AC;				// Set Type as AC
	pinMode(ACdimOutputPin, OUTPUT);
}

void ACDCDimmer::begin(bool initState, uint32_t initPower)
{
	dimState[CURRENT] = initState;
	_dimPower[CURRENT] = initPower;
	if(_dimmerType == DIMMER_AC)
	{
		timer_init();			// Only set interrupts on AC dimmer
		ext_int_init();
		Serial.println("Begin AC Dimmer");
	}
	else
	{
		Serial.println("Begin DC Dimmer");
	}
}

void ACDCDimmer::timer_init(void)
{
	Serial.println("Initialise Timer");
	timer1_attachInterrupt(onTimerISR);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
}

void ACDCDimmer::ext_int_init(void) 
{
	Serial.println("Initialise Ext Interrupt");
	pinMode(ACdimZCPin[CURRENT], INPUT_PULLUP);
	attachInterrupt(ACdimZCPin[CURRENT], isr_ext, RISING);
}

uint8_t ACDCDimmer::getPower(void)
{
	int8_t ret;
	if (dimState[CURRENT] == STATE_ON)
	{
		ret = _dimPower[CURRENT];
//		Serial.println("Getting Power");
	}
	else
	{
		ret = false;
	}
	return ret;
}

void ACDCDimmer::setPower(int8_t power)
{	
	if (power > PWR_MAX) 
	{
		power = PWR_MAX;
	}
	else if(power < PWR_MIN)
	{
		power = PWR_MIN;
	}
	_dimPower[CURRENT] = power;
}

bool ACDCDimmer::chkPowerChange(void)
{	
	bool ret;
	if (_dimPower[CURRENT] != _dimPower[PREVIOUS]) 
	{
		ret = true;
	}
	else
	{
		ret = false;
	}
	_dimPower[PREVIOUS] = _dimPower[CURRENT];		// Reset
	return ret;	
}

void ACDCDimmer::incPower(void)
{	
	if(dimState[CURRENT] == STATE_ON)  // Prevent Inc in off mode
	{
		setPower(getPower() + _pwmIncDec);	//
//		Serial.println("inc Power");
	}
}

void ACDCDimmer::decPower(void)
{	
	setPower(getPower() - _pwmIncDec);	//
//	Serial.println("dec Power");
}

bool ACDCDimmer::getState(void)
{
	bool ret;
	if(dimState[CURRENT] == STATE_ON)
	{
		ret = true;	// For AC
//		Serial.println("getState (on)");
	}
	else
	{
		ret = false;
//		Serial.println("getState (Off)");
	}
	return ret;
}

void ACDCDimmer::setState(bool onOff)
{
	if(onOff == STATE_ON || onOff == STATE_OFF)
	{
		dimState[CURRENT] = onOff;	// For AC
//		Serial.print("setState - Set to ");
//		Serial.println(onOff);
	}
}

void ACDCDimmer::changeState(void)
{
	if(dimState[CURRENT] == STATE_ON)	// For AC
	{
		dimState[CURRENT] = STATE_OFF;
//		Serial.println("changeState (on to off)");
	}
	else
	{
		dimState[CURRENT] = STATE_ON;
//		Serial.println("changeState (off to on)");
	}
}

bool ACDCDimmer::chkStateChange(void)
{	
	bool ret;
	if (dimState[CURRENT] != dimState[PREVIOUS]) 
	{
		ret = true;
		dimState[PREVIOUS] = dimState[CURRENT];		// Reset
//		Serial.println("checkState (Has changed)");
	}
	else
	{
		ret = false;
	}
	return ret;	
}

 // Update Dimmer Power & On Off Status (Scheduler 50ms) ----------------------------------------------
void ACDCDimmer::update(uint8_t *MQTT_flags)
{	
	if (millis() - lastMillisDimmerUpdate > DIMMER_UPDATE_MSECS)
	{
		lastMillisDimmerUpdate = millis();   //Save the last time we were here
		if(chkStateChange())
		{
//			Serial.println("State Change detected");
			MQTT_flags[MQTT_PUB_STATUS_CHANGE] = MQTT_PUB_SET;
			MQTT_flags[MQTT_PUB_CHANGE]++;
			if(getState() == STATE_ON)
			{
				updateOutput(_dimPower[CURRENT]);		// Turn on dimmer to pre-defined power level
			}
			else
			{
				updateOutput(0);			// Turn off dimmer
			}
		}

		if(chkPowerChange())
		{
//			Serial.println("Power Change detected");
			MQTT_flags[MQTT_PUB_POWER_CHANGE] = MQTT_PUB_SET;
			MQTT_flags[MQTT_PUB_CHANGE]++;
			if(getState() == STATE_ON)
			{
				updateOutput(_dimPower[CURRENT]);		// Turn on dimmer to pre-defined power level
			}
		}
	}
	if((micros() - lastMicrosPulseSkipped) > TEN_MS_IN_MICROS)
	{
	}
	
}

void ACDCDimmer::updateOutput(int8_t setPower)
{
	if(_dimmerType == DIMMER_AC)
	{
		ACdimTimerVal = map(setPower, 0, 100, TIMER_MAX, TIMER_MIN); // Val, min_analog, max_analog, 100%, 0%);
//		Serial.println("AC - Changing Output");
	}
	else
	{
	    int dimmerAnalogPower = map(setPower, PWR_MIN, PWR_MAX, 0, PWMRANGE);
		analogWrite(_dimOutputPin, dimmerAnalogPower);    // Update Output
//		Serial.println("DC - Changing Output");
	}
}


void ICACHE_RAM_ATTR isr_ext()
{
	digitalWrite(ACdimOutputPin, LOW);						// Turn off Triac Pulse
	MicrosCheck[CURRENT] = micros();						// Record time
	if (dimState[CURRENT] == STATE_ON) 	
	{
		timer1_write(ACdimTimerVal); 		// Write value representing time from 0s to 9.5ms (z/c to z/c)
		ACmainTimerRunning = TIMER_RUNNING;	// Flag main timer as running
	}

	if((MicrosCheck[CURRENT] - MicrosCheck[PREVIOUS]) >= PULSE_MAX_TIME)	// Check time between Rising pulses
	{
		ACZCSkipCount++;									// cycle Time > 10.05ms - We missed a ZC pulse
		Serial.print("ZC Pulses >= 10.0055ms: ");		// Show number of missed pulse
		Serial.print(ACZCSkipCount);
		Serial.print("  Time between pulses: ");		// Show number of missed pulse
		Serial.println(MicrosCheck[CURRENT] - MicrosCheck[PREVIOUS]);
	}
	MicrosCheck[PREVIOUS] = MicrosCheck[CURRENT];
}

void ICACHE_RAM_ATTR onTimerISR()
{	
	if (dimState[CURRENT] == STATE_ON)
	{
		if(ACmainTimerRunning == TIMER_RUNNING)
		{
			ACmainTimerRunning = TIMER_NOT_RUNNING;	// Turn off main timer run flag
			digitalWrite(ACdimOutputPin, HIGH);		// Turn on Triac pulse for 'ON_PULSE' time (80us)
		}
	}
}

#endif