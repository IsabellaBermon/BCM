#include <String.h>   
#include <stdio.h>
#include <ctype.h>
#include <EEPROM.h>

// Variables de guardado en memoria
int eeAddress, apuntador,copia_apuntador = 0;
float resultado;

// Variables para generar señal triangular
uint32_t t_actual = 0;
uint32_t t_anterior = 0;
uint32_t v_max = 100;
uint32_t period = 500000; //en us
int32_t m = v_max*1000000/(period/2);
int32_t b = 0;
int32_t number_points = 1;
int32_t acum = 0;
float y, y_aux = 0;
bool ban_step, ban_tri = true;
bool ban_stimulus = false;
// Variables para generar señal triangular
int32_t percentage = 50;
float y_out = 0;

// Variables default de interfaz
int32_t average = 1;
int32_t maxcurrent = 1; //1A
bool verbose = false; //verbose = off
int32_t mode = 1; //1 = step, 2 = triangular
uint32_t fsample = 200; //hz
uint32_t ftime = 16000000/256;
int n_samples = 100;
int sample_counter = 0;
uint32_t max_current = 1; //A

// Verificacion de comandos
bool incorrect = false;
const int MAX_WORDS = 3;   ///< Constante global que determina el tamaño de la lista de elementos
String complete[MAX_WORDS]; ///< Lista de máximo dos elementos para verificar válidez del comando

// Sensado de señales
// Clock variables
volatile uint32_t pulse_start_time = 0;  // Tiempo de referencia inicio
volatile uint32_t pulse_end_time = 0;  // Tiempo de referencia final
// Pins de sensores y motor para la generacion
int acs = A0;
int lm35 = A1;
const byte interruptPin = 2; ///< Pin 2 para interrupciones, el arduino UNO attachInterrupt solo sirve en 2 y 3
int pinMotor = 6;
// Variables por sensor ----------------------
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
float measurement_i = 0;
float measurement_vel = 0;
float measurement_temp = 0;

void setup(){
  pinMode(pinMotor,OUTPUT);
  pinMode(acs,INPUT);
  pinMode(lm35,INPUT);
  //analogWrite(pinMotor,voltageMotor);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin),falling_edge_detected,FALLING);  // Interrupcion 0 (pin2) 
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000100; //Set prescalar to 256
  TIMSK1 |= B00000010; //Enable compare match A
  OCR1A = ftime/fsample;
  sei();
  Serial.begin(9600);  
  wait();
}

void loop() {
  // Genera la señal, step o triangular
  //noInterrupts();
  t_actual = micros();
  if(t_actual >= t_anterior + period/number_points){
    noInterrupts();
    t_anterior = micros();
    if (ban_stimulus){
      if (mode == 2){
        acum += period/number_points;
        y = m*(float(acum)/1000000) + b;  // m = v/s
        y = constrain(y,0,100);
        y_out = map(int(y*100),0,v_max*100,0,255);
        //Serial.println(y);
        analogWrite(pinMotor,y_out);
        if(acum >= period/2  && ban_tri){
          m = -m;
          b = v_max-m*(float(period/2)/1000000);  //s
          ban_tri = false;
        }
        else if(acum >= period && !ban_tri){
          acum = 0;
          m = -m;
          b = 0;
          ban_tri = true;
        }
      }
      else{
        stimulus(percentage);
      }
    }
    interrupts();
  }

  // Imprime por serial
  if (verbose){
    Serial.print(" V: ");
    Serial.print(measurement_vel);
    Serial.print(" m/s |");
    Serial.print("I: ");
    Serial.print(measurement_i);
    Serial.print(" A | ");
    Serial.print("T: ");
    Serial.print(measurement_temp);
    Serial.println(" C ");      
  }

  // Sensar los rpm por medio de interrupciones
  pulse_end_time = millis(); 
  if (pulse_end_time > (pulse_start_time + 1000)) {   
    noInterrupts();
    pulse_start_time = millis();  
    actual_vel_freq = contador*30;   
    contador = 0;
    interrupts();
  }

  // Hace la validación de comandos
  if(Serial.available() > 0) {
    TIMSK1 |= B00000000; //Desenable compare match A
    String command = Serial.readStringUntil('\n');
    command.toLowerCase();
    TIMSK1 |= B00000010; //Enable compare match A
    Serial.println(command);
    
    if(command == "start"){    
      ban_stimulus = true;  
      Serial.println("STIMULUS ACTIVATED");
      wait();
    }

    else if (command == "stop"){
      ban_stimulus = false;
      analogWrite(pinMotor,0);
      Serial.println("STIMULUS DISABLED");
      wait();
    }

    else if (command == "state"){
      Serial.println("-------------------------");
      if (mode == 1){
        Serial.println("MODE STP");
      }
      else if (mode == 2){
        Serial.println("MODE TRG");
      }
      
      Serial.print("PERIOD ");
      Serial.println(period/1000); //muestro el valor en milis, pero internamente trabajo con micros
      Serial.print("FREQUENCY ");
      Serial.println(1000000/period);
      Serial.print("AVERAGE ");
      Serial.println(average);
      if (verbose){
        Serial.println("VERBOSE ON");     
      }
      else{
        Serial.println("VERBOSE OFF");
      }
      Serial.print("MAXCURRENT ");
      Serial.println(maxcurrent);
      Serial.println("-------------------------");
      wait();    
    }

    else if (command == "fdata"){
      noInterrupts();
      extract_data_MEM();
      interrupts();      
      wait();
    }

    else if (command == "reset"){
      period = 500000; //500ms
      average = 1;
      maxcurrent = 1; //1A
      verbose = false; //verbose = off
      fsample = 200;
      percentage = 50;
      mode = 1; //1 = step, 2 = triangular
      number_points = 1;
      wait();
    }

    else if (command == "verbose on"){
      verbose = true;      
      wait();
    }

    else if (command == "verbose off"){
      verbose = false; 
      Serial.println("DONE");
      wait();   
    }

    else if (command == "mode trg"){       
      mode = 2;
      number_points = 1000;
      y = 0;
      Serial.println("DONE");
      wait();
    }

    else{
      verify(command);
    }
  }
  
}

void verify(String command){
  int count = 0;
  int index = 0;
  int aux = 0;
  complete[0] = "x";
  complete[1] = "x";
  complete[2] = "x";
  while (index != -1){
    // Busca la posición del string donde hay espacio
    index = command.indexOf(' ', count);
    // Separa cada palabra por el espacio
    String word = command.substring(count, index);
    // Verifica que el comando no sea mayor a dos palabras
    if (aux < MAX_WORDS){
      // Guardo cada palabra en una lista
      complete[aux] = word; 
      //Serial.println(word);
      aux += 1;     
    }
    else{
      incorrect = true;
      Serial.println("Comando incorrecto");
      wait();
    }  
    count = index + 1;    
  }
  // Solo entra si el comando tiene dos palabras
  if (!incorrect){
    // Hace validaciones
    if (complete[0] == "mode"){
      if (complete[1] == "stp"){
        if (complete[2].toInt() != 0){
          mode = 1;
          number_points = 1;
          percentage = complete[2].toInt();         
          Serial.println("DONE");
          wait();
        }
        else{
          Serial.println("Comando incorrecto");
          wait();
        }
      }
      else{
        Serial.println("Comando incorrecto");
      }
    }

    else if (complete[0] == "period"){
      if (complete[1].toInt() != 0){
        period = complete[1].toInt();
        period = period * 1000; //convierto el periodo ingresado de milis a micro
        Serial.println("DONE");
        wait();
      }
      else{
        Serial.println("Comando incorrecto");
        wait();
      }
    }

    else if (complete[0] == "fsample"){
      if (complete[1].toInt() != 0){
        fsample = complete[1].toInt();        
        TCCR1B |= B00000100;  // pone el preescalador de 256, puede contar hasta 62500
        OCR1A = (uint32_t) (ftime)/fsample;
        Serial.println("DONE");
        wait();
      }
      else{
        Serial.println("Comando incorrecto");
        wait();
      }
    }
    
    else if (complete[0] == "average"){
      if (complete[1].toInt() != 0){
        average = complete[1].toInt();
        Serial.println("DONE");
        wait();
      }
      else{
        Serial.println("Comando incorrecto");
        wait();
      }
    }

    else if (complete[0] == "maxcurrent"){
      if (complete[1].toInt() != 0){
        maxcurrent = complete[1].toInt();
        Serial.println("DONE");
        wait();
      }
      else{
        Serial.println("Comando incorrecto");
        wait();
      }
    }

    else{
      Serial.println("Comando incorrecto");
      wait();
    }  
  }
}

void stimulus(int percentage){
  //genera step
  noInterrupts();
  if (y < v_max && ban_step){
    y += percentage;
    y = constrain(y,0,100);
    y_out = map(y,0,v_max,0,255);
    //Serial.println(y);
    analogWrite(pinMotor,y_out);
  }
  else if(y >= 100 || !ban_step){      
    y -= percentage;
    y = constrain(y,0,100);
    y_out = map(y,0,v_max,0,255);
    //Serial.println(y);
    analogWrite(pinMotor,y_out);
    ban_step = false;
    if(y == 0){
      ban_step = true;
    } 
  }
  interrupts();  
}

void wait(){
  Serial.println(">>");
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
      measurement_i = avg_current/n_samples;
      measurement_vel = avg_vel/n_samples;
      measurement_temp = avg_temp/n_samples;
      measurement_vel = measurement_vel*2*3.1416*0.01/60;
      save_data_MEM(measurement_i,measurement_vel,measurement_temp, measurement_temp);
      // Serial.print(measurement_vel);
      // Serial.println(" VEL m/s ");
      // Serial.print(measurement_i);
      // Serial.print(" A | ");
      // Serial.print(measurement_temp);
      // Serial.println(" C");
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

ISR(TIMER1_COMPA_vect){
  // when actual_time == 1/fs
  TCNT1 = 0;
  update_measurement();
}

void falling_edge_detected()    // Funcion que se ejecuta durante cada interrupcion
{
  contador++;           // Se incrementa en uno el contador
}

void save_data_MEM(float corriente, float rpm, float empuje, float temperatura){
  EEPROM.put(eeAddress, corriente);
  eeAddress += 4;
  EEPROM.put(eeAddress, rpm);
  //Serial.println(EEPROM.get(eeAddress, resultado),3);
  eeAddress += 4;
  EEPROM.put(eeAddress, empuje);
  eeAddress += 4;
  EEPROM.put(eeAddress, temperatura);
  eeAddress += 4;
  apuntador ++;
}

void extract_data_MEM(){
  for (int i = 0; i < EEPROM.length(); i+= 4){  //(apuntador)*4
    Serial.print("Position: ");
    Serial.println(i);
    Serial.println(EEPROM.get(i, resultado),3);
  }
}