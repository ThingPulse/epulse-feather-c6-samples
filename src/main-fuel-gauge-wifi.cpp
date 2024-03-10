/*
MIT License

Copyright (c) 2024 Daniel Eichhorn, ThingPulse

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Description:
This example reads the battery voltage and state of charge (SoC) from a MAX17048 LiPo fuel gauge and sends it to a HTTP server as GET request.
The server can be a Node-RED instance, for example, which can then process the data further.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h> // Needed for I2C
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library

#include "settings.h"


const char* ssid     = "yourssid"; // Change this to your WiFi SSID
const char* password = "yourpassw0rd"; // Change this to your WiFi password

const char* host = "192.168.0.100"; // HTTP Server receiving GET request with data, e.g. Node-RED
const int httpPort = 1880; // Port of HTTP Server

SFE_MAX1704X lipo(MAX1704X_MAX17048); // Create a MAX17048


void setup()
{
    Serial.begin(115200);
    while(!Serial){delay(100);}

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println("******************************************************");
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

    lipo.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

    // Set up the MAX17043 LiPo fuel gauge:
    if (lipo.begin() == false) // Connect to the MAX17043 using the default wire port
    {
      Serial.println(F("MAX17043 not detected. Please check wiring. Freezing."));
      while (1)
        ;
    }

    // Quick start restarts the MAX17043 in hopes of getting a more accurate
    // guess for the SOC.
    lipo.quickStart();

    // We can set an interrupt to alert when the battery SoC gets too low.
    // We can alert at anywhere between 1% - 32%:
    lipo.setThreshold(20); // Set alert threshold to 20%.
}

void readResponse(WiFiClient *client){
  unsigned long timeout = millis();
  while(client->available() == 0){
    if(millis() - timeout > 5000){
      Serial.println(">>> Client Timeout !");
      client->stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client->available()) {
    String line = client->readStringUntil('\r');
    Serial.print(line);
  }

  Serial.printf("\nClosing connection\n\n");
}

void loop(){
  WiFiClient client;
  String footer = String(" HTTP/1.1\r\n") + "Host: " + String(host) + "\r\n" + "Connection: close\r\n\r\n";

  // WRITE --------------------------------------------------------------------------------------------
  if (!client.connect(host, httpPort)) {
    return;
  }

  float voltage = lipo.getVoltage();
  float soc = lipo.getSOC();

  client.print("GET /epulsec6?voltage=" + String(voltage, 2) + "&soc=" + String(soc, 2) + footer);
  readResponse(&client);
}
