#include "WiFiEsp.h"
#include <ArduinoJson.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(2, 3); // RX, TX
#endif

char ssid[] = "302";            // your network SSID (name)
char pass[] = "menghuan666";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "61.245.248.180";
int port = 3000;

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 100L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;

void printWifiStatus()
{
  delay(100);
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  delay(100);
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  delay(100);
  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setup()
{
  // initialize serial for debugging
  Serial.begin(115200);
  // initialize serial for ESP module
  Serial1.begin(115200);
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    delay(500);
  }

  Serial.println("You're connected to the network");
  
  printWifiStatus();
}

// this method makes a HTTP connection to the server
void httpRequest()
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.flush();
  client.stop();

  // if there's a successful connection
  if (client.connect(server, port)) {
    Serial.println("Connecting...");

    // Make a HTTP request
    DynamicJsonDocument doc(256);
    String jsonString = "";
    doc["lorry_no"] = 1; // 트럭에 따라 바꿔주기
    doc["speed"] = "2";
    doc["distance"] = 3;

    serializeJson(doc,jsonString);
    Serial.print("jsondata: ");
    Serial.println(jsonString);

    delay(10);
    // send the HTTP POST request
  String PostHeader = "POST /update_data HTTP/1.1\r\n";
    PostHeader += "Host: " + String(server) + ":" + String(port) + "\r\n";
    PostHeader += "Accept: */*\r\n";
    PostHeader += "Content-Type: application/json; charset=utf-8\r\n";
    PostHeader += "Content-Length: " + String(jsonString.length()) + "\r\n\r\n";
    PostHeader += jsonString + "\r\n";
    PostHeader += "Connection: keep-alive\r\n\r\n";
    client.println(PostHeader);
  lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}

void loop()
{
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if postingInterval seconds have passed since your last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}
