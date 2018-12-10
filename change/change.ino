char fc[50];
char ic[20];

void setup() {
  Serial.begin(9600);
}

void loop() {

String s="123.43212";//从一个String开始

float f=s.toFloat();//String转float

int i=s.toInt();//String转int

dtostrf(f,1,5,fc);//将float转为char的数组，其中第三个传入值(5)是保留的位数(5位）

snprintf (ic, 75, "%d", i);//将int转为char的数组.

//以下为串口打印验证
Serial.println(f,5);//打印保留float变量后五位的数
delay(1000);
Serial.println(i);//打印int变量
delay(1000);
Serial.println(fc);//打印float转成的char的数组
delay(1000);
Serial.println(ic);//打印int转成的char的数组
delay(1000);


}
