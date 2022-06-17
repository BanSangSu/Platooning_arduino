// Line tracer
#define ENA A1
#define IN1 8
#define IN2 9
#define ENB A2
#define IN3 10
#define IN4 11

#define CH1 0
#define CH2 1
#define ALL_CH 2

// wifi include
#include "WiFiEsp.h"
#include <ArduinoJson.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(2, 3); // RX, TX
#endif

// wifi end

int leftLineSensor =  12;      // 라인트레이서 왼쪽 센서
int rightLineSensor = 13;      // 라인트레이서 오른쪽 센서

int car_speed = 0;

// Ultra sonic
#define  Measure  1     //Mode select
int URECHO = 6;         // PWM Output 0-25000US,Every 50US represent 1cm
int URTRIG = 5;         // PWM trigger pin
int sensorPin = A0;     // select the input pin for the potentiometer
int sensorValue = 0;    // variable to store the value coming from the sensor

int distance_close = 0;
int close_boolean = 0;
int DistanceMeasured= 0;
// end Ultra sonic

// wifi setting
char ssid[] = "iptime";            // your network SSID (name)
char pass[] = "";                 // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "61.245.248.180";
int port = 3000;

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
unsigned long lastConnectionTime_post = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds
const unsigned long postingInterval_post = 1000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;

// WIFI end
#define NO 3

// printWifiStatus
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
// printWifiStatus end

void setup() {  
  //wifi
  // initialize serial for debugging
  Serial.begin(9600);
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
  // wifi end
  // line tracer
  pinMode(IN1, OUTPUT);     // A 모터 1
  pinMode(IN2, OUTPUT);     // A 모터 2
  pinMode(IN3, OUTPUT);     // B 모터 1
  pinMode(IN4, OUTPUT);     // B 모터 2

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  pinMode(leftLineSensor, INPUT);
  pinMode(rightLineSensor, INPUT);  
//  line tracer end

  // Ultra sonic
  //Serial initialization
  pinMode(URTRIG,OUTPUT);                    // A low pull on pin COMP/TRIG
  digitalWrite(URTRIG,HIGH);                 // Set to HIGH
  pinMode(URECHO, INPUT);                    // Sending Enable PWM mode command
  delay(100);
  Serial.println("Init the sensor");
  // end Ultra sonic
}
// Post data
void postData(){
  // Make a HTTP request
    DynamicJsonDocument doc(256);
    String jsonString = "";
    doc["lorry_no"] = NO; // 트럭에 따라 바꿔주기
    doc["speed"] = car_speed;
    doc["distance"] = DistanceMeasured;

    serializeJson(doc,jsonString);
    Serial.print("jsondata: ");
    Serial.println(jsonString);
    Serial.println();

    // send the HTTP POST request
  String PostHeader = "POST /update_data HTTP/1.1\r\n";
    PostHeader += "Host: " + String(server) + ":" + String(port) + "\r\n";
    PostHeader += "Accept: */*\r\n";
    PostHeader += "Content-Type: application/json; charset=utf-8\r\n";
    
    PostHeader += "Content-Length: ";
    PostHeader.concat(jsonString.length());
    PostHeader += "\r\n";  

    PostHeader += "Connection: Keep-Alive\r\n\r\n";
    
    PostHeader += jsonString;
    PostHeader += "\r\n";
    Serial.print(PostHeader);
    client.print(PostHeader);
    lastConnectionTime_post = millis();
}
// Post dataend

// httpRequest
// this method makes a HTTP connection to the server
void httpRequest()
{
  Serial.println();
    
  // close any connection before send a new request
  // this will free the socket on the WiFi shield
  client.flush();
  client.stop();
  // if there's a successful connection
  while(!client.connected()){
    Serial.println("Connecting...");
    client.connect(server,port);
    delay(3000);
  }
  if(client.connected()){
      Serial.println("Connected.");
      postData();
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
}
// httpRequest end 

void ultra_sonic(){
  Serial.print("Distance Measured=");
  digitalWrite(URTRIG, LOW);
  digitalWrite(URTRIG, HIGH);               // reading Pin PWM will output pulses 
  if(Measure)
  {
    unsigned long LowLevelTime = pulseIn(URECHO, LOW) ;
    if(LowLevelTime>=45000)                 // the reading is invalid.
    {
      Serial.print("Invalid");
    }
    else{
    DistanceMeasured = LowLevelTime /50;   // every 50us low level stands for 1cm
    Serial.print(DistanceMeasured);
    Serial.println("cm1");
  } 
 
  }
  else {
    sensorValue = analogRead(sensorPin);
    if(sensorValue<=10)                   // the reading is invalid.
    {
      Serial.print("Invalid");
    }
    else {
    sensorValue = sensorValue*0.718;     
    Serial.print(sensorValue);
    Serial.println("cm2");
    }
  }
}

void setMotorSpeed(unsigned char mode, unsigned char speed){
  if(mode == CH1){
    analogWrite(ENA, speed);
  }else if(mode == CH2){
    analogWrite(ENB, speed);    
  }else{
    analogWrite(ENA, speed);
    analogWrite(ENB, speed);    
  }
}

void forward() {      // 전진 : 모터 두 개를 모두 정회전 시킴
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);  
}



void back() {         // 후진 : 모터 두 개를 모두 역회전 시킴
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, HIGH);
}



void left() {         // 좌회전 : 오른쪽 모터만 정회전 시킴
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);
      digitalWrite(IN4, LOW);
}



void right() {      // 우회전 : 왼쪽 모터만 정회전 시킴
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
}



void stop() {       // 정지 : 2개의 모터 모두 회전 멈춤
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW); 
}

void line_tracer() {
  // 만약 양쪽 센서가 모두 선 ‘미감지’(반사신호’OK’) 경우 : 전진
  if (DistanceMeasured < 15){
    close_boolean = 1;
  }else if(DistanceMeasured <= 30){
    close_boolean = 0;
    car_speed = 150;
  }
  else{
    close_boolean = 0;
    car_speed = 255;
  }
  setMotorSpeed(ALL_CH, car_speed);
  
  Serial.println(digitalRead(leftLineSensor));
  Serial.println(digitalRead(rightLineSensor));
  Serial.println(car_speed);
  Serial.println ("------");
  if (!digitalRead(leftLineSensor) && !digitalRead(rightLineSensor)   ) {
    forward();  
   } 
  // 만약 왼쪽 센서만 ‘선감지(반사신호X)’ 경우 :  좌회전
  if (!digitalRead(leftLineSensor) && digitalRead(rightLineSensor)) {
    left();  
   }                                                                                                                                                                                                                                                                          
  // 만약 오른쪽 센서만 ‘선감지(반사신호X)’ 경우 :  우회전
  if (digitalRead(leftLineSensor) && !digitalRead(rightLineSensor)) {
    right();
   } 
  // 만약 양쪽 센서 모두 ‘선감지(반사신호X)’ 경우 :  정지
   if ((digitalRead(leftLineSensor) && digitalRead(rightLineSensor)) || close_boolean) {
    stop();  
   }   
}

void loop() {

  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if postingInterval seconds have passed since your last connection,
  // then connect again and send data
//  if (millis() - lastConnectionTime > postingInterval) {
//    httpRequest();
//  }

  ultra_sonic();
  line_tracer();
  if(client.connected()){
      Serial.println("connected!");
      postData();
    }else{
      Serial.println("reconnected");
      httpRequest();
    }

}
