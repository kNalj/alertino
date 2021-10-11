#include "compressorMonitor.h"
#include <MKRGSM.h>

//////////////// This is a list of fridges and their addresses ////////////////
////////////////
////////////////

#define PIN ""                              // PIN number to access the SIM card if there is one
String remoteNumber = "066488326132";       // number which will be called in case of a problem
String name = "Smurf 2";                // Name of the arduino, to be able to differentiate them when on their web servers
bool hasSim = false;                        // Some arduinos dont have SIM cards in them, this is to avoid init of gsm stuff
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xCD      // If your ethernet shield has a sticker with MAC address, you should use that one
};
//IPAddress ip(192, 168, 1, 188);
IPAddress ip(10, 21, 42, 104);             // IP where this relay will be available


EthernetServer server(80);                  // We need this to output the info 
GSM gsmAccess;                              // Access to the SIM card
GSM_SMS sms;                                // Used to send SMS alerts
GSMVoiceCall vcs;                           // Used to make call alerts

// Client variables
char linebuf[80];
int charcount = 0;
bool redirect = false;
// compressor monitor object does all the communication with arduino
CompressorMonitor cm = CompressorMonitor(mac, ip, remoteNumber, name);      // Make an instance of compress or monitor class

void setup() {

    Serial.begin(9600);                                               // Open serial communication at a baud rate of 9600

    delay(2000);

  cm.setHasSIM(hasSim);                                               // Change to true if you plan to use this arduino with sim card
    
    if (cm.getHasSIM()) {
      // Connect to the onboard SIM card
      Serial.println("Attempting to connect to SIM card . . .");      // DEBUG
      if (gsmAccess.begin() == GSM_READY) {                           // Try to connect to card
          Serial.println("Connected");                                // DEBUG
      }
      else {
          Serial.println("Not connected");                            // DEBUG
          delay(1000);                                                // Wait for 1 second
      }
  
      cm.setVCS(vcs);                                                 // Pass the voice call handeler to the compressor monitor
      cm.setSMS(sms);                                                 // Pass the SMS handeler to the compressor monitor
    }
    else {
      Serial.println("No SIM card mode selected.");
    }

    Serial.println("Attempting to open server . . .");                // DEBUG
    Ethernet.begin(cm.getMac());                                      // start the Ethernet connection
    server.begin();                                                   // Start the server
    Serial.print("server is at ");                                    // DEBUG
    Serial.println(Ethernet.localIP());                               // DEBUG
}


// It also print Temperature in C and F
void dashboardPage(EthernetClient & client, bool shouldRedirect) {
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
    client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    if (shouldRedirect) {
      client.println("<meta http-equiv=\"refresh\" content=\"1; URL=/\" />");
    }
    client.println("<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta3/dist/css/bootstrap.min.css'>");
    client.println("</head><body>");
    client.print("<h1 class='display-3'><b>");
    client.print(cm.getName());
    client.print("</b></h1>");
    client.println("<h3><a href=\"#\" data-toggle=\"tooltip\" title=\"Click to refresh site\">Arduino Web Server</a></h3>");

    client.print("<table class=\"w-25 table\"><thead><tr><th>Parameter</th><th>Status</th><th>Action</th></tr></thead><tbody>");
    client.print("<tr><td>Compressor running</td><td>");
    client.print(cm.getCompRun());
    client.print("</td></tr><tr><td>Unit Good</td><td>");
    client.print(cm.getUnitGood());
    client.print("</td></tr><tr><td>Helium High Temp</td><td>");
    client.print(cm.getHeTemperature());
    client.print("</td></tr>");
    client.print("<tr><td>Helium High Pressure</td><td>");
    client.print(cm.getHePressure());
    client.print("</td></tr><tr><td>Battery Voltage</td><td>");
    client.print(cm.getVoltage());
    client.print("</td></tr>");
    
    client.print("<tr><td>Compressor</td><td>");
    if (cm.getRelay1()) {
      client.print("ON");
      client.print("</td><td><a href='/?relay=OFF'><button id=\"relay\" value=\"");
      client.print("ON");
      client.print("\"");
      client.print("class=\"btn btn-danger\">Turn OFF</button></a></td></tr>");
    }
    else {
      client.print("OFF");
      client.print("</td><td><a href='/?relay=ON'><button id=\"relay\" value=\"");
      client.print("OFF");
      client.print("\"");
      client.print("class=\"btn btn-success\">Turn ON</button></a></td></tr>");
    }
    client.print("<tr><td>Monitoring</td><td>");
    if (cm.getMonitoring()) {
      client.print("ON");
      client.print("</td><td><a href='/?monitoring=OFF'><button id=\"monitoring\" value=\"");
      client.print("ON");
      client.print("\"");
      client.print("class=\"btn btn-danger\">Turn OFF</button></a></td></tr>");
    }
    else {
      client.print("OFF");
      client.print("</td><td><a href='/?monitoring=ON'><button id=\"monitoring\" value=\"");
      client.print("OFF");
      client.print("\"");
      client.print("class=\"btn btn-success\">Turn ON</button></a></td></tr></tbody></table>");
    }
    client.println("</body>");
    client.println("</html>");
}


void loop() {

    if (cm.getMonitoring()) {
        cm.checkState();                                        // Thats the function that will check if the compressor is functioning normally
    }
    redirect = false;

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
                    dashboardPage(client, redirect);
                    break;
                }
                if (c == '\n') {
                    if (strstr(linebuf, "GET /?relay=OFF") > 0) {
                      cm.setRelay1(true);
                      redirect = true;
                    }
                    else if (strstr(linebuf, "GET /?relay=ON") > 0) {
                      cm.setRelay1(false);
                      redirect = true;

                    }
                    else if (strstr(linebuf, "GET /?monitoring=ON") > 0) {
                      cm.setMonitoring(true);
                      redirect = true;

                    }
                    else if (strstr(linebuf, "GET /?monitoring=OFF") > 0) {
                      cm.setMonitoring(false);
                      redirect = true;
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
