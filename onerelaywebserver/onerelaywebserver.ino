#include "compressorMonitor.h"
#include <MKRGSM.h>

#define PIN "2542"
String remoteNumber = "066488326132";  // number which will be called in case of a problem
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 109);         // IP where this relay will be available


// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
GSM gsmAccess;
GSMVoiceCall vcs;

// Client variables
char linebuf[80];
int charcount = 0;
// compressor monitor object does all the communication with arduino
CompressorMonitor cm = CompressorMonitor(mac, ip, pin, remoteNumber);

void setup() {

    // Open serial communication at a baud rate of 9600
    Serial.begin(9600);

    delay(1000);
    // Connect to the onboard SIM card
    Serial.println("Attempting to connect to SIM card . . .");
    if (gsmAccess.begin(PIN) == GSM_READY) {
        Serial.println("Connected");
    }
    else {
        Serial.println("Not connected");
        delay(1000);
    }

    // start the Ethernet connection and the server:
    Serial.println("Attempting to open server . . .");
    Ethernet.begin(cm.getMac(), cm.getIP());
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
}


// It also print Temperature in C and F
void dashboardPage(EthernetClient & client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println("Refresh: 5");  // refresh the page automatically every 5 sec
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");

    client.println("<!DOCTYPE HTML><html><head>");
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>");
    client.println("<h3>Arduino Web Server - <a href=\"/\">Refresh</a></h3>");

    // output the pin status
    client.print("<br / >");
    client.print("Compressor running: ");
    client.print(cm.getCompRun());
    client.print("<br / >");

    client.print("UnitGood: ");
    client.print(cm.getUnitGood());
    client.print("<br / >");

    client.print("Helium High Temp: ");
    client.print(cm.getHeTemperature());
    client.print("<br / >");

    client.print("Helium High Pressure: ");
    client.print(cm.getHePressure());
    client.print("<br / >");

    client.print("BatteryVoltage");
    client.print(" is ");
    client.print(cm.getVoltage());
    client.println("<br />");

    // Generates buttons to control the 
    // If relay is off, it shows the button to turn the output on
    if (cm.getRelay1() == 0) {
        client.println("<h4>Relay 1 - State: Off</h4>");
        client.println("<a href=\"/relay1on\"><button>ON</button></a>");
    }
    // If relay is on, it shows the button to turn the output off
    else if (cm.getRelay1() == 1) {
        client.println("<h4>Relay 1 - State: On</h4>");
        client.println("<a href=\"/relay1off\"><button>OFF</button></a>");
    }
    client.println("</body></html>");
}


void loop() {

    cm.checkState(vcs);

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
                        cm.setRelay1(false);
                    }
                    else if (strstr(linebuf, "GET /relay1on") > 0) {
                        cm.setRelay1(true);
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
