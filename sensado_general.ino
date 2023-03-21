#include "HX711.h"

// Current sensor var
uint32_t actual_current, avg_current, sens_current, sample_counter = 0;
uint32_t offset_i = 0;

// Thurst sensor var
uint32_t actual_thrust, avg_thrust = 0;
uint32_t scale_cell = 439430.25; // valor incial antes de calibrar / valor esperado antes de calibrar de objeto conocido

// Temperature sensor var
uint32_t actual_temp, avg_temp = 0;
uint32_t offset_temp = 0;

// Velocity sensor var
uint32_t actual_vel,actual_vel_freq, avg_vel = 0;

// Default variables
uint32_t period = 500; //ms
uint32_t fs = 200; //Hz
#define sample_time 1/fs //5ms
uint32_t n_samples = 1;
uint32_t max_current = 1; //A

// Set clocks
uint32_t currentTime = 0;
uint32_t signal_frequency = 0; 

// Pins
int acs = A0;
int lm35 = A1;
int DOUT_loadcell = A2;
int CLK_loadcell = A3;
const byte interruptPin = 2; // change to interrup if not 2 allowed
volatile uint32_t pulse_end_time = 0; 

HX711 m_thrust;

void setup() {
  noInterrupts();
  pinMode(acs,INPUT);
  pinMode(lm35,INPUT);
  m_thrust.begin(DOUT_loadcell, CLK_loadcell);
  m_thrust.set_scale(scale_cell); // Establecemos la escala
  m_thrust.tare(20);  //El peso actual es considerado Tara.
  attachInterrupt(digitalPinToInterrupt(interruptPin), rising_edge_detected, RISING);
  // timer counter setup
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000100; //Set prescalar to 256
  TIMSK1 |= B00000010; //Enable compare match A
  OCR1A = 31250;
  interrupts();
}

ISR(TIMER1_COMPA_vect) {
  // when actual_time == 1/fs
  TCNT1 = 0;
  noInterrupts();
  update_measurement(sample_counter);
  interrupts();
}

void rising_edge_detected() {
    signal_frequency++;   
}

void set_timer1_prescaler(uint8_t prescaler_value) {
  noInterrupts();
  TCCR1B &= B11111000;  // clear prescaler bits
  TCCR1B |= prescaler_value;  // set new prescaler value
  interrupts();
}

void setup_compare_match(int value){
  noInterrupts();
  if (value / 62 < 65535) {
    set_timer1_prescaler(1)
    OCR1A = value / 62;
  }
  else {
    value /= 1e6; // convert to milliseconds
    if (value / 0.5 < 65535) {
      set_timer1_prescaler(2)
      OCR1A = value / 0.5;
    }
    else {
      value /= 1e3; // convert to microseconds
      if (value / 4 < 65535) {
        set_timer1_prescaler(3)
        OCR1A = value / 4;
      }
      else {
        set_timer1_prescaler(4)
        OCR1A = value / 16;
      }
    }
  }
  interrupts();
}

void save_data_MEM(uint32_t measurement){
  // idk !
}

void update_measurement(uint32_t sample_counter){
  noInterrupts();
  if (sample_counter == n_samples){
      uint32_t measurement_i = avg_current/n_samples;
      uint32_t measurement_t = avg_thrust/n_samples;
      uint32_t measurement_vel = avg_vel/n_samples;
      uint32_t measurement_temp = avg_temp/n_samples;
      // save measurement or show
      // save_data_MEM(measurement_i)
      avg_current = 0;
      avg_thrust = 0;
      avg_vel = 0;
      avg_temp = 0;
      sample_counter = 0;
  }
  else {
    // sense all variables
    actual_current = sense_current();
    delayMicroseconds(104); // Wait for the ADC to settle
    actual_thrust = sense_thrust();
    actual_vel = actual_vel_freq;
    actual_temp = sense_temp();
    // update average
    avg_current += actual_current;    
    avg_thrust += actual_thrust;
    avg_vel += actual_vel;
    avg_temp += actual_temp;
    sample_counter += 1;    
  }
  interrupts();
}
  
uint32_t sense_current(){
  uint32_t voltaje = analogRead(acs) * (5.0 / 1023.0);
  uint32_t current = offset_i +(voltaje -2.5)/sens_current; // calculate current 
  return current;
}

uint32_t sense_temperature(){
  uint32_t voltage = analogRead(lm35) * (5.0 / 1023.0);
  uint32_t temp = offset_temp + voltage * 100;
  return temp;
}

uint32_t sense_thrust(){
  uint32_t thrust = m_thrust.get_units(20);
  return thrust;
}

void loop() {
  float t = 1.0 / fs;
  int value = t * 1e9; // convert to nanoseconds
  // when fs change call setup_compare_match(value)
  noInterrupts();
  pulse_end_time = millis();  
  if (pulse_end_time > (pulse_start_time + 1000)) {
      actual_vel_freq = signal_frequency;      
      signal_frequency = 0;
      pulse_start_time = millis();      
  }
  interrupts();
}
