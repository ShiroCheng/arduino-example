#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <dht11.h>
#include <ArduinoJson.h>
#define MQTT_VERSION MQTT_VERSION_3_1_1
const char* WIFI_SSID = "Li-511";
const char* WIFI_PASSWORD = "googleiot";
const PROGMEM char* MQTT_CLIENT_ID = "dht11";
const PROGMEM char* MQTT_SERVER_IP = "123.206.127.199";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "admin";
const PROGMEM char* MQTT_PASSWORD = "password";

WiFiClient wifiClient;
PubSubClient client(wifiClient);
dht11 DHT11;
#define DHT11PIN D1
// 收到信息后会调用
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }

  // handle message topic

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
      // Once connected, publish an announcement...

    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void publishData(float humi, float temp, float dewp, float dewpt) {
  // create a JSON object
  // doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // INFO: the data must be converted into a string; a problem occurs when using floats...
  root["humi"] = (String)humi;
  root["temp"] = (String)temp;
  root["dewp"] = (String)dewp;
  root["dewpt"] = (String)dewpt;
  root.prettyPrintTo(Serial);
  /*
     {
        "temperature": "23.20" ,
        "humidity": "43.70"
     }
  */
  char data[200];
  root.printTo(data, root.measureLength() + 1);
  client.publish("DHT11", data, true);
}
void setup() {
  Serial.begin(9600);
  // init the WiFi connection
  Serial.println();
  Serial.println();
  Serial.print("INFO: Connecting to ");
  WiFi.mode(WIFI_STA);
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("INFO: WiFi connected");
  Serial.print("INFO: IP address: ");
  Serial.println(WiFi.localIP());

  // init the MQTT connection
  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

double Fahrenheit(double celsius) {
        return 1.8 * celsius + 32;
}    //摄氏温度度转化为华氏温度
double Kelvin(double celsius){
        return celsius + 273.15;
}     //摄氏温度转化为开氏温度
double dewPoint(double celsius, double humidity){
        double A0= 373.15/(273.15 + celsius);
        double SUM = -7.90298 * (A0-1);
        SUM += 5.02808 * log10(A0);
        SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
        SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
        SUM += log10(1013.246);
        double VP = pow(10, SUM-3) * humidity;
        double T = log(VP/0.61078);   // temp var
        return (241.88 * T) / (17.558-T);
}
double dewPointFast(double celsius, double humidity){
        double a = 17.271;
        double b = 237.7;
        double temp = (a * celsius) / (b + celsius) + log(humidity/100);
        double Td = (b * temp) / (a - temp);
        return Td;
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  int chk = DHT11.read(DHT11PIN);
  publishData((float)DHT11.humidity,(float)DHT11.temperature,dewPoint(DHT11.temperature, DHT11.humidity),dewPointFast(DHT11.temperature, DHT11.humidity));

  client.loop();
}

