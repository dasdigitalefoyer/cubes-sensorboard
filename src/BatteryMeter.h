#ifndef __BATTERY_METER__
#define __BATTERY_METER__

#include <Arduino.h>

class BatteryMeter{
  private:
    int analogPin;
    
    int maxCounterValue;
    int counter;

  public:
    BatteryMeter(int analogPin = 8) {
      this->analogPin = analogPin;
      pinMode(analogPin, INPUT);
    }

    float getVoltage(){
      return ((analogRead(analogPin) * 2.57f) / 51000.0f)*5.7f;
    }

  

    // return true if voltage ist to Low()
    void process(){
        String s =  "{\"sensorEvents\": [";  
        s+="{\"voltage\": ";
        
        s+=String(getVoltage(),2);                   
        s+="}";
        s+="]}";
        Serial.println(s);
     
    }
};
#endif