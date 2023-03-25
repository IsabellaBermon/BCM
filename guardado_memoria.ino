#include <EEPROM.h>

float corriente, rpm, empuje, temperatura = 0;
int eeAddress, cont = 0;
float resultado;

void setup() {
  Serial.begin(9600);
}

void loop() {
  corriente = 0.014;
  rpm = 1500;
  empuje = 100;
  temperatura = 150;

  if (cont <= 32){
    // Guarda hasta la pos 512 en memoria corriente, hasta la 524 temperatura y termina
    EEPROM.put(eeAddress, corriente);
    eeAddress += 4;
    EEPROM.put(eeAddress, rpm);
    eeAddress += 4;
    EEPROM.put(eeAddress, empuje);
    eeAddress += 4;
    EEPROM.put(eeAddress, temperatura);
    eeAddress += 4;
  }
  else{
    for (int i = 0; i < EEPROM.length(); i+= 4){
      Serial.print("pos: ");
      Serial.println(i);
      Serial.println(EEPROM.get(i, resultado),3);
    }
  }
  cont ++;
}
