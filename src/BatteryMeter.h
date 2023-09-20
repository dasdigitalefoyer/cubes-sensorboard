#ifndef __BATTERY_METER__
#define __BATTERY_METER__

#include <Arduino.h>

class BatteryMeter{
  private:
    int analogPin;
    
    int maxCounterValue;
    int counter;

    float alpha = 0.9f;
    float lastVoltage = 0.0f;
    float voltage = 0;
    const float VOLTAGE_MULTIPLIER = 0.0018326206f;

  public:
    BatteryMeter(int analogPin = 8) {
      this->analogPin = analogPin;
      pinMode(analogPin, INPUT);
      lastVoltage = analogRead(analogPin);
    }

    float getVoltage(){
      
      voltage = alpha * lastVoltage + (1 - alpha) * analogRead(analogPin);
      if(voltage > lastVoltage -3)
      {    
          lastVoltage = voltage;
      }
      
      return voltage * VOLTAGE_MULTIPLIER;
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