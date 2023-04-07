 /**
  * @file Interfaz.ino
  *
  * @mainpage Banco de pruebas de caracterización de motores y hélices 
  *
  * @authors Creado por: Isabella Bermón Rojas y
  *                      Daniela Cuartas Marulanda.
  *
  * @date 6/04/2023
  *
  * @section descripción Descripción
  * Interfaz de usuario de control del banco de pruebas de caracterización de motores y hélices con validación de comandos.
  * Se encarga de sensar corriente, temperatura, velocidad y empuje del motor en tiempo real, además de mostrar el duty de la señal generada.
  * Genera dos tipos de excitación al motor: Señal triangular y señal step de escalones continuos.
  *
  * @section circuito Circuito
  * Pines análogos:
  * A0 : entrada sensor de corriente
  * A1 : entrada sensor de temperatura
  * A4 : pin Dout celda de carga
  * A5 : pin SCK celda de carga
  * 
  * Pines digitales:
  * pin 2: pin de interrupción para medir rpm
  * pin 6: pin de generación de estímulo al motor
  *
  * @section librerías Librerías
  * Celda de carga: https://github.com/whandall/NBHX711
  * Desarollo en https://github.com/IsabellaBermon/BCM.git
  *
  */

  #include <String.h>   
  #include <stdio.h>
  #include <ctype.h>
  #include <EEPROM.h>
  #include <NBHX711.h>

  // Variables de guardado en memoria
  uint16_t eeAddress;
  float resultado;

  // Variables para generar señal triangular
  uint32_t t_actual = 0;
  uint32_t t_anterior = 0;
  uint32_t v_max = 100;
  uint32_t period = 500000; // en us
  int32_t m = v_max*1000000/(period/2);
  int32_t b = 0;
  int32_t number_points = 1;
  int32_t acum = 0;
  float y, y_out = 0;
  bool ban_step, ban_tri = true;
  bool ban_stimulus = false;

  // Variables para generar señal step
  int8_t percentage = 50;

  // Variables default de interfaz
  uint16_t average = 1; // número de muestras
  uint8_t maxcurrent = 1; // 1 A
  bool verbose = false; // verbose = off
  uint8_t mode = 1; //1 = step, 2 = triangular
  uint32_t fsample = 200; // Hzz
  uint32_t ftime = 16000000/256; // f pre-escalador 
  uint16_t sample_counter = 0;
  uint8_t duty = 0;
  bool verbose_flag = false; // bandera para guardar o mostrar
  
  // Verificacion de comandos
  bool incorrect = false;
  const int MAX_WORDS = 3;   ///< Constante global que determina el tamaño de la lista de elementos
  String complete[MAX_WORDS]; ///< Lista de máximo dos elementos para verificar válidez del comando

  // Sensado de señales -------------------------
  // Clock variables
  volatile uint32_t pulse_start_time = 0;  // Tiempo de referencia inicio
  volatile uint32_t pulse_end_time = 0;  // Tiempo de referencia final
  // Pines de sensores y motor para la generación
  int acs = A0;
  int lm35 = A1;
  const int LOADCELL_DOUT_PIN = A4;
  const int LOADCELL_SCK_PIN = A5;
  NBHX711 hx711(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); // Instanciación celda de carga
  const byte interruptPin = 2; //< Pin 2 para interrupción de rpm
  int pinMotor = 6; // generación de señal motor
  
  // Variables de cada sensor ----------------------
  
  // Variables sensor de corriente
  float actual_current, avg_current, measurement_i = 0;
  
  // Variables sensor de temperatura
  float actual_temp, avg_temp, measurement_temp = 0;
  
  // Variables sensor de rpm
  volatile int contador = 0;   // Valor del contador de pulsos por segundo
  uint16_t actual_vel, actual_vel_freq, avg_vel = 0;
  uint16_t measurement_vel = 0;
  
  // Variable sensor de empuje
  float actual_thrust, avg_thrust, measurement_thrust = 0;

  // Variables posición de cada medición
  int x_=0; // current (4 bytes)
  int y_=4; // rpm(2 bytes)
  int w_=6;  // thrust (4 bytes)
  int z_= 10; // temperature (4 bytes)
  int p_= 14; // duty (1 byte)
  int u= 0; // index de memoria

  /**
   * Se inicializan los pines del motor y los sensores.
   * Se inicializa objeto de celda de carga 
   * Se inicializa el pin de interrupción de rpm
   * Se configura el reloj interno del arduino y el preescalador para interrupción de frecuencia de muestreo.
   * Se inicializa el monitor serial   
   */
  void setup(){
    pinMode(pinMotor,OUTPUT);
    pinMode(acs,INPUT);
    pinMode(lm35,INPUT);
    hx711.begin();
    hx711.setScale(16610.857);
    hx711.tare(10);
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

  /**
   * Recibe los comandos por el puerto serial y hace las respectivas validaciones de comandos correctos o incorrectos.
   * Genera el estímulo seleccionado al motor.   
   */  
  void loop() {
    t_actual = micros();
    // Genera señal triangular
    if(t_actual >= t_anterior + period/number_points){
      noInterrupts();
      t_anterior = micros();
      if (ban_stimulus){
        if (mode == 2){
          acum += period/number_points;
          y = m*(float(acum)/1000000) + b;  // m = v/s
          y = constrain(y,0,100);
          y_out = map(int(y*100),0,v_max*100,0,255);
          duty = y_out*100/255;
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
          // Genera señal step 
          stimulus(percentage);
        }
      }
      interrupts();
    }

    // Gurda/Imprime los valores sensados can n muestras
    if (verbose_flag){
      if (verbose) {
        //Serial.print(" I: ");
        Serial.print(measurement_i,3);
        Serial.print(" A | ");
        Serial.print(" V: ");
        Serial.print(measurement_vel);
        Serial.print(" m/s | ");    
        Serial.print(" E: ");
        Serial.print(measurement_thrust,3);
        Serial.print(" N | ");
        Serial.print("T: ");
        Serial.print(measurement_temp);
        Serial.print(" C | ");   
        Serial.print(" Duty: ");
        Serial.println(" %"); 
      }
      else {
        save_data_MEM(measurement_i,measurement_vel,measurement_thrust,measurement_temp,duty);
      }
      verbose_flag = false; 
    }

    // Sensa los rpm por medio de interrupcion con ventana de 1 segundo
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

      // Inicia el estímulo al motor
      if(command == "start"){    
        ban_stimulus = true;  
        Serial.println("STIMULUS ACTIVATED");
        wait();
      }
      
      // Detiene el estímulo al motor      
      else if (command == "stop"){
        ban_stimulus = false;
        analogWrite(pinMotor,0);
        Serial.println("STIMULUS DISABLED");
        wait();
      }
      
      // Imprime el estado actual de la configuración del banco de pruebas      
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
      
      // Asigna forma de visualizar las variables sensadas
      else if (command == "fdata"){
        noInterrupts();
        extract_data_MEM();
        interrupts();      
        wait();
      }

      // Pone variables por defecto
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
      
      // Asigna forma de visualizar las variables sensadas
      else if (command == "verbose on"){
        verbose = true;      
        wait();
      }

      // Asigna forma de visualizar las variables sensadas 
      else if (command == "verbose off"){
        verbose = false; 
        Serial.println("DONE");
        wait();   
      }
      
      // Asigna el modo del estímulo
      else if (command == "mode trg"){       
        mode = 2;
        number_points = 1000;
        y = 0;
        Serial.println("DONE");
        wait();
      }

      // Verifica el resto de comandos
      else{
        verify(command);
      }
    }
    
  }

  /**
    * Verifica el comando ingresado por consola y hace asignaciones a variables.
    *
    * @param command  Cadena de texto ingresada por consola.
    */
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
      // Verifica que el comando no sea mayor a tres palabras
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
    // Solo entra si el comando tiene tres palabras
    if (!incorrect){
      
      // Asigna el modo del estímulo
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

      // Asigna el periodo de la señal
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

      // Asigna frecuencia de muestreo      
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
      
      // Asigna número de muestras a promediar
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

      // Asigna valor máximo de corriente
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

  /**
    * Genera el estímulo de la señal step de escalones continuos
    *
    * @param percentage  porcentaje de cambio en y del estímulo
    */
  void stimulus(int percentage){
    //genera step
    noInterrupts();
    if (y < v_max && ban_step){
      y += percentage;
      y = constrain(y,0,100);
      y_out = map(y,0,v_max,0,255);
      duty = y_out*100/255;
      analogWrite(pinMotor,y_out);
    }
    else if(y >= 100 || !ban_step){      
      y -= percentage;
      y = constrain(y,0,100);
      y_out = map(y,0,v_max,0,255);
      duty = y_out*100/255;
      analogWrite(pinMotor,y_out);
      ban_step = false;
      if(y == 0){
        ban_step = true;
      } 
    }
    interrupts();  
  }

  /**
    * Imprime por consola los caracteres ">>".
    *
    */
  void wait(){
    Serial.println(">>");
  }

  /**
    * Sensa la corriente.
    *
    */
  float sense_current(){
    float sens_current = 0.185;
    float voltaje = analogRead(acs) * (5.0 / 1023.0);  
    float current = abs(voltaje -2.48)/sens_current-0.12 ; // calculate current 
    return abs(current);
  }

  /**
    * Sensa el empuje.
    *
    */
  float sense_thrust(){
    hx711.update();
    float thrust = (hx711.getUnits(10)-7.00)*35 ;
    // Al valor de empuje obtenido se resta el peso del motor
    // y multiplica por gravedad para medición en Newtons
    thrust = (abs(thrust -6.23)/1000)*9.8;    
    return thrust;
  }

  /**
    * Sensa la temperatura.
    *
    */
  float sense_temperature(){
    float voltage = analogRead(lm35) * (5.0 / 1023.0);
    float temp = voltage * 100;
    return temp;
  }

  /**
    * Actualiza los valores de las variables sensadas y activa la bandera para visualizar o guardar en EEPROM.
    *
    */
  void update_measurement(){
    // Actualiza medida cuando se cumpla el número de muestras promediando
    if (sample_counter == average){
        measurement_i = avg_current/average;
        measurement_vel = avg_vel/average;
        measurement_temp = avg_temp/average;
        measurement_thrust = actual_thrust/average;
        // Para obtener la velocidad en base RPM se hace la siguiente conversión
        //measurement_vel = measurement_vel*2*3.1416*0.01/60;
        verbose_flag = true;
        avg_current = 0;
        avg_vel = 0;
        avg_temp = 0;
        actual_thrust = 0;
        sample_counter = 0;
    }
    else {
      // Sensa todas las variables
      actual_current = sense_current();
      delayMicroseconds(104); // espera los ciclos de reloj para que el ADC mida correctamente
      actual_vel = actual_vel_freq;  
      actual_thrust = sense_thrust();
      actual_temp = sense_temperature();
      // Actualiza el promedio sensado
      avg_current += actual_current;   
      avg_vel += actual_vel;
      avg_thrust += actual_thrust;
      avg_temp += actual_temp;
      sample_counter += 1;    
    }
  }

  /**
    * Interrupción del reloj interno del arduino para medir cuando se cumple la frecuencia de muestreo.
    *
    */
  ISR(TIMER1_COMPA_vect){
    TCNT1 = 0;
    update_measurement();
  }

  /**
    * Cuenta las interrupciones del rpm
    *
    */
  void falling_edge_detected()   
  {
    contador++;           
  }

  /**
    * Guarda los valores sensados en la memoria EEPROM para visualización postergada
    *
    * @param corriente
    * @param rpm
    * @param empuje
    * @param temperatura
    * @param duty
    */
    void save_data_MEM(float corriente, uint16_t rpm, float celda, float temperatura, uint8_t pwm){
      EEPROM.put(x_, corriente);
      EEPROM.put(y_, rpm);
      EEPROM.put(z_, celda);
      EEPROM.put(w_, temperatura);  
      EEPROM.put(p_, pwm);
      x_ += 15;
      y_ += 15;
      w_ += 15;
      z_ += 15;
      p_ += 15;
      if(x_ == 1020){
        x_=0; 
        y_=4; 
        w_=6;  
        z_ = 10; 
        p_ = 14; 
      } 
    }

  /**
    * Extrae las variables de la memoria EEPROM
    *
    */
    void extract_data_MEM(){
      u = 0;
      while(u<1020){
        u +=1;
        Serial.print(EEPROM.get(x_, measurement_i),3);
        Serial.print(", ");
        Serial.print(EEPROM.get(y_, measurement_vel));
        Serial.print(", ");
        Serial.print(EEPROM.get(z_, measurement_thrust),3);
        Serial.print(", ");
        Serial.print(EEPROM.get(w_, measurement_temp),3);
        Serial.print(", ");
        Serial.println(EEPROM.get(p_, duty));
        x_ += 15;
        y_ += 15;
        w_ += 15;
        z_ += 15;
        p_ += 15;
        if(x_ == 1020){
          u = 1020; // guarda hasta el máximo espacio
          x_= 0; 
          y_= 4; 
          w_= 6;  
          z_ = 10;
          p_ = 14;
        }
      }
    }
