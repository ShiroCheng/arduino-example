#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define OUTPIN1 D1
#define OUTPIN2 D2



const char* ssid = "LI-511";//连接的路由器的名字
const char* password = "googleiot";//连接的路由器的密码
const char* mqtt_server = "192.168.74.1";//服务器的地址 iot.eclipse.org是开源服务器

//const char* ssid = "WLAN_AP_506";
//const char* password = "yc2016pwd";
//const char* mqtt_server = "192.168.1.105";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;//存放时间的变量 
char msg[50];    //存放要发的数据
bool setting=0;  //被遥控的变量

void setup_wifi() {//自动连WIFI接入网络
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}


void reconnect() {//等待，直到连接上服务器
    while (!client.connected()) {//如果连接上
    if (client.connect("HYCdashuaigehaha579")) {//接入时的用户名，尽量取一个很不常用的用户名
      client.subscribe("BBQ");//接收外来的数据时的intopic
    } 
    else {
      Serial.print("failed, rc=");//连接失败
      Serial.print(client.state());//重新连接
      Serial.println(" try again in 5 seconds");//延时5秒后重新连接
      delay(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {//用于接收服务器接收的数据
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);//串口打印出接收到的数据
  }
  Serial.println();//换行
  if ((char)payload[0] == 'a') {//如果收到的数据是a,控制前进
         setting=!setting;//改变继电器的状态。
         if(setting==0){
          digitalWrite(OUTPIN1,LOW);
            }
          else{
            digitalWrite(OUTPIN1,HIGH);
          }//根据setting的值控制继电器的状态。
  } 
  Serial.println();//换行
  if ((char)payload[0] == 'b') {//如果收到的数据是b,控制后退
         setting=!setting;//改变继电器的状态。
         if(setting==0){
          digitalWrite(OUTPIN2,LOW);
            }
          else{
            digitalWrite(OUTPIN2,HIGH);;
          }//根据setting的值控制继电器的状态。
  } 
}




void setup() 
{
   pinMode(OUTPIN1,OUTPUT);//设置D1引脚状态为可输出状态
   pinMode(OUTPIN2,OUTPUT);//设置D1引脚状态为可输出状态
   Serial.begin(9600);//设置串口波特率（与烧写用波特率不是一个概念）
   setup_wifi();//自动连WIFI接入网络
   client.setServer(mqtt_server, 1883);//61613为端口号
   client.setCallback(callback); //用于接收服务器接收的数据
}

 void loop() {//主循环 
    float tem=88;//温度，测试使用
    reconnect();//确保连上服务器，否则一直等待。
    client.loop();//MCU接收数据的主循环函数。
    long now = millis();//记录当前时间
    if (now - lastMsg > 1000) {//每隔1秒发一次信号
    dtostrf(tem,1,2,msg);//将float转为char的数组msg，其中第三个传入值(2)是保留的位数(2位）
    client.publish("BBQ", msg);//发送数据，其中BBQ是发出去的topic
    Serial.println("ok!");//串口打印OK！
    lastMsg = now;//刷新上一次发送数据的时间
  }
 }
  

