#ifndef __BATTERY_METER__
#define __BATTERY_METER__

#include <Arduino.h>

class BatteryMeter{
  private:
    int analogPin;
    
    int maxCounterValue;
    int counter;

    float alpha = 0.9f;
    int lastRaw = 0;
    int raw = 0;
    const float VOLTAGE_MULTIPLIER = 0.0018326206f;

  public:
    BatteryMeter(int analogPin = 8) {
      this->analogPin = analogPin;
      pinMode(analogPin, INPUT);
      lastRaw = (analogRead(analogPin) + analogRead(analogPin) + analogRead(analogPin)) / 3;  
    }

    float getVoltage(){
      
      raw = alpha * lastRaw + (1 - alpha) * analogRead(analogPin);
      if(raw > lastRaw -500 && raw < lastRaw + 500)
      {    
          lastRaw = raw;
      }
      
      return raw * VOLTAGE_MULTIPLIER;
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