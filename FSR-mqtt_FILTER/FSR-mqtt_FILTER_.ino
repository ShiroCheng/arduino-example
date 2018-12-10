#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <ArduinoJson.h>
#define MQTT_VERSION MQTT_VERSION_3_1_1
#define FILTER_N 3

const char* WIFI_SSID = "Li-511";
const char* WIFI_PASSWORD = "googleiot";
const PROGMEM char* MQTT_CLIENT_ID = "fsrReading11";
const PROGMEM char* MQTT_SERVER_IP = "123.206.127.199";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
const PROGMEM char* MQTT_USER = "admin";
const PROGMEM char* MQTT_PASSWORD = "password";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int Filter_Value;   //filter:滤波, 定义滤波值
int fsrAnalogPin = A0; // FSR is connected to analog A0
int LEDpin = D1; // connect Red LED to pin D1 (PWM pin)
int fsrReading; // the analog reading from the FSR resistor divider
int LEDbrightness;
char msg[50];
char message[50];
bool flag = false;
int data, rate, deep;
int number = 0;
long starttime = 0;
long lasttime = 0;
bool arrived = false;
int fsrVoltage;
unsigned long fsrResistance;
unsigned long fsrConductance;
long fsrForce;

// 收到信息后会调用
void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  // concat the payload into a string
  String payload;
  for (uint8_t i = 0; i < p_length; i++) {
    payload.concat((char)p_payload[i]);
  }  
  Serial.println(payload);

  if(payload.toInt() == 0){
    arrived = true;
  }
  else if(payload.toInt() == 1){
    arrived = false;
  }
  // handle message topic
}


//void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
//    message[i] = (char)payload[i];
//    Serial.print((char)payload[i]);
//  }
//  
//  Serial.println();
//
////  if ((char)payload[0] == '1') {
////    arrived = false;
////  } else {
////    arrived = true;
////  }
//}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("INFO: Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("INFO: connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "okok");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("ERROR: failed, rc=");
      Serial.print(client.state());
      Serial.println("DEBUG: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishData(long fsrForce, int deep, int rate, int number) {
//   create a JSON object
//   doc : https://github.com/bblanchon/ArduinoJson/wiki/API%20Reference
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
//   INFO: the data must be converted into a string; a problem occurs when using floats...
  root["data"] = (String)fsrForce;
  root["deep"] = (String)deep;
  root["rate"] = (String)rate;
  root["number"] = (String)number;
  root.prettyPrintTo(Serial);
    char Jsondata[200];
  root.printTo(Jsondata, root.measureLength() + 1);
  client.publish("FSR", Jsondata,true);
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

   pinMode(LEDpin, OUTPUT);
   starttime = millis();
}



void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
//  Serial.print("Analog reading = ");
//  Serial.println(fsrReading);
//  int chk = DHT11.read(DHT11PIN);

  fsrReading = Filter();  //获得滤波器输出值
  // analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
  fsrVoltage = map(fsrReading, 0, 1023, 0, 5000);
  if (fsrVoltage == 0) {
    fsrForce = 0;
  } 
  else{
    fsrResistance = 5000 - fsrVoltage; // fsrVoltage is in millivolts so 5V = 5000mV
    fsrResistance *= 10000; // 10K resistor
    fsrResistance /= fsrVoltage;
    fsrConductance = 1000000; // we measure in micromhos so
    fsrConductance /= fsrResistance;
    if (fsrConductance <= 1000) {
      fsrForce = fsrConductance *1000 / 80;
    } 
    else {
      fsrForce = fsrConductance - 1000;
      fsrForce = fsrForce *1000 / 30;
    }
  }
 
  //计算呼吸频率
  if(fsrForce == 0 && !flag){
    deep = fsrForce;
  }
  else if(fsrForce > 0){
    flag = true;
    if(deep < fsrForce){
       deep = fsrForce;
    }
  }
  else if(fsrForce == 0 && flag){
    number++;
    flag = false;
    long now = millis();
    rate =  number * 1000 * 60 / (now - starttime) ;
  }

  long now = millis();
  if(now - lasttime > 200){
    lasttime = now;
    
    //注意本句一定要加！
    client.subscribe("inTopic");
    
    if(!arrived){
      publishData((long)fsrForce, (int)deep, (int)rate, (int)number);
      //若收到Android发送的邮件，则停止传输数据
    }
    else{
      fsrForce = deep = rate = number = 0;
      lasttime = now;
    }
  }
  
  client.loop();
}

//获取传感器数据
int Get_AD(){
    fsrReading = analogRead(fsrAnalogPin);
  if(fsrReading <= 15){  //手动滤波，消除数值持续在 15 以下的噪声（欠妥）  噪声非常稳定，不知道用什么滤波法了……
    fsrReading = 0; 
  }
  return fsrReading;
}

//递推平均滤波法（又称滑动平均滤波法）
int filter_buf[FILTER_N + 1]; //把连续取得的N个采样值看成一个队列，队列的长度固定为N
int Filter(){
  int i;
  int filter_sum = 0;
  filter_buf[FILTER_N] = Get_AD();    //获取传感器数据
  for(i = 0; i < FILTER_N; i++) {
    filter_buf[i] = filter_buf[i+1]; //所有数据左移，低位扔掉
    filter_sum += filter_buf[i];
  }
  return (int)(filter_sum / FILTER_N);  //求平均，得到滤波值
}

 /*
A、名称：递推平均滤波法（又称滑动平均滤波法）
B、方法：
把连续取得的N个采样值看成一个队列，队列的长度固定为N，
每次采样到一个新数据放入队尾，并扔掉原来队首的一次数据（先进先出原则），
把队列中的N个数据进行算术平均运算，获得新的滤波结果。
N值的选取：流量，N=12；压力，N=4；液面，N=4-12；温度，N=1-4。
 C、优点：
对周期性干扰有良好的抑制作用，平滑度高；
适用于高频振荡的系统。
D、缺点：
灵敏度低，对偶然出现的脉冲性干扰的抑制作用较差；
不易消除由于脉冲干扰所引起的采样值偏差；
不适用于脉冲干扰比较严重的场合；
比较浪费RAM。
E、整理：shenhaiyu 2013-11-01
 */

