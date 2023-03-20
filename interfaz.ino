#include <String.h>   
#include <stdio.h>
#include <ctype.h>

// para generar señal triangular
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
// step
int32_t percentage = 50;
float y_out = 0;

int average = 1;
int maxcurrent = 1; //1A
bool verbose = false; //verbose = off
int mode = 1; //1 = step, 2 = triangular
int fsample = 200; //hz

bool incorrect = false;
const int MAX_WORDS = 3;   ///< Constante global que determina el tamaño de la lista de elementos
String complete[MAX_WORDS]; ///< Lista de máximo dos elementos para verificar válidez del comando

void setup(){
  Serial.begin(9600);  
  wait();
}

void loop() {
  t_actual = micros();
  if(t_actual >= t_anterior + period/number_points){
    t_anterior = micros();
    if (ban_stimulus){
      if (mode == 2){
        acum += period/number_points;
        y = m*(float(acum)/1000000) + b;  // m = v/s
        y = constrain(y,0,100);
        y_out = map(int(y*100),0,v_max*100,0,255);
        Serial.println(y_out);
        analogWrite(6,y_out);
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
        stimulus(mode, percentage);
      }
    }
  }
  if(Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.toLowerCase();
    Serial.println(command);
    
    if(command == "start"){    
      ban_stimulus = true;  
      Serial.println("STIMULUS ACTIVATED");
      wait();
    }

    else if (command == "stop"){
      ban_stimulus = false;
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
      //imprime la tabla de datos de la memoria
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
      Serial.println("DONE");      
      wait();
    }

    else if (command == "verbose off"){
      verbose = false; 
      Serial.println("DONE");
      wait();   
    }

    else if (command == "mode trg"){ 
      ban_stimulus = false;      
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
          ban_stimulus = false;
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

void stimulus(int mode, int percentage){
  if (mode == 1){
    //genera step
    if (y < v_max && ban_step){
      y += percentage;
      y = constrain(y,0,100);
      y_out = map(y,0,v_max,0,255);
      Serial.println(y_out);
      analogWrite(6,y_out);
    }
    else if(y >= 100 || !ban_step){      
      y -= percentage;
      y = constrain(y,0,100);
      y_out = map(y,0,v_max,0,255);
      Serial.println(y_out);
      analogWrite(6,y_out);
      ban_step = false;
      if(y == 0){
        ban_step = true;
      } 
    }   
  }
  else{
    //no genera
  }
}

void wait(){
  Serial.println(">>");
}
