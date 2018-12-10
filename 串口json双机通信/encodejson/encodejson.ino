#include <ArduinoJson.h>
String sensor = "gps";
double altitude = 124.891;
long time = 1351824120;


void setup() {
  Serial.begin(9600);
}

void loop() {
  encodeJson();
  delay(1000);
}

void encodeJson() {
  DynamicJsonBuffer jsonBuffer;
  //    StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root1 = jsonBuffer.createObject();

  root1["sensor"] = sensor;
  root1["time"] = time;
  root1["altitude"] = double_with_n_digits(altitude, 6);

  JsonArray& data = root1.createNestedArray("data");
  data.add(double_with_n_digits(48.756080, 6));
  data.add(double_with_n_digits(2.302038, 6));

  //  root1.printTo(Serial);
  //  root1.prettyPrintTo(Serial);

  String json;
  root1.printTo(json);
  Serial.println(json);

}
