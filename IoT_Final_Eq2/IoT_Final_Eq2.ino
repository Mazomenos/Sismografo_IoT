//-----------------------------------------------------------------------------------------------------------------------
// Implementación de internet de las cosas
// Grupo 501
// Proyecto Final: Sismografo IoT
// Equipo 2
// Integrantes:
// Gustavo Betancourt Mazomenos - A01252832
// Alejandro Jauregui Zarate - A01252368
// Javier Eduardo Hurtado Rodríguez - A01254039
// Juan Carlos García - A01252830
//-----------------------------------------------------------------------------------------------------------------------

//Librerias utilizadas
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "DHT.h"
#include <Ticker.h>

//Se declaran los tickers para tomar tiempos a consideracion
Ticker sendInfo, ledMQTT, led5seconds;


//Se definen los componentes utilizados
#define SW 23 // boton
#define RED_LED 18 // led rojo
#define GREEN_LED 19 // led verde
#define POT 34 // potenciometro
#define DHTPIN 32 // Sensor de humedad / temperatura
#define DHTTYPE DHT11 // Se define el tipo de sensor utilizado
DHT dht(DHTPIN, DHTTYPE); // Se linkea el senosr dht con su tipo de sensor

// Credenciales de WiFi para entrar al internet
const char* ssid = "no";
const char* password = "se";

// URL para meter datos al servidor (mysql)
String serverName="http://epiz2104166.infinityfreeapp.com/Api/datosHT.php";
String serverSismo="http://epiz2104166.infinityfreeapp.com/Api/datosTiempo.php";


bool greenFlag = false;

// Url del host de mqttx
const char* mqtt_server = "test.mosquitto.org";

//Se declara el cliente de WiFi
WiFiClient espClient;
PubSubClient client(espClient);

// Se declara el acelerometro
Adafruit_MPU6050 mpu;

//-----------------------------------------------------------------------------------------------------------------------

// Funcion para mandar la humedad y temperatura al servidor
void sendTempHum(){
  Serial.println("Enviando datos");
  float humedad = dht.readHumidity();
  float temperatura = dht.readTemperature();

  // Se crea el path con el url y los datos
  String datosEnviar;
    datosEnviar += "&m=";
    datosEnviar += 3; 
    datosEnviar += "&t=";
    datosEnviar += temperatura;
    datosEnviar += "&h=";
    datosEnviar += humedad;
  HTTPClient http;
   // Se manda el identifiacdor del esp, temperatura y humedad 
  http.begin(serverName);
  http.addHeader("Content-Type","application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(datosEnviar.c_str());
}

//-----------------------------------------------------------------------------------------------------------------------

//  Se crea el path con el url y los datos
void sendSismo(unsigned long duracion){
  String datosEnviar;
    datosEnviar += "&m=";
    datosEnviar += 3; 
    datosEnviar += "&ds=";
    datosEnviar += duracion/1000.00;
  HTTPClient http;
  http.begin(serverSismo);
  // Se manda el identifiacdor del esp y  la duracion del sismo
  http.addHeader("Content-Type","application/x-www-form-urlencoded");
  int httpResponseCode = http.POST(datosEnviar.c_str()); 

}

// Esta funcion apaga el led despues de 5 seg desde que el sismo paro
void turnoffLedR(){
  digitalWrite(RED_LED, LOW);
}
//-----------------------------------------------------------------------------------------------------------------------

// Setup donde se empiezan los procesos y se establecen la funcion de los pines
void setup() {
  Serial.begin(115200); // Se incia el monitor serial para debug
  pinMode(GREEN_LED, OUTPUT); // Se establece el led verde como salida
  pinMode(RED_LED, OUTPUT); // Se establece el led rojo como salida
  pinMode(SW, INPUT); // Se establece el boton como entrada
  wifiConnect(); // Se establece la conexion a internet
  mpu.begin(); // Se inicializa el acelerometro
  Wire.begin();  // Se inicializa el potenciometro
  dht.begin(); // Se inicializa el sensor de humedad y temperatura
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G); // se define el rango del acelerometro
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ); // Se filtra la banda del acelerometro
  client.setServer(mqtt_server, 1883); // Se conecta el mqttx al puerto 1883
  client.setCallback(callback); // se establece el callback para cuando reciba un mensaje de mqttx
  sendInfo.attach(30, sendTempHum); // Se manda la temperatura y humedad cada 30 segundos
  Serial.println("");
  delay(100);
}

//-----------------------------------------------------------------------------------------------------------------------

// loop donde se realizan todos los procesos
void loop() {
  // Se reconecta con el mqttx
  if(!client.connected()){
    reconnect();
  }
  // se establece el loop de conexion del cliente
  client.loop();

  static unsigned long timer, timerSis, ledTimer, TotalTime, Max; // variables de tiempo
  float excess,x,y,z,v; // variables potenciometro
  sensors_event_t a, g, t; // declara funciones del acelerometro
  mpu.getEvent(&a, &g, &t); // se declara la obtencion de valores del acelerometro
  static bool Vsismo = false; // booleano para identificar si hay sismo o no

  // Se establece los valores del potenciometro para ajustar la sensibilidad del sismografo
  int pot = analogRead(POT);
  int val = map(pot, 0, 4095, 0, 16);

  // se meten las lecturas del acelerometro a variables
  x = a.acceleration.x;
  y = a.acceleration.y;
  z = a.acceleration.z;
  v = sqrt(pow(x,2)+pow(y,2)+pow(z,2)); // se calcula la magnitud
  excess = abs(v-9.4); // se ajusta el valor de la magnitud para que estandarizar la gravedad

  // Se checa si el led verde esta prendido
  if(digitalRead(GREEN_LED) && greenFlag == false){
    ledTimer = millis();
    greenFlag = true;
  }
  // Se apaga el led verde al presionar el boton
  if(!digitalRead(SW) == true || millis()-ledTimer >= 10000){
    digitalWrite(GREEN_LED, LOW);
    greenFlag = false;
  }
  // Se toma la magnitud maxima
  if(excess > val){
    if(excess > Max){
      Max = excess;
    }
    timer = millis();
  }
  // Se detecta la primera punta del sismo
  if(millis()-timer <= 100 && Vsismo == false){
    timerSis = millis();
    Vsismo = true;
  }
  // Se verifica si sigue con el sismo despues de medio segundo para saber si es falso positivo
  if(millis()-timer >=500 && Vsismo == true){
    TotalTime = (millis()-timerSis)-500;
    // Se detecta sismo
    if(TotalTime >= 1000){ // Sismo
      digitalWrite(RED_LED, HIGH);
      Serial.println("Sismo");
      String text = "1";
      client.publish("TC1004B/Javier/sismo",text.c_str()); // se publica a mqttx para prender los leds verdes de los otros esp´s
      client.publish("TC1004B/Alejandro/sismo",text.c_str()); // se publica a mqttx para prender los leds verdes de los otros esp´s
      sendIFTTTnotif(TotalTime); // Se manda la notificacion a IFTTT
      sendSismo(TotalTime);
      led5seconds.once(5, turnoffLedR); // Se apaga el led rojo despues de 5 segundos del sismo
    }
    Vsismo = false; // Se declara el sismo como falso
  }
}

//-----------------------------------------------------------------------------------------------------------------------

// Funcion para establecer conexion de wifi
void wifiConnect(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a la red: ");
  Serial.println(ssid);
  Serial.print("Direccion IP: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

//-----------------------------------------------------------------------------------------------------------------------

// Funcion de reconexion con mqttx
void reconnect(){
  while(!client.connected()){
    Serial.println("Intentando conexion a broker MQTT");
    String clientId = "ESP32client-";
    clientId+=String(random(0xffff),HEX);
    if(client.connect(clientId.c_str())){
      Serial.println("Conectado!");
      client.subscribe("TC1004B/Gustavo/#");
    }
    else{
      Serial.println("Conexion fallida, reintentando en 5 segundos...");
      delay(5000);
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------

// Se establece que al recibir un 1 en su canal de mqttx, este prenda su led verde
void callback(char* topic, byte* payload, unsigned int length){
  if(strcmp(topic, "TC1004B/Gustavo/sismo") == 0){
    if(payload[0] == '1'){
      digitalWrite(GREEN_LED, HIGH);
    } 
  }
}

//-----------------------------------------------------------------------------------------------------------------------

// Funcion que manda la notificacion a IFTTT con los datos del sismo
void sendIFTTTnotif(unsigned long TotalTime){
  HTTPClient http;
  String serverPath = "https://maker.ifttt.com/trigger/test/with/key/_ZwFpyqfgLuZ4WuCEnM2A?value1="; // URL de IFTTT pra mandar mensaje
  //  Se le agrega al path los valores que recibe y muestra la notificacion
  serverPath += TotalTime/1000.00; 
  http.begin(serverPath.c_str());
  
  int httpResponseCode = http.GET();

  if(httpResponseCode > 0){
  Serial.print("Http Response Code: ");
  Serial.println(httpResponseCode);
  Serial.println(http.getString());
  }
  else{
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
