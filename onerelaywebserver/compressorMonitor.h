#ifndef COMPRESSORMONITOR_H
#define COMPRESSORMONITOR_H
#pragma once

#include <SPI.h>
#include <Ethernet.h>
#include <MKRGSM.h>

class CompressorMonitor {
    public:
        CompressorMonitor(const byte* mac, const IPAddress ip, const String phone);

        byte* getMac();
        IPAddress getIP();

        int getRelay1();
        void setRelay1(const bool);
        int getRelay2();
        void setRelay2(const bool);

        void setRelay1State(const String state);
        String getRelay1State();

        int getRunPin();
        int getHePPin();
        int getHeTPin();
        int getUnitGoodPin();

        int getCompRun();
        int getHePressure();
        int getHeTemperature();
        int getUnitGood();

        int getBatteryState();
        float getVoltage();

        void setVCS(GSMVoiceCall vcs);
        void setSMS(GSM_SMS sms);


        boolean requestRestart();      // calls the actual restart routine

        bool getSendSMS();
        void sendSMS(char* msg);
        bool getMakeCall();
        void callUser();           // method that makes a phone call and/or sends msg

        void checkState();

    private:
        bool debug;

        // Ethernet stuff
        byte mac[6];                // Hols a MAC address for your controller below.
        IPAddress ip;               // The IP address will be dependent on your local network:

        String remoteNumber;

        String relay1State;
        const int relay1;           //run/stop, default is run (relay is wired in normally closed option, if not actuated compressor tries to run, actuate to stop)
        const int relay2;           //local/remote, default state is remote, dont use if no switchover to local needed

        const int runPin;           // compressor running status
        const int highHePPin;       // He Pressure to high
        const int highHeTPin;       // He Temperature to high
        const int unitGoodPin;      // No errors and warnings

        int comprunState;           // variable for reading the  status
        int highHePState;           // variable for reading the  status
        int highHeTState;           // variable for reading the  status
        int unitgoodState;          // variable for reading the  status

        int battState;              // battery state
        float voltage;              // voltage

        unsigned long lastCall;
        unsigned long nextCallTimer;
        unsigned long lastAlert;
        unsigned long nextAlertTimer;

        // GSM RELATED STUFF
        char charbuffer[20];
        char txtmsg[50];
        GSMVoiceCall vcs;
        GSM_SMS sms;

        void prepareRelay();        // helper method that sets all pin modes
        bool restartRoutine();   // method that tries to restart the compressor

};

#endif