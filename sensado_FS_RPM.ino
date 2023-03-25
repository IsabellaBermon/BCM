bool LED_STATE = false;
volatile int contador = 0;   // Variable para freq de RPM
volatile uint32_t contador_copy = 0; //Valor del contador de pulsos por segundo
volatile uint32_t pulse_start_time = 0;  // Tiempo de referencia inicio
volatile uint32_t pulse_end_time = 0;  // Tiempo de referencia final
const byte interruptPin = 2; ///< Pin 2 para interrupciones, el arduino UNO attachInterrupt solo sirve en 2 y 3

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin),interrupcion0,FALLING);  // Interrupcion 0 (pin2) 
  Serial.begin(57600);
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000100; //Set prescalar to 256
  TIMSK1 |= B00000010; //Enable compare match A
  OCR1A = 31250;
  sei();
}

void loop() {
  pulse_end_time = millis(); 
  noInterrupts(); 
  if (pulse_end_time > (pulse_start_time + 1000)) {    
    contador_copy = contador*30;   
    contador = 0;
    pulse_start_time = millis();    
  }
  interrupts();
}

ISR(TIMER1_COMPA_vect){
  TCNT1 = 0;
  LED_STATE = !LED_STATE;
  digitalWrite(LED_BUILTIN, LED_STATE);
  Serial.print(contador_copy);
  Serial.println(" RPM");
}

void interrupcion0()    // Funcion que se ejecuta durante cada interrupcion
{
  contador++;           // Se incrementa en uno el contador
}
