/*************************
  Joel Bartlett
  SparkFun Electronics
  December 27, 2012

  This code controls a stepper motor with the
  EasyDriver board. It spins forwards and backwards
***************************/
//Adapted from sample code on adafruit.com for sound detection

int dirpin = 2;
int steppin = 3;
float minVolt = 0.5;
float maxVolt = 1.7;

#define analogPinForMic    0   // change to pins you the analog pins are using

const int sampleWindow = 1000; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup()
{
  pinMode(dirpin, OUTPUT);
  pinMode(steppin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{

  int i;

  {
    unsigned long startMillis = millis(); // Start of sample window
    unsigned int peakToPeak = 0; // peak-to-peak level
    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;
    // collect data for 50 mS
    while (millis() - startMillis < sampleWindow) {
      sample = analogRead(analogPinForMic);
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
    if (volts > maxVolt or volts < minVolt){
        if (volts > maxVolt) {
          Serial.println("TOO LOUD");
          digitalWrite(dirpin, HIGH);     // Set the direction.
          delay(100);
        } else {
          Serial.println("TOO SOFT");
          digitalWrite(dirpin, LOW);    // Change direction.
          delay(100);
        }
    for (i = 0; i < 1024; i++)     // Iterate for 4000 microsteps
    {
      digitalWrite(steppin, LOW);  // This LOW to HIGH change is what creates the
      digitalWrite(steppin, HIGH); // "Rising Edge" so the easydriver knows to when to step.
      delayMicroseconds(500);      // This delay time is close to top speed for this
    }                              // particular motor. Any faster the motor stalls.
    }
    else{
      Serial.println("VOLUME IN RANGE");
    }
  }
}
