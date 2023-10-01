#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define b1 18
#define b2 19
#define b3 23
#define red 17
#define white 5
#define yellow 16


bool currentTrigger;

bool lastTrigger;


const char* ssid = "Changopolis";
const char* password = "lachanguita";

const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(b1,INPUT);
  pinMode(b2,INPUT);
  pinMode(b3,INPUT);
  pinMode(red,OUTPUT);
  pinMode(white,OUTPUT);
  pinMode(yellow,OUTPUT);
  digitalWrite(red,HIGH);
  digitalWrite(white,HIGH);
  digitalWrite(yellow,HIGH);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  connectWifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()){
    reconnect();
  }
  else{
    client.loop();
  }

  currentTrigger = !digitalRead(b1);
  
  if (currentTrigger == true && lastTrigger == false){
    Serial.println("Rising");
    String text;
     
    if(digitalRead(b2) == true) text = "Verdadero";
  else text = "Falso";
  client.publish("TC1004B/GUS/switch", text.c_str());
  }

  if (currentTrigger == false && lastTrigger == true){
    Serial.println("Falling");
  }

  lastTrigger = currentTrigger;

}

void connectWifi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.println();

  while(WiFi.status() != WL_CONNECTED){
    //digitalWrite(blue, !digitalRead(blue));
    delay(500);
    Serial.print(".");
  }
   //digitalWrite(blue,LOW);
   //digitalWrite(white,HIGH);
   Serial.print(" ");
   Serial.print("Conectado a la red: ");
   Serial.println(ssid);
   Serial.print("IP Address: ");
   Serial.println(WiFi.localIP()); 
   delay(1000);
   //digitalWrite(white,LOW);
}

void reconnect(){
  while(!client.connected()){
    Serial.println("Intentando conexion a broker :p");
    String clientId = "ESP32client-";
    clientId+=String(random(0xffff),HEX);
    if(client.connect(clientId.c_str())){
      Serial.println("In Session");
      client.subscribe("TC1004B/informacion");
      client.subscribe("TC1004B/switch");
      client.subscribe("TC1004B/Iot-redLed");
      client.subscribe("TC1004B/Iot-whiteLed");
      client.subscribe("TC1004B/Iot-yellowLed");
      client.subscribe("TC1004B/all/#");
      client.publish("TC1004B/informacion", "Hola, soy gus!");
      
    }
    else{
      Serial.println("retry");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.println("Mensaje recibido de [");
  Serial.println(topic);
  Serial.println("]");

  for (int i=0; i<length;i++){
    Serial.print((char)payload[i]);
    
  }
  Serial.println();

  if(String(topic).indexOf("TC1004B/all")>= 0){
    payload[length] = '\0';
    if(String(topic).indexOf("/red") >= 0){
      if(strcmp((char*)payload, "false") == 0) digitalWrite(red, LOW);
      else digitalWrite(red, HIGH);
    }

  if(String(topic).indexOf("/yellow") >= 0){
      if(strcmp((char*)payload, "false") == 0) digitalWrite(yellow, LOW);
      else digitalWrite(yellow, HIGH);

  }

   if(String(topic).indexOf("/green") >= 0){
      if(strcmp((char*)payload, "false") == 0) digitalWrite(white, LOW);
      else digitalWrite(white, HIGH);
    }
  }
  

  if(strcmp(topic, "TC1004B/Iot-redLed") == 0){
    if(payload[0] == '0') digitalWrite(red,LOW);
    else if(payload[0] == '1') digitalWrite(red,HIGH);
  }

   if(strcmp(topic, "TC1004B/Iot-whiteLed") == 0){
    if(payload[0] == '0') digitalWrite(white,LOW);
    else if(payload[0] == '1') digitalWrite(white,HIGH);
  }

   if(strcmp(topic, "TC1004B/Iot-yellowLed") == 0){
    if(payload[0] == '0') digitalWrite(yellow,LOW);
    else if(payload[0] == '1') digitalWrite(yellow,HIGH);
  }
  
}
