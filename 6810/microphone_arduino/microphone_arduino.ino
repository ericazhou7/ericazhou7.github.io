#include <Servo.h>

int micPin = 0;
int dirPin = 2;
int stepPin = 3;
int servoPin = 8;
float minVolt = 0.5;
float maxVolt = 0.9;
float stepperThreshold = 0.0;
double volts;
double oldVolts;

const int sampleWindow = 1000; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

Servo myservo; 
int servoPos = 0;

void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  myservo.attach(servoPin); 
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
    volts = (peakToPeak * 5.0) / 25; // convert to volts
    if (oldVolts == NULL) {
      oldVolts = volts;
    }
    
    boolean isLouderThanBefore = (oldVolts - volts) > 0;
    boolean notSilent = volts > 0.05;
    Serial.print(volts);
    
    if ((volts - maxVolt) > stepperThreshold or ((minVolt - volts) > stepperThreshold) && notSilent) {
        if ((volts - maxVolt) > stepperThreshold) { //also need to be checking the difference between the old reading and new one
            Serial.println(" TOO TOO LOUD");
            if (isLouderThanBefore) { //if louder than before AND still too loud, need to move in opposite direction
              digitalWrite(dirPin, LOW);
            } else {  //if softer than before AND too loud, getting closer! move in same direction
              digitalWrite(dirPin, HIGH); 
            }    
         } else {
            Serial.println(" TOO TOO SOFT");
            if (isLouderThanBefore) { //if louder than before AND too soft, getting closer! move in same direction
              digitalWrite(dirPin, LOW);
            } else {  //if softer than before AND still too soft, need to move in opposite direction
              digitalWrite(dirPin, LOW);
            }
        } 
        delay(100);
        for (int i = 0; i < 1024; i++) {   
            digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
            digitalWrite(stepPin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
            delayMicroseconds(500);      // This delay time is close to top speed for this
        }                              // particular motor. Any faster the motor stalls.
    } else if (volts > maxVolt or volts < minVolt) {
        if (volts > maxVolt) {
          Serial.println(" TOO LOUD");
          //myservo.write(servoPos+1);
        } else {
          Serial.println(" TOO SOFT");
          //myservo.write(servoPos-1);
        }
    } else {
      Serial.println("VOLUME IN RANGE");
    }
    oldVolts = volts;
}
