#include <ArduinoJson.h>

String json="";
String sensor="";
double altitude=0;
long time =0;
double data[2];

void setup() {
   Serial.begin(9600);
}


void loop(){
  if( receive()){
      decodeJson();
  }
     
     Serial.print("sensor  ");
     Serial.println(sensor);
     Serial.print("altitude  ");
     Serial.println(altitude);
     Serial.print("time  ");
     Serial.println(time);
     Serial.print("data[0]  ");
     Serial.println(data[0]);
     Serial.print("data[1]  ");
     Serial.println(data[1]);
     Serial.println("");
     Serial.print("json  ");
     Serial.println(json);

     delay(1000);
}


void  decodeJson() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
   if (!root.success()) {
     Serial.println("parseObject() failed");
   }
   const char* sensorCopy = root["sensor"];
   sensor = sensorCopy ;
   time=root["time"];
   altitude = root["altitude"];
   data[0]=root["data"][0];
   data[1]=root["data"][1];
}

bool receive(){
  bool op=0;
      while (Serial.available() > 0)  
          {
            if(!op){
              json="";
            }
              json += char(Serial.read());
              delay(2);//一个一个接收数据，每次停顿2毫秒
              op=1;
          }
    return op;
  }

