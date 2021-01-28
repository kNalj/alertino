#include "compressorMonitor.h"

CompressorMonitor::CompressorMonitor(const byte* mac, const IPAddress ip, const String phone)
	: relay1(1),
	  relay2(2),
	  runPin(7),
	  highHePPin(3),
	  highHeTPin(0),
	  unitGoodPin(6)

{
	this->debug = false;

	this->remoteNumber = phone;

	// mac address  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
	this->mac[0] = mac[0];
	this->mac[1] = mac[1];
	this->mac[2] = mac[2];
	this->mac[3] = mac[3];
	this->mac[4] = mac[4];
	this->mac[5] = mac[5];

	this->ip = IPAddress(192, 168, 1, 109);

	this->comprunState = this->getCompRun();
	this->highHePState = this->getHePressure();
	this->highHeTState = this->getHeTemperature();
	this->unitgoodState = this->getUnitGood();

	this->lastCall = millis();
	this->lastAlert = millis();
	this->nextCallTimer = 900000;		// 15 minutes
	this->nextAlertTimer = 180000;		// 3 minutes

	this->prepareRelay();
}


//////////////////////////////////////////////////////////
/////////////////// GETERS AND SETERS ////////////////////
//////////////////////////////////////////////////////////

byte* CompressorMonitor::getMac() {
	return this->mac;
}

IPAddress CompressorMonitor::getIP() {
	return this->ip;
}

int CompressorMonitor::getRelay1() {
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

void CompressorMonitor::setRelay1State(const String str) {
	this->relay1State = str;
}

String CompressorMonitor::getRelay1State() {
	return this->relay1State;
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

void CompressorMonitor::setVCS(GSMVoiceCall vcs) {
	this->vcs = vcs;
}

void CompressorMonitor::setSMS(GSM_SMS sms) {
	this->sms = sms;
}

//////////////////////////////////////////////////////////
///////////////////// HELPER METHODS /////////////////////
//////////////////////////////////////////////////////////

void CompressorMonitor::prepareRelay() {
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
	this->setRelay1(false);			// turn compressor off to make sure
	delay(10000);					// wait 10 seconds
	Serial.println("Calling a restart routine.");
	return this->restartRoutine();	// try to re-start compressor
}

boolean CompressorMonitor::restartRoutine() {
	/*
	* 
	*	This method returns boolean because it is used to determine if the compressor should send alert msg.
	*/
	Serial.println("Check if ready to restart");
	if (this->getUnitGood() == 1) {
		Serial.println("It was ready. Start the compressor.");
		this->setRelay1(true);
		delay(10000);
		if (this->getCompRun() == 1) {
			return true;
		}
	}
	else {
		Serial.println("The compressor is not ready to be restarted");
	}
	return false;
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