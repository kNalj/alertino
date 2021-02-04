#include "compressorMonitor.h"

CompressorMonitor::CompressorMonitor(const byte* mac, const IPAddress ip, const String phone)
	: relay1(1),
	  relay2(2),
	  runPin(7),
	  highHePPin(3),
	  highHeTPin(0),
	  unitGoodPin(6)

{
	/*
	* Think about monitoring the temperature
	* Monitoring the battery level
	* Consider remote commands via SMS
	*/

	this->debug = false;			// Use this if you want to print stuff for debug
	this->attempts = 0;				// Sometimes the compressor UnitGood never goes to true, we use this to try to turn it on after 3 unsuccessful trie
	
	this->remoteNumber = phone;		// This is the phone number that contacted when there is a problem

	// mac address  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
	// Newer ethernet shields come with a sticker that has the MAC address on them
	this->mac[0] = mac[0];
	this->mac[1] = mac[1];
	this->mac[2] = mac[2];
	this->mac[3] = mac[3];
	this->mac[4] = mac[4];
	this->mac[5] = mac[5];

	// MUST CHANGE THIS TO READ IT FROM THE PARAMETER PASSED BY THE USER
	this->ip = IPAddress(192, 168, 1, 109);

	this->comprunState = this->getCompRun();		// Read the initial state of the compressor Run pin
	this->highHePState = this->getHePressure();		// Read the initial state of the pressure from compressor
	this->highHeTState = this->getHeTemperature();	// Read the initial state of the temperature from compressor
	this->unitgoodState = this->getUnitGood();		// Read the initial state of the UnitGood flag from compressor

	this->lastCall = millis();						// Keep track when the last phone call was made
	this->lastAlert = millis();						// Keep track when the last message was sent
	this->nextCallTimer = 900000;					// 15 minutes
	this->nextAlertTimer = 180000;					// 3 minutes

	this->prepareRelay();							// Helper methodd
}

CompressorMonitor::~CompressorMonitor() {
	/*
	* Destructor for the compressor monitor class
	*/
	free(this->charbuffer);
	free(this->txtmsg);
	free(this->mac);
}


//////////////////////////////////////////////////////////
/////////////////// GETERS AND SETERS ////////////////////
///////////////// RETURNS CLASS MEMBERS //////////////////
////////////// DOESNT READ FROM COMPRESSOR ///////////////
//////////////////////////////////////////////////////////
byte* CompressorMonitor::getMac() {
	/*
	* Returns current MAC address of the ARDUINO device
	*/
	return this->mac;
}

IPAddress CompressorMonitor::getIP() {
	/*
	* Returns current IP of the ARDUINO device
	*/
	return this->ip;
}

int CompressorMonitor::getRelay1() {
	/*
	* Returns the current state of relay1 (used to stop and run the compressor)
	*/
	return this->relay1;
}

void CompressorMonitor::setRelay1(bool state) {
	if (state) {
		digitalWrite(this->relay1, HIGH);
	}
	else
	{
		digitalWrite(this->relay1, LOW);
	}
}

int CompressorMonitor::getRelay2() {
	return this->relay2;
}

void CompressorMonitor::setRelay2(bool state) {
	if (state) {
		digitalWrite(this->relay2, HIGH);
	}
	else
	{
		digitalWrite(this->relay2, LOW);
	}
}

int CompressorMonitor::getRunPin() {
	return this->runPin;
}

int CompressorMonitor::getHePPin() {
	return this->highHePPin;
}

int CompressorMonitor::getHeTPin() {
	return this->highHeTPin;
}

int CompressorMonitor::getUnitGoodPin() {
	return this->unitGoodPin;
}

void CompressorMonitor::setVCS(GSMVoiceCall vcs) {
	this->vcs = vcs;
}

void CompressorMonitor::setSMS(GSM_SMS sms) {
	this->sms = sms;
}


//////////////////////////////////////////////////////////
//////////////////////// GETERS //////////////////////////
////////////////// READ FROM COMPRESSOR //////////////////
//////////////////////////////////////////////////////////
int CompressorMonitor::getCompRun() {
	return digitalRead(this->runPin);
}

int CompressorMonitor::getHePressure() {
	return digitalRead(this->highHePPin);
}

int CompressorMonitor::getHeTemperature() {
	return digitalRead(this->highHeTPin);
}

int CompressorMonitor::getUnitGood() {
	return digitalRead(this->unitGoodPin);
}

int CompressorMonitor::getBatteryState() {
	return analogRead(ADC_BATTERY);
}

float CompressorMonitor::getVoltage() {
	return this->getBatteryState() * (4.3 / 1023.0);
}


//////////////////////////////////////////////////////////
///////////////////// HELPER METHODS /////////////////////
//////////////////////////////////////////////////////////

void CompressorMonitor::debugPrint(char* msg) {
	/*
	* Helper method to print everything to the serial
	* Print only if debug flag is on
	*/
	if (this->debug) {
		Serial.println(msg);
	}
}

void CompressorMonitor::prepareRelay() {
	/*
	* Helper method that sets pin modes and initial values
	*/
	pinMode(this->relay1, OUTPUT);
	pinMode(this->relay2, OUTPUT);
	digitalWrite(this->relay1, LOW);

	pinMode(this->runPin, INPUT);
	pinMode(this->highHePPin, INPUT);
	pinMode(this->highHeTPin, INPUT);
	pinMode(this->unitGoodPin, INPUT);

	return;
}


void CompressorMonitor::checkState() {
	/*
	* Helper method that checks the state of the compressor and calls alert and restart methods
	* This is the only method that should be accessible outside of the object
	*/
	if (this->getCompRun() == 0) {
		Serial.println("Check the time of last call.");
		if (this->getMakeCall()) {
			Serial.println("Trying to make a phone call.");
			this->callUser();
		}
		else {
			Serial.println("Check the time of the last msg");
			if (this->getSendSMS()) {
				Serial.println("Attempting to send a msg.");
				this->sendSMS("This is an automated msg from Arduino monitor. Your system is still OFF. Next update in 3 minutes.");
			}
		}

		Serial.println("Requesting a restart");
		if (this->requestRestart()) {
			Serial.println("Inform user of successful restart.");
			this->sendSMS("Monitor was ready to be turned ON and Arduino monitor turned it ON.");
		}
	}
}

boolean CompressorMonitor::requestRestart() {
	/*
	* Helper method that calls the restart method
	*/
	Serial.println("Turning off.");
	this->setRelay1(true);							// turn compressor off to make sure
	delay(10000);									// wait 10 seconds
	Serial.println("Calling a restart routine.");	// DEBUG
	return this->restartRoutine();					// try to re-start compressor
}

boolean CompressorMonitor::restartRoutine() {
	/*
	* 
	*	This method returns boolean because it is used to determine if the compressor should send alert msg.
	*/
	Serial.println("Check if ready to restart");
	if (this->getUnitGood() == 1) {									// check if compressor is ready to be started
		Serial.println("It was ready. Start the compressor.");		// DEBUG
		this->setRelay1(false);										// Send a signal to turn on the compressor
		delay(10000);												// wait for 10 seconds
		if (this->getCompRun() == 1) {								// check if the compressor was actually started
			return true;												// return true if it was
		}
	}
	else {
		if (this->attempts < 3) {												// check if there were already 3 unsuccessful attempts
			Serial.println("The compressor is not ready to be restarted");			// DEBUG
			this->attempts++;														// increase the number of unsuccessful attempts
		}
		else {
			// If 3 times the compressor was not ready to be started 3 times, try to start it anyway
			// Sometimes the ready flag never goes to true
			this->setRelay1(false);				// Send a signal to turn on the compressor
			delay(10000);						// wait for 10 seconds
			this->attempts = 0;					// reset unsuccessful attempts back to 0
			if (this->getCompRun() == 1) {		// check if the compressor was actually started
				return true;						// return true if it was
			}
		}
		
	}
	return false;													// return false otherwise
}

bool CompressorMonitor::getSendSMS() {
	/*
	* This one is responsible for determining if an alert should be sent.
	*/
	if (millis() - this->lastAlert >= this->nextAlertTimer) {
		this->lastAlert = millis();
		return true;
	}
	return false;
}

void CompressorMonitor::sendSMS(char * msg) {
	/*
	* Sends an SMS to a specified user number
	*/
	this->remoteNumber.toCharArray(this->charbuffer, 20);
	this->sms.beginSMS(this->charbuffer);
	this->sms.print(msg);
	this->sms.endSMS();
}

bool CompressorMonitor::getMakeCall() {
	/*
	* This one is responsible for determining if a phone call should be made.
	*/
	if (millis() - this->lastCall >= this->nextCallTimer) {
		this->lastCall = millis();
		return true;
	}
	return false;
}

void CompressorMonitor::callUser() {
	this->remoteNumber.toCharArray(this->charbuffer, 20);
	if (this->vcs.voiceCall(charbuffer)) {
		delay(20000);
		this->vcs.hangCall();
	}
}