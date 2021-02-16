#include "compressorMonitor.h"
#include <MKRGSM.h>

#define PIN "2542"                          // PIN number to access the SIM card if there is one
String remoteNumber = "066488326132";       // number which will be called in case of a problem
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED      // If your ethernet shield has a sticker with MAC address, you should use that one
};
IPAddress ip(192, 168, 1, 109);             // IP where this relay will be available


EthernetServer server(80);                  // We need this to output the info 
GSM gsmAccess;                              // Access to the SIM card
GSM_SMS sms;                                // Used to send SMS alerts
GSMVoiceCall vcs;                           // Used to make call alerts

// Client variables
char linebuf[80];
int charcount = 0;
// compressor monitor object does all the communication with arduino
CompressorMonitor cm = CompressorMonitor(mac, ip, remoteNumber);        // Make an instance of compressor monitor class

void setup() {

    Serial.begin(9600);                     // Open serial communication at a baud rate of 9600

    delay(1000);
    // Connect to the onboard SIM card
    Serial.println("Attempting to connect to SIM card . . .");      // DEBUG
    if (gsmAccess.begin() == GSM_READY) {                        // Try to connect to card
        Serial.println("Connected");                                // DEBUG
    }
    else {
        Serial.println("Not connected");                            // DEBUG
        delay(1000);                                                // Wait for 1 second
    }

    cm.setVCS(vcs);                                                 // Pass the voice call handeler to the compressor monitor
    cm.setSMS(sms);                                                 // Pass the SMS handeler to the compressor monitor

    Serial.println("Attempting to open server . . .");              // DEBUG
    Ethernet.begin(cm.getMac(), cm.getIP());                        // start the Ethernet connection
    server.begin();                                                 // Start the server
    Serial.print("server is at ");                                  // DEBUG
    Serial.println(Ethernet.localIP());                             // DEBUG
}


// It also print Temperature in C and F
void dashboardPage(EthernetClient & client) {
    /*
    * A function that builds and displays a simple "web page" that has some information about the compressor and its status on it
    */
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");                // the connection will be closed after completion of the response
    client.println("Refresh: 5");                       // refresh the page automatically every 5 sec
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");

    client.println("<!DOCTYPE HTML><html><head>");
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>");
    client.println("<h3>Arduino Web Server - <a href=\"/\">Refresh</a></h3>");

    // output the pin status
    client.print("<br / >");
    client.print("Compressor running: ");
    client.print(cm.getCompRun());              // Grab and display if the compressor is ON or OFF
    client.print("<br / >");

    client.print("UnitGood: ");
    client.print(cm.getUnitGood());             // Grab and display UnitGood flag, this indicates if the compressor is ready to be restarted
    client.print("<br / >");

    client.print("Helium High Temp: ");
    client.print(cm.getHeTemperature());        // Currently this is not used, but the plan is that it will display the temperature
    client.print("<br / >");

    client.print("Helium High Pressure: ");
    client.print(cm.getHePressure());           // Currently this is not used, but the plan is that it will display the pressure
    client.print("<br / >");

    client.print("BatteryVoltage");
    client.print(" is ");
    client.print(cm.getVoltage());              // This displays current level of the battery that powers the device
    client.println("<br />");

    // Generates buttons to control the 
    // If relay is off, it shows the button to turn the output on
    if (cm.getRelay1() == 0) {                                              // If compressor is ON, build and display a button to turn it OFF
        client.println("<h4>Relay 1 - State: Off</h4>");
        client.println("<a href=\"/relay1on\"><button>ON</button></a>");
    }
    // If relay is on, it shows the button to turn the output off
    else if (cm.getRelay1() == 1) {                                         // If compressor is OFF, build and display a button to turn it ON
        client.println("<h4>Relay 1 - State: On</h4>");
        client.println("<a href=\"/relay1off\"><button>OFF</button></a>");
    }
    client.println("</body></html>");
}


void loop() {

    cm.checkState();                                        // Thats the function that will check if the compressor is function normally

    // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
        Serial.println("new client");
        memset(linebuf, 0, sizeof(linebuf));
        charcount = 0;
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                //read char by char HTTP request
                linebuf[charcount] = c;
                if (charcount < sizeof(linebuf) - 1) charcount++;
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank) {
                    dashboardPage(client);
                    break;
                }
                if (c == '\n') {
                    if (strstr(linebuf, "GET /relay1off") > 0) {
                        cm.setRelay1(true);
                    }
                    else if (strstr(linebuf, "GET /relay1on") > 0) {
                        cm.setRelay1(false);
                    }
                    // you're starting a new line
                    currentLineIsBlank = true;
                    memset(linebuf, 0, sizeof(linebuf));
                    charcount = 0;
                }
                else if (c != '\r') {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }

        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disonnected");
    }
}
