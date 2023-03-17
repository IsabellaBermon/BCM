bool LED_STATE = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000100; //Set prescalar to 256
  TIMSK1 |= B00000010; //Enable compare match A
  OCR1A = 31250;
  sei();
}

void loop() {
  // main code here
}

ISR(TIMER1_COMPA_vect){
  TCNT1 = 0;
  LED_STATE = !LED_STATE;
  digitalWrite(LED_BUILTIN, LED_STATE);
  if (LED_STATE == true) {
    Serial.println("LED turned on!");
  }
}
