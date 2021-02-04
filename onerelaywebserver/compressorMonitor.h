#ifndef COMPRESSORMONITOR_H
#define COMPRESSORMONITOR_H
#pragma once

#include <SPI.h>
#include <Ethernet.h>
#include <MKRGSM.h>

class CompressorMonitor {
    public:
        CompressorMonitor(const byte* mac, const IPAddress ip, const String phone);     // Constructor
        ~CompressorMonitor();                                                           // Destructor

        byte* getMac();                             // Get the mac of the arduino from compressor monitor object
        IPAddress getIP();                          // Get the IP of the arduino from compressor monitor object

        int getRelay1();                            // Get the value of the relay 1 from the compressor monitor object
        void setRelay1(const bool);                 // Fer seting the stop/start pin of the compressor
        int getRelay2();                            // Get the value of the relay 2 from the compressor monitor object
        void setRelay2(const bool);                 // For seting the remote access pin of the compressor

        int getRunPin();                            // Get the number of the compressor running pin
        int getHePPin();                            // Get the number of the pressure pin
        int getHeTPin();                            // Get the number of the temperature pin
        int getUnitGoodPin();                       // Get the number of the UnitGood pin

        int getCompRun();                           // Get the status of the Compressor running pin
        int getHePressure();                        // Get the pressure from the compressor
        int getHeTemperature();                     // Get the temperature from the compressor
        int getUnitGood();                          // Get the status of the UnitGood pin from the compressor

        int getBatteryState();                      // Gets the raw level of battery level
        float getVoltage();                         // Get the current level of the battery as a float

        void setVCS(GSMVoiceCall vcs);              // Set the voice call handling variable to the compressor monitor object
        void setSMS(GSM_SMS sms);                   // Set the sms handling variable to the compressor monitor object


        boolean requestRestart();                   // calls the actual restart routine

        bool getSendSMS();                          // Helper method to determine if new SMS needs to be sent
        void sendSMS(char* msg);                    // Helper method for sending SMS from arduino
        bool getMakeCall();                         // Helper method to determine if a call needs to be made
        void callUser();                            // Helper method that makes a phone call

        void checkState();                          // Helper method that checks if the compressor is ON, and calls methods to alert the user and restart the compressor

    private:
        bool debug;
        int attempts;

        // Ethernet stuff
        byte mac[6];                    // Hols a MAC address for your controller below.
        IPAddress ip;                   // The IP address will be dependent on your local network:

        String remoteNumber;            // This is where the number to shich the alerts will be sent to is stored

        String relay1State;
        const int relay1;               // run/stop, default is run (relay is wired in normally closed option, if not actuated compressor tries to run, actuate to stop)
        const int relay2;               // local/remote, default state is remote, dont use if no switchover to local needed

        const int runPin;               // compressor running status
        const int highHePPin;           // He Pressure to high
        const int highHeTPin;           // He Temperature to high
        const int unitGoodPin;          // No errors and warnings

        int comprunState;               // variable for reading the  status
        int highHePState;               // variable for reading the  status
        int highHeTState;               // variable for reading the  status
        int unitgoodState;              // variable for reading the  status

        int battState;                  // battery state
        float voltage;                  // voltage

        unsigned long lastCall;         // Keep time when last call was made
        unsigned long nextCallTimer;    // Minimum amout of time that has to pass from last call to make another call
        unsigned long lastAlert;        // Keep time when last SMS was sent
        unsigned long nextAlertTimer;   // Minimum amout of time that has to pass from last SMS to send another SMS

        // GSM RELATED STUFF
        char charbuffer[20];            // Buffer to store phonenumber in when making a phone call (call method requires char array rather then string)
        char txtmsg[50];                // char array buffer for text msg that will be sent (possibly not needed)
        GSMVoiceCall vcs;               // GSM library variable that is capable of making phone calls
        GSM_SMS sms;                    // GMS library variable that is capable of sending msgs

        void prepareRelay();            // helper method that sets all pin modes
        bool restartRoutine();          // method that tries to restart the compressor
        void debugPrint(char* msg);     // helper method that prints something to serial if debug flag is true

};

#endif