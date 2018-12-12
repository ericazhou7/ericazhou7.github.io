#include <Servo.h>

//microphone constants
int micPin = 0;
float minVolt = 1.2;
float maxVolt = 2.7;
float silentThreshold = 1.0;
double volts;
double oldVolts;
const int sampleWindow = 1000; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

//stepper constants
int dirPin = 2;
int stepPin = 4;
float stepperThreshold = 1.0;
int stepperDir = HIGH; //up

//servo constants
int servoPin = 5;
Servo servo; 
int servoDir = 1; //up
int servoAngle = 95;
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
    volts = (peakToPeak * 5.0) / 1024; // convert to volts
    if (oldVolts == NULL) {
      oldVolts = volts;
    }
    
    boolean isLouderThanBefore = (volts - oldVolts) > 0.05;
    boolean notSilent = volts > silentThreshold;
    Serial.print(volts);
    
    if (((volts - maxVolt) > stepperThreshold or ((minVolt - volts) > stepperThreshold)) && notSilent) {
      if ((volts - maxVolt) > stepperThreshold) { //also need to be checking the difference between the old reading and new one
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
      digitalWrite(dirPin, stepperDir);
      Serial.print(stepperDir);
      delay(100);
      for (int i = 0; i < 2048; i++) {   
          digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
          digitalWrite(stepPin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
          delayMicroseconds(500);      // This delay time is close to top speed for this
      }                              // particular motor. Any faster the motor stalls.
    } else if (((volts > maxVolt) or (volts < minVolt)) && notSilent) {
        if (volts > maxVolt) {
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
        if (servoDir == 1) {
          servoAngle -= 5;
          servoAngle = max(servoAngle, servoMin);
        } else {
          servoAngle += 5;
          servoAngle = min(servoAngle, servoMax);
        }
        servo.write(servoAngle);
    } else if (notSilent) {
      Serial.println("VOLUME IN RANGE");
    } else {
      Serial.println("NO SPEECH");
    }
    oldVolts = volts;
    servo.write(servoAngle);
    Serial.println(servoAngle);
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
