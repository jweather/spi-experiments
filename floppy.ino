// pinout
int ledPin = 13;
int stepperPinA = 2;
int dirPinA = 3;
int stepperPinB = 4;
int dirPinB = 5;
int stepperSleepA = A2;
int stepperSleepB = A0;

int homeAPin = 10;

void setup() {
  pinMode(ledPin, OUTPUT);

  pinMode(dirPinA, OUTPUT);
  pinMode(stepperPinA, OUTPUT);
  pinMode(stepperSleepA, OUTPUT);
  pinMode(homeAPin, INPUT_PULLUP);
  
  pinMode(dirPinB, OUTPUT);
  pinMode(stepperPinB, OUTPUT);
  pinMode(stepperSleepB, OUTPUT);
  
  digitalWrite(dirPinA, HIGH);
  digitalWrite(dirPinB, HIGH);
  for (int i = 0; i < 80; i++) {
    digitalWrite(stepperPinA, HIGH);
    digitalWrite(stepperPinB, HIGH);
    delayMicroseconds(1);
    digitalWrite(stepperPinA, LOW);
    digitalWrite(stepperPinB, LOW);
    delayMicroseconds(5000);
  }
  digitalWrite(dirPinA, LOW);
  digitalWrite(dirPinB, LOW);
  
  Serial.begin(9600);
  Serial.println("Console");  
}

// timers and counters
int steppedA, steppedB; // did we raise the step pin last loop?
unsigned long intA, intB; // usec interval between steps
unsigned long nextA, nextB; // usec next step scheduled at

char voices[8];
int voiceA = 0, voiceB = 1;
unsigned long lastMove;
int asleep = 0;

unsigned long stepCountA = 0;
unsigned long stepCountB = 0;

int dirA = 0, dirB = 0;

// 24 - 60
unsigned long pitches[256] = {0,115447,108967,102851,97079,91630,86487,81633,77051,72727,68645,64792,61156,57723,54483,51425,48539,45815,43243,40816,38525,36363,34322,32396,30578,28861,27241,25712,24269,22907,21621,20408,19262,18181,17161,16198,15289,14430,13620,12856,12134,11453,10810,10204,9631,9090,8580,8099,7644,7215,6810,6428,6067,5726,5405,5102,4815,4545,4290,4049,3822,3607,3405,3214,3033,2863,2702,2551,2407,2272,2145,2024,1911,1803,1702,1607,1516,1431,1351,1275,1203,1136,1072,1012,955,901,851,803,758,715,675,637,601,568,536,506,477,450,425,401,379,357,337,318,300,284,268,253,238,225,212,200,189,178,168,159,150,142,134,126,119,112,106,100,94,89,84,79,75,71,67,63,59,56,53,50,47,44,42,39,37,35,33,31,29,28,26,25,23,22,21,19,18,17,16,15,14,14,13,12,11,11,10,9,9,8,8,7,7,7,6,6,5,5,5,4,4,4,4,3,3,3,3,3,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// run once per loop()
void driverRun() {
  byte p;
  unsigned long now = micros();
  
  if (intA > 0 || intB > 0) {
    lastMove = millis();
    if (asleep) {
      //Serial.println("wakeup");
      asleep = 0;
      digitalWrite(stepperSleepA, LOW);
      digitalWrite(stepperSleepB, LOW);
    }
  } else if (!asleep && millis() - lastMove > 2000) {
    //Serial.println("sleep is disabled");
    asleep = 1;
//    digitalWrite(stepperSleepA, HIGH);
//    digitalWrite(stepperSleepB, HIGH);
  }    
    
  if (steppedA) {
    digitalWrite(stepperPinA, LOW);
    steppedA = 0;
    
    nextA = now + intA;
  } else if (intA > 0) {
    if (now >= nextA) {
      digitalWrite(stepperPinA, HIGH);
      stepCountA++;
      if (stepCountA % 80 == 0) {
        dirA = !dirA;
        digitalWrite(dirPinA, dirA);
      }
      steppedA = 1;
      lastMove = millis();
    }
  } else {
    nextA = now + intA;
  }    

  if (steppedB) {
    digitalWrite(stepperPinB, LOW);
    steppedB = 0;

    nextB = now + intB;
  } else if (intB > 0) {
    if (now >= nextB) {
      digitalWrite(stepperPinB, HIGH);
      stepCountB++;
      if (stepCountB % 80 == 0) {
        dirB = !dirB;
        digitalWrite(dirPinB, dirB);
      }
      steppedB = 1;
      lastMove = millis();
    }
  } else {
    nextB = now + intB;
  }
}

int blinkstate = 0, blinkstep = 0;
int lastHomeA = 2;
int octave = 0, tuning = -96;
int lastPitch = 65;

void pitch(int p) {
  intA = pitches[p+octave*12] + tuning;
  intB = intA;
  Serial.println(p+octave*12, DEC);
  lastPitch = p;
}

void loop() {
  driverRun();

  int homeA = digitalRead(homeAPin);
  if (homeA != lastHomeA) {
    Serial.print("homeA = ");
    Serial.println(homeA, DEC);
    lastHomeA = homeA;
  }
  
  if (Serial.available() > 0) {
    unsigned char c = Serial.read();
    switch (c) {
#if 0
      case '+':
        octave++;
        Serial.println(octave, DEC);
        pitch(lastPitch);
        break;
      case '-':
        octave--;
        Serial.println(octave, DEC);
        pitch(lastPitch);
        break;
      case '[':
        tuning++;
        Serial.println(tuning, DEC);
        pitch(lastPitch);
        break;
      case ']':
        tuning--;
        Serial.println(tuning, DEC);
        pitch(lastPitch);
        break;
      case 'D':
        dirA = !dirA;
        digitalWrite(dirPinA, dirA);
        Serial.print("dirA = ");
        Serial.println(dirA, DEC);
        break;
      case 's':
        digitalWrite(stepperPinA, HIGH);
        delayMicroseconds(10);
        digitalWrite(stepperPinA, LOW);
        Serial.println("step");
        break;
      case 'a': pitch(45); break;
      case 'b': pitch(47); break;
      case 'c': pitch(48); break;
      case 'd': pitch(50); break;
      case 'e': pitch(52); break;
      case 'f': pitch(53); break;
      case 'g': pitch(55); break;
      case 'x':
 #else
      case 0:
        intA = 0;
        break;
      case 100:
        intB = 0;
        break;
      default:
        if (c >= 24 && c <= 60) {
          intA = pitches[c] + tuning;
        } else if (c >= 124 && c <= 160) {
          intB = pitches[c-100] + tuning;
          blinkstate = !blinkstate;
          digitalWrite(ledPin, blinkstate);
        }
        break;
#endif
    }
  }
  
  if (blinkstep++ > 10000) {
    blinkstep = 0;
  }
}


