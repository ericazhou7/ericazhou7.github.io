#include <Servo.h>

int micPin = 0;
int dirPin = 2;
int stepPin = 3;
int servoPin = 5;
float minVolt = 0.6;
float maxVolt = 1.7;
float motorThreshold = 0.2;

const int sampleWindow = 1000; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

Servo servo; 
int servoAngle = 155;
int servoMin = 110;
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
    double volts = (peakToPeak * 5.0) / 1024; // convert to volts
    Serial.print(volts);
    if ((volts - maxVolt) > motorThreshold or (minVolt - volts) > motorThreshold) {
        if ((volts - maxVolt) > motorThreshold) {
            Serial.println(" TOO TOO LOUD");
            digitalWrite(dirPin, HIGH);     
         } else {
            Serial.println(" TOO TOO SOFT");
            digitalWrite(dirPin, LOW);   
        } 
        delay(100);
        for (int i = 0; i < 2048; i++) {   
            digitalWrite(stepPin, LOW);  // This LOW to HIGH change is what creates the
            digitalWrite(stepPin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
            delayMicroseconds(500);      // This delay time is close to top speed for this
        }                              // particular motor. Any faster the motor stalls.
    } else if (volts > maxVolt or volts < minVolt) {
        if (volts > maxVolt) {
          Serial.println(" TOO LOUD");
          servo.write(servoAngle-5);
        } else {
          Serial.println(" TOO SOFT");
          servo.write(servoAngle+5);
        }
    } else {
      Serial.println("VOLUME IN RANGE");
    }
    servo.write(servoAngle);
    Serial.println(servoAngle);
}
