#include "compressorMonitor.h"

CompressorMonitor::CompressorMonitor(const byte* mac, const IPAddress ip, const String pin, const String phone)
	: relay1(1),
	  relay2(2),
	  runPin(7),
	  highHePPin(3),
	  highHeTPin(0),
	  unitGoodPin(6)

{
	this->relay1State = "On";
	char buffer[10];
	pin.toCharArray(buffer, 10);
	this->pin = buffer;
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

char* CompressorMonitor::getPin() {
	return this->pin;
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

void CompressorMonitor::setSendAlert(boolean sendAlert) {
	this->sendAlert = sendAlert;
}

bool CompressorMonitor::getSendAlert() {
	return this->sendAlert;
}

bool CompressorMonitor::shouldSendAlert() {
	return this->sendAlert;
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


void CompressorMonitor::checkState(GSMVoiceCall vcs) {
	if (this->getCompRun() == 0) {
		if (this->getSendAlert()) {
			this->alertUser(vcs);
		}
		this->setSendAlert(this->requestRestart());
	}
}

boolean CompressorMonitor::requestRestart() {
	this->setRelay1(false);			// turn compressor off to make sure
	delay(10000);					// wait 10 seconds

	return this->restartRoutine();	// try to re-start compressor
}

boolean CompressorMonitor::restartRoutine() {
	/*
	* 
	*	This method returns boolean because it is used to determine if the compressor should send alert msg.
	* 
		Check if the compressor is ready to be turned back ON
			if it is:
				Turn on monitor
				return false
			if it is not:
				Seand a msg that it is not ready

			return true
	
	*/

	if (this->getUnitGood() == 1) {
		this->setRelay1(true);
		return false;
	}
	else {
		Serial.println("The compressor is not ready to be restarted");
	}
	return true;
}

void CompressorMonitor::alertUser(GSMVoiceCall vcs) {
	this->remoteNumber.toCharArray(this->charbuffer, 20);
	if (vcs.voiceCall(charbuffer)) {
		delay(10000);
		vcs.hangCall();
	}
}