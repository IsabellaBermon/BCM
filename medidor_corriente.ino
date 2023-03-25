  #include <String.h>   
  #include <stdio.h>
  #include <ctype.h>

float Sensibilidad=0.185; //sensibilidad en Voltios/Amperio para sensor de 5A
String vel_;
int n_muestras = 50;

void setup() {
  pinMode(9,OUTPUT);
  Serial.begin(9600);
}

void loop() {
  if(Serial.available() > 0){
    vel_ = Serial.readStringUntil('\n');
  }
  int vel = vel_.toInt();
  float voltajeSensor;
  float corriente = 0;
  analogWrite(9,vel);
  for(int i=0;i<n_muestras;i++)
  {
    voltajeSensor = analogRead(A0) * (5.0 / 1023.0);////lectura del sensor
    corriente=corriente+(voltajeSensor-2.5)/Sensibilidad; //Ecuación  para obtener la corriente
  }
  corriente=corriente/n_muestras;
  Serial.print("Corriente: ");
  Serial.println(corriente,3); 
  delay(200);     
}