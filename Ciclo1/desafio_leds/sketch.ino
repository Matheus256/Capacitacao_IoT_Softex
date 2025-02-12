#define LED1 2
#define LED2 4
#define LED3 5
#define LED4 17

#define BUTTON 32

int8_t v;

void setup() {
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(LED4,OUTPUT);
  pinMode(BUTTON, INPUT);
  v=0;
}

void loop() {
  if (digitalRead(BUTTON)==HIGH){
    digitalWrite(LED1,(v>>0)&1);
    digitalWrite(LED2,(v>>1)&1);
    digitalWrite(LED3,(v>>2)&1);
    digitalWrite(LED4,(v>>3)&1);
  }
  else {
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    digitalWrite(LED3,LOW);
    digitalWrite(LED4,LOW);
    v=v+1;
  }
}
