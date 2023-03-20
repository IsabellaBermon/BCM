// Current sensor var
uint32_t actual_current, avg_current, sens_current, sample_counter = 0;
uint32_t offset_i = 0;
// Thurst sensor var
uint32_t actual_thrust, avg_thrust = 0;
// Temperature sensor var
uint32_t actual_temp, avg_temp = 0;
uint32_t offset_temp = 0;
// Velocity sensor var
uint32_t actual_vel, avg_vel = 0;
// Default var
uint32_t period = 500; //ms
uint32_t fs = 200; //Hz
#define sample_time 1/fs ; //5ms
uint32_t n_samples = 1;
uint32_t max_current = 1; //A
// Set clocks
uint32_t currentTime = 0; 
bool start = true;

void setup() {
  noInterrupts();
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

void save_data_MEM(uint32_t measurement){
  // idk !
}

void update_measurement(uint32_t sample_counter){
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
      actual_thrust = sense_thrust();
      actual_vel = sense_vel();
      actual_temp = sense_temp();
      // update average
      avg_current += actual_current;    
      avg_thrust += actual_thrust;
      avg_vel += actual_vel;
      avg_temp += actual_temp;
      sample_counter += 1;    
      }
  }
  
uint32_t sense_current(){
  uint32_t voltaje = analogRead(A0) * (5.0 / 1023.0);
  uint32_t current = offset_i +(voltaje -2.5)/sens_current; // calculate current 
  return current
}

uint32_t sense_temperature(){
  uint32_t voltage = analogRead(A1) * (5.0 / 1024.0);
  uint32_t temp = offset_temp + voltage * 100;
  return temp
}

uint32_t sense_vel(){
  // Encoder program
  uint32_t vel = actual_vel;
  return vel
}

uint32_t sense_thrust(){
  uint32_t thrust = analogRead(A3);
  return thrust
}

void loop() {

}
