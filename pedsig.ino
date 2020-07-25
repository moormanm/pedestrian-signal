#define DEBUG 1

#include <Adafruit_Soundboard.h>
#include <Wire.h>

//Soundboard related pins
#define SFX_RST 3
#define SFX_ACT 4

//pins that control relays 
#define WALK_PIN 50
#define DWALK_PIN 52


//sound files
char doorClosingFile[] = "CLSEDOORWAV";
char nudgeModeFile[] = "NUDGEMDEOGG";
char helpFireFile[] = "HELPFIREWAV";
char normalChimeFile[] = "PASSCHMEWAV";
char dieselDucyFile[] = "DIESELDUWAV";

//Soundboard
Adafruit_Soundboard sfx = Adafruit_Soundboard(&Serial1, &Serial, SFX_RST);

void debugPrintLn(const char* fmt, ...) {
  #ifdef DEBUG
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf,fmt, args);
    Serial.println(buf);
    va_end(args);
  #endif
}


void powerHand(boolean on) {
  debugPrintLn("Powering Hand %d", on);
  digitalWrite(DWALK_PIN, on ? HIGH : LOW);
}
void powerMan(boolean on) {
  debugPrintLn("Powering Man %d", on);
  digitalWrite(WALK_PIN, on ? HIGH : LOW);
}
void setup() {
  
  #ifdef DEBUG
  Serial.begin(9600);
  #endif 

  //Soundboard init
  Serial1.begin(9600); 
  
  //if (!sfx.reset()) {
  //  while (1); //Hang if the soundboard could not be reset successfully 
  //}

  //Configure the activity detection pin for the sfx card - this is a workaround to be able to send a 'stop' command if a sound is currently playing
  pinMode(SFX_ACT, INPUT_PULLUP);

  pinMode(WALK_PIN, OUTPUT);
  pinMode(DWALK_PIN, OUTPUT);


  //Start init sequence for pedestrian signal device
  powerHand(false);
  powerMan(false);
  powerHand(true);
  delay(500);
  powerHand(false);
  powerMan(true);
  delay(500);
  
}




enum Phase { WALK, FINISH_WALKING, DONT_WALK };
#define WALK_DURATION 5000
#define FINISH_WALKING_DURATION 15000
#define DONT_WALK_DURATION 15000


Phase currentPhase = FINISH_WALKING;
void loop() {
  if(shouldChangePhases()) {
     changePhases();
     debugPrintLn("changed to phase: %d", currentPhase);    
  }
 
  switch(currentPhase) {
    case WALK: walkPhase(); break;
    case FINISH_WALKING: finishWalkingPhase(); break;
    case DONT_WALK: dontWalkPhase(); break;
  }
}

void initWalkPhase() {
   powerHand(false);
   powerMan(true);
  
}
void walkPhase() {
  
}

unsigned long lastFlipTime;
#define FLIP_DELAY 498;
boolean handIsFlippedOn = false;


void initFinishWalkingPhase() {
  powerHand(false);
  powerMan(false);
  handIsFlippedOn = false;
  lastFlipTime = 0;
}

void finishWalkingPhase() {
  boolean shouldFlip =  millis() - lastFlipTime > FLIP_DELAY;
  if(shouldFlip) {
      powerHand(!handIsFlippedOn); 
      handIsFlippedOn = !handIsFlippedOn;
      lastFlipTime = millis();
  }
}

void initDontWalkPhase() {
  powerHand(true);
  powerMan(false);
  
}
void dontWalkPhase() {
  
}


unsigned long phaseStartTimeMillis;
unsigned long phaseDuration;


boolean shouldChangePhases() {
   return millis() - phaseStartTimeMillis > phaseDuration;
}

void changePhases() {
  phaseStartTimeMillis = millis();
  switch(currentPhase) {
    case WALK: 
      currentPhase = FINISH_WALKING;
      phaseDuration = FINISH_WALKING_DURATION;
      initFinishWalkingPhase();      
      break;
    case FINISH_WALKING:
      currentPhase = DONT_WALK;
      phaseDuration = DONT_WALK_DURATION;
      initDontWalkPhase();      
      break;
    case DONT_WALK:
      currentPhase = WALK;
      phaseDuration = WALK_DURATION;
      initWalkPhase();      
      break; 
  }
}

