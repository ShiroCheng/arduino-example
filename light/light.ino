#include<ESP8266WiFi.h>
#include<PubSubClient.h>

#define PIN_A 0
#define PIN_D 2

const char* ssid = "lxz";
const char* password = "lxz19971006";
const char* mqtt_server = "123.206.127.199";

WiFiClient espClient;
PubSubClient client(espClient);
char LightString[50];

void setup_wifi(){                               //自动连WiFi接入网络
  delay(10);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup()
{
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void reconnect() {                               //连接服务器
  while (!client.connected()) {
    if (client.connect("Light")) {           //mqtt协议的注册ID
      client.publish("Light", LightString);
    } 
    else {
      Serial.print("failed, rc=");              //连接失败  
      Serial.print(client.state());             //重新连接
      Serial.println(" try again in 5 seconds");          //延迟5秒后重新连接
      delay(5000);
    }
  }
}
void loop()
{
  if(!client.connected()) {                         //连接服务器
    reconnect();                   
  }
  client.loop();
  
  int val;
  val = analogRead(PIN_A);
//  Serial.print("a:");
  Serial.println(val);
  snprintf(LightString,75,"%d",val);
  client.publish("LightTopic",LightString);
  delay(500);
}

