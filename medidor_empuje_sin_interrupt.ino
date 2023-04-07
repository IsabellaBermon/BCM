#include <NBHX711.h>
NBHX711 hx711(A2, A3);
void setup() {
  Serial.begin(9600);
  hx711.begin();
  hx711.setScale(16610.857);
  hx711.tare(10);
}
void loop() {
  static unsigned long lastRead;
  hx711.update();
  Serial.println((hx711.getUnits(10)-7.00)*35);
}
