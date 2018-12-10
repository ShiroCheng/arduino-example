// Prueba de medidas de distancia y temperatura con US-100 en modo serie
// Por RafaG, 2014
 
#include <SoftwareSerial.h>;
 
const int US100_TX = D2;
const int US100_RX = D3;
 
// Instancia nuevo canal serie
SoftwareSerial puertoUS100(US100_RX, US100_TX);
 
unsigned int MSByteDist = 0;
unsigned int LSByteDist = 0;
unsigned int mmDist = 0;
int temp = 0;
 
void setup() {
    Serial.begin(9600);
    puertoUS100.begin(9600);
}
 
void loop() {
 
    puertoUS100.flush(); // limpia el buffer del puerto serie
    puertoUS100.write(0x55); // orden de medición de distancia
 
    delay(500);
 
    if(puertoUS100.available() >= 2) // comprueba la recepción de 2 bytes
    {
        MSByteDist = puertoUS100.read(); // lectura de ambos bytes
        LSByteDist  = puertoUS100.read();
        mmDist  = MSByteDist * 256 + LSByteDist; // distancia
        if((mmDist > 1) && (mmDist < 10000)) // comprobación de la distancia dentro de rango
        {
            Serial.print("Distancia: ");
            Serial.print(mmDist, DEC);
            Serial.println(" mm");
        }
    }
 
    puertoUS100.flush(); // limpia el buffer del puerto serie
    puertoUS100.write(0x50); // orden de medición de distancia
 
    delay(500);
    if(puertoUS100.available() >= 1) // comprueba la recepción de 1 byte
    {
        temp = puertoUS100.read(); // lectura del byte
        if((temp > 1) && (temp < 130)) // comprobación de rango válido
        {
            temp -= 45; // corrige offset de 45º
            Serial.print("Temperatura: ");
            Serial.print(temp, DEC);
            Serial.println(" ºC.");
        }
    }
 
    delay(500);
}

