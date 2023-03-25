#include <EEPROM.h>

int eeAddress, apuntador = 0;
float resultado;

bool LED_STATE = false;
// Clock variables
volatile uint32_t pulse_start_time = 0;  // Tiempo de referencia inicio
volatile uint32_t pulse_end_time = 0;  // Tiempo de referencia final

// Pins
int acs = A0;
int lm35 = A1;
const byte interruptPin = 2; ///< Pin 2 para interrupciones, el arduino UNO attachInterrupt solo sirve en 2 y 3

// Motor voltaje 0 - 255
int pinMotor = 9;
int voltageMotor = 0; // vence inercia con 70 (sin ACS712), 80 con ACS712

// Current sensor var
float actual_current, avg_current = 0;
float sens_current = 0.185;
uint32_t offset_i = 0;
// Temperature sensor var
float actual_temp, avg_temp = 0;
float offset_temp = 0;
// Velocity sensor var
volatile int contador = 0;   // Valor del contador de pulsos por segundo
uint32_t actual_vel,actual_vel_freq, avg_vel = 0;

// Default variables
uint32_t period = 500; //ms
uint32_t fs = 200; //Hz
#define sample_time 1/fs //5ms
uint32_t n_samples = 10;
int sample_counter = 0;
uint32_t max_current = 1; //A

void setup() {
  pinMode(pinMotor,OUTPUT);
  pinMode(acs,INPUT);
  pinMode(lm35,INPUT);
  Serial.begin(9600);
  analogWrite(pinMotor,voltageMotor);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin),falling_edge_detected,FALLING);  // Interrupcion 0 (pin2) 
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000100; //Set prescalar to 256
  TIMSK1 |= B00000010; //Enable compare match A
  OCR1A = 31250;
  sei();
}

float sense_current(){
  float voltaje = analogRead(acs) * (5.0 / 1023.0);  
  float current = abs(voltaje -2.48)/sens_current-0.12 ; // calculate current 
  return current;
}

float sense_temperature(){
  float voltage = analogRead(lm35) * (5.0 / 1023.0);
  float temp = offset_temp + voltage * 100;
  return temp;
}

void update_measurement(){
  if (sample_counter == n_samples){
      float measurement_i = avg_current/n_samples;
      float measurement_vel = avg_vel/n_samples;
      float measurement_temp = avg_temp/n_samples;
      save_data_MEM(measurement_i,measurement_vel,measurement_temp,measurement_temp);
      // save measurement or show
      Serial.print(measurement_vel);
      Serial.print(" RPM | ");
      Serial.print(measurement_i);
      Serial.print(" A | ");
      Serial.print(measurement_temp);
      Serial.println(" Cº");
      avg_current = 0;
      avg_vel = 0;
      avg_temp = 0;
      sample_counter = 0;
  }
  else {
    // sense all variables
    actual_current = sense_current();
    delayMicroseconds(104); // Wait for the ADC to settle
    actual_vel = actual_vel_freq;    
    actual_temp = sense_temperature();
    // update average
    avg_current += actual_current;   
    avg_vel += actual_vel;
    avg_temp += actual_temp;
    sample_counter += 1;    
  }
}

void save_data_MEM(float corriente, float rpm, float empuje, float temperatura){
  EEPROM.put(eeAddress, corriente);
  eeAddress += 4;
  EEPROM.put(eeAddress, rpm);
  eeAddress += 4;
  EEPROM.put(eeAddress, empuje);
  eeAddress += 4;
  EEPROM.put(eeAddress, temperatura);
  eeAddress += 4;
  apuntador ++;  
}

void extract_data_MEM(){
  for (int i = 0; i < (apuntador+1)*4; i+= 4){
    Serial.print("Position: ");
    Serial.println(i);
    Serial.println(EEPROM.get(i, resultado),3);
  }
}

void loop() {
  if (Serial.available()) {
    String voltageM = Serial.readStringUntil('\n');
    voltageMotor = voltageM.toInt();
    analogWrite(pinMotor,voltageMotor);
  }
  pulse_end_time = millis(); 
  if (pulse_end_time > (pulse_start_time + 1000)) {   
    pulse_start_time = millis();  
    actual_vel_freq = contador*30;   
    contador = 0;
  }
}

ISR(TIMER1_COMPA_vect){
  // when actual_time == 1/fs
  TCNT1 = 0;
  update_measurement();
  LED_STATE = !LED_STATE;
  digitalWrite(LED_BUILTIN, LED_STATE);
}

void falling_edge_detected()    // Funcion que se ejecuta durante cada interrupcion
{
  contador++;           // Se incrementa en uno el contador
}