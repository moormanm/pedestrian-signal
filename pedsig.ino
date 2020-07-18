#define WALK 50
#define DWALK 52
void setup() {
  
  // put your setup code here, to run once:
  pinMode(WALK, OUTPUT);
  pinMode(DWALK, OUTPUT);
  digitalWrite(WALK, LOW);
  digitalWrite(DWALK, LOW);
  delay(5000);

  
}

void loop() {
  
 
  for(int j=0; j<17;j++) {
     digitalWrite(DWALK, HIGH);
     delay(500);
     digitalWrite(DWALK, LOW);
     delay(500);  
  }
  digitalWrite(DWALK, HIGH);
  delay(10000);
  digitalWrite(DWALK, LOW);

  digitalWrite(WALK, HIGH);
  delay(10000);
  digitalWrite(WALK, LOW);

}
