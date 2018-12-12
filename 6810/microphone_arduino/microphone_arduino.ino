#include <Servo.h>

//microphone constants
int micPin = 0;
float minVolt = 1.2;
float maxVolt = 2.7;
float silentThreshold = 1.0;
const int sampleWindow = 1000; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
double volts;
double oldVolts;

//stepper constants
int dirPin = 2;
int stepPin = 4;
float stepperThreshold = 1.0;
int stepperDir = LOW; //up

//servo constants
int servoPin = 5;
Servo servo; 
int servoDir = 1; //up
int servoAngle = 155;
int servoMin = 95;
int servoMax = 155;

void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  servo.attach(servoPin); 
  servo.write(servoMax);
  Serial.begin(9600);
}

void loop() {

    unsigned int peakToPeak = sampleAudio(sampleWindow);
    volts = convertAnalogToVolts(peakToPeak);
    if (oldVolts == NULL) { //if this is the first reading, set oldVolts to whatever is read
      oldVolts = volts;
    }
    
    boolean isLouderThanBefore = (volts - oldVolts) > 0.05;
    boolean notSilent = volts > silentThreshold;
    boolean aboveStepperThreshold = (volts - maxVolt) > stepperThreshold; //if the volts reading differs from the upper
                                                                          //and lower bounds for acceptable volume voltages
                                                                          //by more than stepperThreshold, the stepper will turn.
    boolean belowStepperThreshold = (minVolt - volts) > stepperThreshold;
    boolean isTooLoud = volts > maxVolt;
    boolean isTooSoft = volts < minVolt;
    Serial.print(volts);
    
    if ((aboveStepperThreshold or belowStepperThreshold) && notSilent) {
      
      if (aboveStepperThreshold) { //also need to be checking the difference between the old reading and new one
        Serial.println(" TOO TOO LOUD");
        if (isLouderThanBefore) { //if louder than before AND still too loud, need to move in opposite direction
          stepperDir = changeStepperDir(stepperDir);
        }
      } else {
        Serial.println(" TOO TOO SOFT");
        if (!isLouderThanBefore) { //if softer than before AND still too soft, need to move in opposite direction
          stepperDir = changeStepperDir(stepperDir);
        }
      } 
      moveStepper(stepperDir);
      
    } else if ((isTooLoud or isTooSoft) && notSilent) {
      
        if (isTooLoud) {
          Serial.println(" TOO LOUD");
          if (isLouderThanBefore) { //change direction
             servoDir = changeServoDir(servoDir);
          }
        } else {
          Serial.println(" TOO SOFT");
          if (!isLouderThanBefore) { //change direction
             servoDir = changeServoDir(servoDir);
          }
        }
        moveServo(servoDir);
        
    } else if (notSilent) {
      
      Serial.println(" VOLUME IN RANGE");
      
    } else {
      
      Serial.println(" NO SPEECH");
      
    }
    oldVolts = volts;
}


//helper functions

void moveServo(int servoDir) {
  //turns the servo 5 degrees in direction servoDir
  if (servoDir == 1) {
    servoAngle -= 5;
    servoAngle = max(servoAngle, servoMin);
  } else {
    servoAngle += 5;
    servoAngle = min(servoAngle, servoMax);
  }
  servo.write(servoAngle);
}

void moveStepper(int stepperDir) {
  //turns the stepper a small distance in direction stepperDir
  digitalWrite(dirPin, stepperDir);
  Serial.print(stepperDir);
  delay(100);
  for (int i = 0; i < 2048; i++) {   
      digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
      digitalWrite(stepPin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
      delayMicroseconds(500);      // This delay time is close to top speed for this
   }                              // particular motor. Any faster the motor stalls.
}

int sampleAudio(int sampleWindow) {
    unsigned long startMillis = millis(); // Start of sample window
    unsigned int peakToPeak = 0; // peak-to-peak level
    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;
    // collect data for 50 mS
    while (millis() - startMillis < sampleWindow) {
      sample = analogRead(micPin);
      if (sample < 1024) {
        if (sample > signalMax) {
          signalMax = sample; // save just the max levels
        } else if (sample < signalMin) {
          signalMin = sample; // save just the min levels
        }
      }
    }
    peakToPeak = signalMax - signalMin; // max - min = peak-peak amplitude
    return peakToPeak;
}

double convertAnalogToVolts(unsigned int peakToPeak) {
  return (peakToPeak * 5.0) / 1024; // convert to volts
}

int changeServoDir(int servoDir) {
  if (servoDir == 1) {
      servoDir = -1;
  } else {
      servoDir = 1;
  }
}

int changeStepperDir(int stepperDir) {
  if (stepperDir == 1) {
      stepperDir = 0;
  } else {
      stepperDir = 1;
  }
}
