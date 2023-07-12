#ifndef __CUSTOMSERIAL__
#define __CUSTOMSERIAL__

#include <Wire.h>
#include <Arduino.h>
/*
 * Author: Thilo Reinhard
 * Organisation: Hochschule Düsseldorf
 * Handles comminikation with the neighbourhood sensores
 */
class CustomSerial{
  private:
    uint16_t dataPin;
    uint16_t clockPin;
    uint8_t address;

    uint16_t microsDelay = 100;

    typedef void(*callbackType)(uint8_t*, uint8_t);

    callbackType reciveCallback;
    callbackType requestCallback;

  public:
    bool isMaster(){
      return address == 0;
    }

    /*dataPin: pinnummer des daten pins
    / clockPin: pinnummer des clock pins
    / address: adresse des chips, unique
    /  0 => master
    /  1-255 => slave
    */
    CustomSerial(uint16_t dataPin, uint16_t clockPin, callbackType reciveCallback, callbackType requestCallback, uint8_t address = 0, uint16_t microsDelay = 100){
      this->dataPin = dataPin;
      this->clockPin = clockPin;
      this->address = address;
      this->reciveCallback = reciveCallback;
      this->requestCallback = requestCallback;
      this->microsDelay = microsDelay;
      if(isMaster()){
        pinMode(dataPin, OUTPUT);
        pinMode(clockPin, OUTPUT);
        digitalWrite(dataPin, LOW);
        digitalWrite(clockPin, LOW);
      }
      else{
        pinMode(dataPin, INPUT);
        pinMode(clockPin, INPUT);
      }
    }
    
    /*0-> Ok
    / 1-> timeOut(Slave Only)
    / 2-> parrity failed
    / 4-> no response
    / 8-> data not responded
    / 16-> is not master
    */
    uint8_t Send(uint8_t* data, uint8_t count, uint8_t address){
      if(!isMaster())
        return 16;

      uint8_t error = 0;
      //put address and count
      error |= setOnToLine(address);
      error |= setOnToLine(count);
      error |= setOnToLine(0x01);

      for(int i = 0; i < count; i++)
        error |= setOnToLine(data[i]);
      
      if(error != 0)
        return error;

      return 0;
    }

    /*0-> Ok
    / 1-> timeOut(Slave Only)
    / 2-> parrity failed
    / 4-> no response
    / 8-> data not responded
    */
    uint8_t Recive(uint8_t* buffer, uint8_t count, uint8_t address){
      if(!isMaster())
        return 16;
     
      uint8_t error = 0;
      //put address and count
      error |= setOnToLine(address);
      error |= setOnToLine(count);
      error |= setOnToLine(0x02);

      for(int i = 0; i < count; i++)
        error |= getFromLine(buffer + i);

      if(error != 0)
        return error;
      
      return 0;
    }

    //slave only
    void HandleIncomming(){
      if(isMaster())
        return;

      uint8_t error = 0;

      while(digitalRead(clockPin) != HIGH);
      uint8_t address, count, readWrite;

      error |= getFromLine(&address);
      error |= getFromLine(&count);
      error |= getFromLine(&readWrite);
      
      if(address != this->address)
        return;

      if(readWrite == 1){
        uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * count);
        
        for(int i = 0; i < count; i++)
          error |= getFromLine(buffer + i);
        
        (*reciveCallback)(buffer, count);

        delete buffer;
      }else if(readWrite == 2){
        //write
        uint8_t* buffer = (uint8_t*)malloc(sizeof(uint8_t) * count);

        (*requestCallback)(buffer, count);
        
        for(int i = 0; i < count; i++)
          error |= setOnToLine(buffer[i]);

        delete buffer;
      }
    }

  private:
    bool waitForState(bool state){
      if(isMaster()){
        delayMicroseconds(microsDelay);
        digitalWrite(clockPin, state);
        return true;
      }else{
        uint64_t time = micros() + microsDelay*2;
        while(digitalRead(clockPin) != state && time > micros());
        return time > micros();
      }
    }

    //0xff -> 11111111 0
    //0x00 -> 00000000 1
    //0xAA -> 10101010 1
    //0x55 -> 01010101 0
    uint8_t setOnToLine(uint8_t data){
      pinMode(dataPin, OUTPUT);
      
      for(int i = 0; i < 8; i++){
        uint8_t temp = data;
        if(!waitForState(HIGH))
          return 1;
        digitalWrite(dataPin, ((temp >> i) & 0x01) != 0 ? HIGH : LOW);
        if(!waitForState(LOW))
          return 1;
      }
      if(!waitForState(HIGH))
        return 1;
      if(!waitForState(LOW))
        return 1;

      digitalWrite(dataPin, LOW);
      if(!isMaster())
        pinMode(dataPin, INPUT);
      
      return 0;
    }
    uint8_t getFromLine(uint8_t* data){
      if(isMaster())
        pinMode(dataPin, INPUT_PULLDOWN);
      else
        pinMode(dataPin, INPUT);

      uint8_t temp = 0;
      for(int i = 0; i < 8; i++){
        if(!waitForState(HIGH))
          return 1;
        if(!waitForState(LOW))
          return 1;
        temp += (((uint8_t)digitalRead(dataPin) & 0x01) << i);
      }
      *data = temp;

      if(!waitForState(HIGH))
        return 1;
      if(!waitForState(LOW))
        return 1;

      if(isMaster()){
        pinMode(dataPin, OUTPUT);
        digitalWrite(dataPin,LOW);
      }

      return 0;
    }
};

#endif //__CUSTOMSERIAL__