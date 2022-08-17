
#include "WiFi.h"
#include <WebServer.h>
#include <EEPROM.h>

void createWebServer(void);

// Replace with your network credentials (STATION)
//const char* ssid = "CST WIFI";
//const char* password = "iotdevs123";

WebServer server(80);


void initWiFi(String ssid, String password) {

  int c = 0;


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    if (c == 10) return;
    Serial.print('.');
    delay(1000);
    c++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected To ");
    Serial.println(WiFi.localIP());
  }


}

void setup() {


  Serial.begin(115200);
  EEPROM.begin(512); //Initialasing EEPROM

  Serial.println("Reading EEPROM SSID...");
  String eepromSSID;

  for (int i = 0; i < 32; ++i) {
    eepromSSID += char(EEPROM.read(i));
  }

  Serial.print("Saved SSID: ");
  Serial.println(eepromSSID);
  Serial.println();
  /*****************************************************************/

  /************** Read EEPROM for Password  from 32-64 ******************/
  Serial.println("Reading EEPROM Password...");
  String eepromPassword = "";

  for (int i = 32; i < 64; ++i) {
    eepromPassword += char(EEPROM.read(i));
  }

  Serial.print("Saved Password: ");
  Serial.println(eepromPassword);
  Serial.println();
  /*****************************************************************/

  // Set WiFi to station mode and disconnect from an AP if it was previously connected

  initWiFi(eepromSSID, eepromPassword);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");

  WiFi.softAP("MVM OVIT", "");
  server.begin();
  Serial.println("Server started");
  createWebServer();
  }
  

}

void loop() {

  server.handleClient();
  delay(100);
}

void createWebServer() {
  IPAddress ip = WiFi.softAPIP();
  Serial.println(ip);

  server.on("/connect", []() {
    String querySsid = server.arg("ssid");
    String queryPassword = server.arg("pass");

    Serial.println(querySsid);
    Serial.println(queryPassword);

    server.send(200, "text/html", "Success");
    initWiFi(querySsid, queryPassword);


  });

  server.on("/scan", []() {

    Serial.println("scan start");
    String networkSSID = "";

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
      Serial.println("no networks found");
    } else {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found



        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");

        if (n > 0) {
          networkSSID += "{\"ssid\":";
          networkSSID += "\"";
          networkSSID += WiFi.SSID(i);
          networkSSID += "\"}";

          if (i < n - 1) {
            networkSSID += ",";
          }
        } else {
          networkSSID += "{\"no_of_networks\":";
          networkSSID += "\"";
          networkSSID += n;
          networkSSID += "\"}";

        }
        delay(10);
      }
    }

    server.send(200, "text/html", "[" + networkSSID + "]");
  });


server.on("/setting", []() {
    //Getting credentials
    String querySsid = server.arg("ssid");
    String queryPassword = server.arg("pass");

    //Saving Credentials and phone webserver URL to EEPROM
    if ((querySsid.length() > 0) && (queryPassword.length() > 0)) {
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      for (int i = 0; i < querySsid.length(); ++i) {
        EEPROM.write(i, querySsid[i]);
      }
      for (int i = 0; i < queryPassword.length(); ++i) {
        EEPROM.write(32 + i, queryPassword[i]);
      }

      EEPROM.commit();

      //Respond to the request
      server.send(200, "text/plain", "{\"status\":\"success\"}");   //cre_rec = Credentials Received
      delay(3500);

      //Restart the esp to connect to Wi-Fi with new credentials.
      ESP.restart();
    }
  });
}
