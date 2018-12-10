#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <stdlib.h>
#include <string.h>
#include <Wire.h>  
//以上为使用的库函数

#define BMP180ADD 0x77  // I2C address of BMP180    
                           //write is (0xEE)     read is (0xEF)         
unsigned char OSS;                              
/**********************MSB      LSB******/  
int ac1;           // 0xAA     0xAB  
int ac2;           // 0xAC     0xAD  
int ac3;           // 0xAE     0xAE  
unsigned int ac4;  // 0xB0     0xB1  
unsigned int ac5;  // 0xB2     0xB3  
unsigned int ac6;  // 0xB4     0xB5  
int b1;            // 0xB6     0xB7  
int b2;            // 0xB8     0xB9  
int mb;            // 0xBA     0xBB  
int mc;            // 0xBC     0xBD  
int md;            // 0xBE     0xBF  
long b5;           
float temperature;    
double pressure;     
double pressure2;  
 
double altitude;    


//以上为读取传感器数据使用的全局变量


const char* ssid = "hzy";//连接的路由器的名字
const char* password = "88888888";//连接的路由器的密码
const char* mqtt_server = "123.206.127.199";//服务器的地址 iot.eclipse.org是开源服务器

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;//存放时间的变量 
char msg[100];//存放要发的数据



void setup_wifi() {//自动连WIFI接入网络
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {//用于接收服务器接收的数据
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);//串口打印出接收到的数据
  }
}

void reconnect() {//等待，直到连接上服务器
  while (!client.connected()) {//如果没有连接上
    if (client.connect("HZYhahahASa")) {//接入时的用户名，尽量取一个很不常用的用户名
      client.subscribe("setmessage");//接收外来的数据时的intopic
    } else {
      Serial.print("failed, rc=");//连接失败
      Serial.print(client.state());//重新连接
      Serial.println(" try again in 5 seconds");//延时5秒后重新连接
      delay(5000);
    }
  }
}


//以上为esp8266功能模块函数

void esp8266offset()  
{  
  ac2=0xFFFF0000+ac2;
  ac3=0xFFFF0000+ac3;
  mb=0xFFFF0000+mb;
  mc=0xFFFF0000+mc;
}  

/** calculate centure **/  
void calculate()  
{  
  temperature = bmp180GetTemperature(bmp180ReadUT());  
  temperature = temperature*0.1;  
  pressure = bmp180GetPressure(bmp180ReadUP());  
  pressure2 = pressure/101325;  
  pressure2 = pow(pressure2,0.19029496);  
  altitude = 44330*(1-pressure2);                            //altitude = 44330*(1-(pressure/101325)^0.19029496);  
}  
  
/** print reslut **/  
void show()  
{  
  Serial.print("Temperature: ");  
  Serial.print(temperature, 1);                            //10 hexadecimal  
  Serial.println(" C");  
  Serial.print("Pressure: ");  
  Serial.print(pressure, 0);                               //10 hexadecimal  
  Serial.println(" Pa");  
  Serial.print("altitude:");  
  Serial.print(altitude);  
  Serial.println("m");  
}  
  
/**BMP180 satrt program**/  
void BMP180start()  
{                     /*MSB*/  
  ac1 = bmp180ReadDate(0xAA);                      //get full data  
  ac2 = bmp180ReadDate(0xAC);    
  ac3 = bmp180ReadDate(0xAE);    
  ac4 = bmp180ReadDate(0xB0);    
  ac5 = bmp180ReadDate(0xB2);    
  ac6 = bmp180ReadDate(0xB4);    
  b1  = bmp180ReadDate(0xB6);    
  b2  = bmp180ReadDate(0xB8);    
  mb  = bmp180ReadDate(0xBA);    
  mc  = bmp180ReadDate(0xBC);    
  md  = bmp180ReadDate(0xBE);  
  esp8266offset(); //only for esp8266
}  
  
/***BMP180 temperature Calculate***/  
short bmp180GetTemperature(unsigned int ut)  
{  
  long x1, x2;  
  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;  //x1=((ut-ac6)*ac5)/(2^15)  
  x2 = ((long)mc << 11)/(x1 + md);                //x2=(mc*2^11)/(x1+md)  
  b5 = x1 + x2;                                   //b5=x1+x2  
  return ((b5 + 8)>>4);                           //t=(b5+8)/(2^4)  
}  
  
/***BMP180 pressure Calculate***/  
long bmp180GetPressure(unsigned long up)  
{  
  long x1, x2, x3, b3, b6, p;  
  unsigned long b4, b7;  
    
  b6 = b5 - 4000;  
  
  x1 = (b2 * (b6 * b6)>>12)>>11;  
  x2 = (ac2 * b6)>>11;  
  x3 = x1 + x2;  
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;  
    
  x1 = (ac3 * b6)>>13;  
  x2 = (b1 * ((b6 * b6)>>12))>>16;  
  x3 = ((x1 + x2) + 2)>>2;  
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;  
    
  b7 = ((unsigned long)(up - b3) * (50000>>OSS));  
  if (b7 < 0x80000000)  
    p = (b7<<1)/b4;  
  else  
    p = (b7/b4)<<1;  
      
  x1 = (p>>8) * (p>>8);  
  x1 = (x1 * 3038)>>16;  
  x2 = (-77357 * p)>>16;  
  p += (x1 + x2 + 3791)>>4;  
    
  return p;  
}  
  
/*** Read 1 bytes from the BMP180  ***/  
  
int bmp180Read(unsigned char address)  
{  
  unsigned char data;  
  Wire.beginTransmission(BMP180ADD);  
  Wire.write(address);  
  Wire.endTransmission();  
  Wire.requestFrom(BMP180ADD, 1);  
  while(!Wire.available());  
  return Wire.read();  
}  
  
/*** Read 2 bytes from the BMP180 ***/  
int bmp180ReadDate(unsigned char address)  
{  
  unsigned char msb, lsb;  
  Wire.beginTransmission(BMP180ADD);  
  Wire.write(address);  
  Wire.endTransmission();  
  Wire.requestFrom(BMP180ADD, 2);  
  while(Wire.available()<2);  
  msb = Wire.read();  
  lsb = Wire.read();  
  return (int) msb<<8 | lsb;  
}  
  
/*** read uncompensated temperature value ***/  
unsigned int bmp180ReadUT()  
{  
  unsigned int ut;  
  Wire.beginTransmission(BMP180ADD);  
  Wire.write(0xF4);                       // Write 0x2E into Register 0xF4  
  Wire.write(0x2E);                       // This requests a temperature reading  
  Wire.endTransmission();    
  delay(5);                               // Wait at least 4.5ms  
  ut = bmp180ReadDate(0xF6);               // read MSB from 0xF6 read LSB from (16 bit)  
  return ut;  
}  
  
/*** Read uncompensated pressure value from BMP180 ***/  
unsigned long bmp180ReadUP()  
{  
  unsigned char msb, lsb, xlsb;  
  unsigned long up = 0;  
    
  Wire.beginTransmission(BMP180ADD);  
  Wire.write(0xF4);                        // Write 0x34+(OSS<<6) into register 0xF4  
  Wire.write(0x34 + (OSS<<6));             // 0x34+oss*64  
  Wire.endTransmission();   
  delay(2 + (3<<OSS));                     // Wait for conversion, delay time dependent on OSS  
  
  Wire.beginTransmission(BMP180ADD);  
  Wire.write(0xF6);                        // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)  
  Wire.endTransmission();  
    
  Wire.requestFrom(BMP180ADD, 3);   
  while(Wire.available() < 3);             // Wait for data to become available  
  msb = Wire.read();  
  lsb = Wire.read();  
  xlsb = Wire.read();  
  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);//16 to 19 bit  
  return up;  
}  


//以上为bmp180模块功能函数

void encodeJson(){
  long pressureCopy=pressure;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root1 = jsonBuffer.createObject();
  
  root1["Temperature"] = double_with_n_digits(temperature,1);
  root1["Pressure"] = pressureCopy;
  root1["altitude"] = altitude;
  
  String json;
  root1.printTo(json);
  Serial.println(json);
  loadMsg(json);
}

void loadMsg(String msgCopy){
   int len=msgCopy.length();
   for(int i=0;i<len;i++){
    msg[i]=msgCopy[i];
   }
}

void setup() {//初始化程序，只运行一遍

  Serial.begin(9600);//设置串口波特率（与烧写用波特率不是一个概念）
  Wire.begin();  //设置为IIC主机
  OSS = 2;  // Oversampling Setting           0: single    1: 2 times    2: 4 times   3: 8 times   
  setup_wifi();//自动连WIFI接入网络
  client.setServer(mqtt_server, 1883);//1883为端口号
  client.setCallback(callback); //用于接收服务器接收的数据
  BMP180start();  
  
}



void loop() {//主循环

  reconnect();//确保连上服务器，否则一直等待。
  client.loop();//MQTT接收数据事件监听。
  long now = millis();//记录当前时间
  if (now - lastMsg > 1000) {//每隔1秒发一次信号
    calculate();  //读取并计算BMP180数据
    encodeJson(); //合成json包
    client.publish("bmp180", msg);//发送数据，反馈设定的温度值
    lastMsg = now;//刷新上一次发送数据的时间
 
  }
}

