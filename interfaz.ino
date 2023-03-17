#include <String.h>   
#include <stdio.h>
#include <ctype.h>

int period = 0.5; //500ms
int average = 1;
int maxcurrent = 1; //1A
bool verbose = false; //verbose = off
int mode = 0; //1 = step, 2 = triangular
int fsample = 200; //hz
int percentage = 50;

bool incorrect = false;
const int MAX_WORDS = 3;   ///< Constante global que determina el tamaño de la lista de elementos
String complete[MAX_WORDS]; ///< Lista de máximo dos elementos para verificar válidez del comando

void setup(){
  Serial.begin(9600);
  wait();
}

void loop() {
  if(Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.toLowerCase();
    Serial.println(command);
    
    if(command == "start"){      
      period = 0.5; //500ms
      average = 1;
      maxcurrent = 1; //1A
      verbose = false; //verbose = off
      fsample = 200;
      mode = 1; //1 = step, 2 = triangular
      percentage = 50;
      stimulus(mode, percentage);
      Serial.println("STIMULUS ACTIVATED");
      wait();
    }

    else if (command == "stop"){
      mode = 0;
      percentage = 0;
      stimulus(mode, percentage);
      Serial.println("STIMULUS DISABLED");
      wait();
    }

    else if (command == "state"){
      Serial.print("MODE ");
      Serial.println(mode);
      Serial.print("PERIOD ");
      Serial.println(period);
      Serial.print("FREQUENCY ");
      Serial.println(1/period);
      Serial.print("AVERAGE ");
      Serial.println(average);
      Serial.print("VERBOSE ");
      if (verbose){
        Serial.println("ON");      
      }
      else{
        Serial.println("OFF");
      }
      Serial.print("MAXCURRENT ");
      Serial.println(maxcurrent);
      wait();    
    }

    else if (command == "fdata"){
      //imprime la tabla de datos de la memoria
      wait();
    }

    else if (command == "reset"){
      period = 0.5; //500ms
      average = 1;
      maxcurrent = 1; //1A
      verbose = false; //verbose = off
      fsample = 200;
      percentage = 50;
      mode = 1; //1 = step, 2 = triangular
      wait();
    }

    else if (command == "verbose on"){
      verbose = true;
      Serial.println("verbose true");      
      wait();
    }

    else if (command == "verbose off"){
      verbose = false; 
      Serial.println("verbose false");
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
          percentage = complete[2].toInt(); 
          Serial.println("percentage yes");
          wait();
        }
        else{
          Serial.println("Comando incorrecto");
          wait();
        }
      }
      else if (complete[1] == "trg"){
        //triangular
        Serial.println("tri yes");
        wait();
      }
      else{
        Serial.println("Comando incorrecto");
        wait();
      }
    }

    else if (complete[0] == "period"){
      if (complete[1].toInt() != 0){
        period = complete[1].toInt();
        Serial.println("period yes");
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
        Serial.println("fsample yes");
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
        Serial.println("average yes");
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
        Serial.println("maxcurrent yes");
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
  }
  else if (mode == 2){
    //genera triangular
  }
  else{
    //no genera
  }
}

void wait(){
  Serial.println(">>");
}
