// Current sensor var
uint32_t actual_Current, prom_Current, sens_Current, sample_counter_i  = 0;
uint32_t offset_i = 0;
// Thurst sensor var
uint32_t actual_empuje, prom_empuje, sample_counter_e = 0;
// Temperature sensor var
uint32_t actual_temp, prom_temp, sample_counter_temp = 0;
uint32_t offset_temp = 0;
// Velocity sensor var
uint32_t actual_vel, prom_vel, sample_counter_vel = 0;
// Default var
uint32_t period = 500; //ms
uint32_t fs = 200; //Hz
#define sample_time 1/fs ; //5ms
uint32_t n_samples = 1;
uint32_t max_current = 1; //A
// Set clocks
uint32_t currentTime_i, currentTime_temp, currentTime_e, currentTime_vel = 0; 
bool iflag, tempflag, tflag, velflag = false;
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
  TCNT1 = 0;
  // start other variables clock
  if (start == true) {    
    if (currentTime_i=1){
      tempflag = true;                
      }
    if (currentTime_i=2){
      tflag = true;                
      }
    if (currentTime_i=3){
      velflag = true;
      start = false;                
      }
    }
  // Increment the current time variable every millisecond
  if (iflag == true){
    currentTime_i += 1;               
    }
  if (tempflag == true){
      currentTime_temp += 1;
    }
  if (tflag == true){
      currentTime_e += 1;
    }
  if (velflag == true){
      currentTime_vel += 1;
    }
}

uint32_t update_measurement(sample_counter,prom,var){
    if (sample_counter == n_samples){
        uint32_t measurement = prom/n_samples;
        // save measurement or show
        prom, sample_counter = 0;
      }
    else {
      // sense variable actual = analogRead(); void measure()
      prom += actual;    
      sample_counter += 1;        
      }
    switch (var) {
      case 1: 
              sample_counter_i = sample_counter; 
              break;      
      case 2: 
              sample_counter_Temp = sample_counter;
              break;
      case 3:
              sample_counter_e = sample_counter;
              break;
      case 4: 
              sample_counter_vel = sample_counter;
              break;
      default: 
              break;
    }  
    return prom
  }
  
uint32_t sense_current(){
  uint32_t voltaje = analogRead(A0) * (5.0 / 1023.0);
  uint32_t current = offset_i +(voltaje -2.5)/sens_Current; // calculate current 
  return current
}

uint32_t sense_temperature(){
  uint32_t voltage = analogRead(A1) * (5.0 / 1024.0);
  uint32_t temp = offset_temp + voltage * 100;
  return temp
}

uint32_t sense_vel(){
  uint32_t velp = analogRead(A1) * 100;
  return vel
}

uint32_t sense_empuje(){
  uint32_t empuje = analogRead(A1) * 100;
  return empuje
}

void loop() {
  // Condition if actual_time == 1/fs
  noInterrupts();
  if (currentTime_i >= sample_time) {
    prom_Current = update_measurement(sample_counter_i,prom_Current,1);
    currentTime = 0; // Reset clock
  }  
  else if (currentTime_temp >= sample_time) {
    prom_temp = update_measurement(sample_counter_temp,prom_temp,2);
    currentTime_temp = 0; // Reset clock
  }
  else if (currentTime_e >= sample_time) {
    prom_temp = update_measurement(sample_counter_e,prom_Emp,3);
    currentTime_e = 0; // Reset clock
  }
  else if (currentTime_vel >= sample_time) {
    prom_empuje = update_measurement(sample_counter_temp,prom_empuje,4);
    currentTime_vel = 0; // Reset clock
  }
  interrupts();
}
