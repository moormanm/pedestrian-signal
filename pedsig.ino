//#define DEBUG 1

#include <Adafruit_Soundboard.h>
#include <Wire.h>

//Soundboard related pins
#define SFX_RST 3
#define SFX_ACT 4

//pins that control relays 
#define WALK_PIN 50
#define DWALK_PIN 52

#define PUSH_BUTTON_PIN A7

//sound files

char beginCrossingVoice[] = "VBEG_CRSOGG";
char finCrossingVoice[]   = "VFIN_CRSOGG";

char waitVoices[4][12] = {
                            "VWAIT   OGG",
                            "VWAT2CRSOGG",
                            "VWAT_LNGOGG",
                            "VPLS_WATOGG" 
};
char beep1[]              = "BEEP_1  OGG";
char beep2[]              = "BEEP_2  OGG";
char beep3[]              = "BEEP_3  OGG";

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

boolean playSound(char* soundFile) {
  debugPrintLn("attempting to play sound %s ", soundFile);
  safeStop();
  
  if(!sfx.playTrack(soundFile)) {
    debugPrintLn("failed to play sound %s ", soundFile);
    sfx.reset();
    return false;
  }
  return true;
}
void safeStop() {
  return;
  int isPlaying = digitalRead(SFX_ACT);
  if (isPlaying == LOW) {
    debugPrintLn("trying to stop sounds");
    if(!sfx.stop()) {
        sfx.reset();
        debugPrintLn("stop cmd failed");  
    }
  }
}
void checkActPin() {
  int isPlaying = digitalRead(SFX_ACT);
  debugPrintLn("isPlaying %d", isPlaying);
}
boolean waitButtonIsPressed() {
  int result = digitalRead(PUSH_BUTTON_PIN);
  return result == 0;
}
void powerHand(boolean on) {
  //debugPrintLn("Powering Hand %d", on);
  digitalWrite(DWALK_PIN, on ? HIGH : LOW);
}
void powerMan(boolean on) {
  //debugPrintLn("Powering Man %d", on);
  digitalWrite(WALK_PIN, on ? HIGH : LOW);
}
void setup() {
  
  #ifdef DEBUG
  Serial.begin(9600);
  #endif 

  //Soundboard init
  Serial1.begin(9600); 
  
  if (!sfx.reset()) {
    while (1); //Hang if the soundboard could not be reset successfully 
  }

  //Configure the activity detection pin for the sfx card - this is a workaround to be able to send a 'stop' command if a sound is currently playing
  pinMode(SFX_ACT, INPUT_PULLUP);


  
  pinMode(WALK_PIN, OUTPUT);
  pinMode(DWALK_PIN, OUTPUT);


  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);

  //Start init sequence for pedestrian signal device
  powerHand(false);
  powerMan(false);
  powerHand(true);
  delay(500);
  powerHand(false);
  powerMan(true);
  delay(500);
  
}



boolean lastPressedState;
boolean waitButtonWasPressed;
void updateWaitButtonPressedState() {
  waitButtonWasPressed = false;
  if(waitButtonIsPressed() ) {
    if(!lastPressedState) {
      debugPrintLn("Button pressed");
      waitButtonWasPressed = true;    
    }
    lastPressedState = true;
  }
  else {
    if(lastPressedState) {
      debugPrintLn("Button released");
    }
    lastPressedState = false;
  }
}



enum Phase { WALK, FINISH_WALKING, DONT_WALK };
#define WALK_DURATION 16000
#define FINISH_WALKING_DURATION 35100
#define DONT_WALK_DURATION 20000


Phase currentPhase = DONT_WALK;
void loop() {
  delay(10);
  //checkActPin();
  updateWaitButtonPressedState();
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
#define BEEP_DELAY 2000
unsigned long lastBeepTime;
void initWalkPhase() {
   powerHand(false);
   powerMan(true);
   playSound(beginCrossingVoice);
   lastBeepTime = millis();
}
void walkPhase() {
  if(waitButtonWasPressed) { 
     playSound(beginCrossingVoice);
     lastBeepTime = millis();   
  }
  if(millis() - lastBeepTime > BEEP_DELAY) {
    playSound(beep1);
    lastBeepTime = millis();
  }
}

unsigned long lastFlipTime;
#define FLIP_DELAY 498
boolean handIsFlippedOn = false;

#define START_PHASE_DELAY 4000
unsigned long initPhaseTime;
void initFinishWalkingPhase() {
  powerHand(false);
  powerMan(false);
  handIsFlippedOn = false;
  lastFlipTime = 0;
  lastBeepTime = millis();
  initPhaseTime = millis();
  playSound( finCrossingVoice );
  
}

#define FINISH_CROSSING_BEEP 1000
void finishWalkingPhase() {
 
  boolean shouldFlip =  millis() - lastFlipTime > FLIP_DELAY;
  if(shouldFlip) {
      powerHand(!handIsFlippedOn); 
      handIsFlippedOn = !handIsFlippedOn;
      lastFlipTime = millis();
  }

  if(waitButtonWasPressed) {
  
     playSound(nextWaitButtonSound());
     lastBeepTime = millis();   
  }
  
  if(millis() - lastBeepTime > FINISH_CROSSING_BEEP && millis() - initPhaseTime > START_PHASE_DELAY ) {
    if(playSound(beep2)) {
       lastBeepTime = millis();      
    }
  }

  
}

void initDontWalkPhase() {
  powerHand(true);
  powerMan(false);
  lastFlipTime = 0;
  lastBeepTime = millis();
}

int waitSoundIdx=0;
char* nextWaitButtonSound() {
  return waitVoices[ waitSoundIdx++ % 4 ];
}


void dontWalkPhase() {
  if(waitButtonWasPressed) {
     playSound(nextWaitButtonSound());
     lastBeepTime = millis();    
  }


  if(millis() - lastBeepTime > BEEP_DELAY) {
    if(playSound(beep3)) {
      lastBeepTime = millis();  
    }
  }
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
