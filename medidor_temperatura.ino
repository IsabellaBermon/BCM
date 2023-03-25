//Creamos una variable de tipo entero
int lectura, cont = 0;
int num_muestras = 100;
volatile uint32_t pulse_start_time = 0;  ///< Tiempo de referencia inicio
volatile uint32_t pulse_end_time = 0;  ///< Tiempo de referencia final 

//Creamos una variable de tipo flotante
float temperatura, prom, out = 0.0;

void setup() {
  //Iniciamos la comunicación serial
  Serial.begin(9600);
}

void loop() {
  pulse_end_time = micros();
  if (pulse_end_time > (pulse_start_time + 1000)){
    lectura = analogRead(A0);
    temperatura = lectura * (500.0 / 1023.0);
    prom += temperatura;
    pulse_start_time = micros();
    cont++;
  }
  if(cont >= num_muestras){
    cont = 0;
    out = prom/num_muestras;
    Serial.println(out);
    prom = 0;
  }
}