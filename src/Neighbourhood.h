/*
 * Author: Thilo Reinhard / Patrick Pogscheba
 * Organisation: Hochschule Düsseldorf
 * Handles comminikation with the neighbourhood sensores
 */

#ifndef __NEIGHBOURHOOD__
#define __NEIGHBOURHOOD__

#include <Wire.h>
#include <Arduino.h>

#include <CustomSerial.h>

class Contact
{
private:
  uint8_t address = 0;
  int upperBound = 0;  //120
  int lowerBound = 0;
  bool connected = false;
  CustomSerial *wireInterface;
  uint8_t buffer[3];

  
  int calibrationSteps = 10;
  float average = 0;
  bool state = false;
  int raw;
  

public:
  Contact(uint8_t address = 10)
  {
    this->address = address;
  }
  void init(CustomSerial *wireInterface)
  {
    // Serial.println("INITIALIZING CONTACT");
    this->wireInterface = wireInterface;
    lowerBound = 0xFFFF;
    upperBound = 0x0000;

    int temp;
    for(int i = 0; i < 100; i++){
      delay(1);
      int error = this->wireInterface->Recive(buffer, 3, this->address);
      // Serial.print(error);
      // Serial.print("\t");
      // // Serial.print("BUFFER: ");
      // Serial.print(buffer[0]);
      // Serial.print(buffer[1]);
      // Serial.println(buffer[2]);
      if(buffer[0] != this->address)
        continue;
      temp = abs(buffer[1] + buffer[2] * 256);
      lowerBound = min(temp, lowerBound);
      upperBound = max(temp, upperBound);
      connected = true;
      // Serial.println("CONTACT CONNECTED");
    }
    if(lowerBound - upperBound > 30000)
    {
      connected = false;
      Serial.println("CONTACT NOT CONNECTED");
      return;
    }
    Serial.println("CONTACT CONNECTED");
    lowerBound -= 25;
    upperBound += 25;

    
    //  Serial.println(lowerBound);
    // Serial.println(upperBound);
    // wireInterface->requestFrom(address, 4);
    // unsigned long time = millis() + 500;
    // while (!wireInterface->available() && time > millis())
    // {
    // }
    // if (wireInterface->available())
    // {
    //    while (wireInterface->available())
    //      (wireInterface->read() + 48);
    //   Serial.println("CONTACT CONNECTED");
    //   connected = true;
    // }
  }
  bool isConnected() { return connected; }

  float getAverage() {return average; }
  float getOffset() {return lowerBound; }
  bool getState() {return state; }
  int getRaw() {return raw; }


  bool process(int *data)
  {
      // if (!connected)
      //   return false;
      
    
      this->wireInterface->Recive(buffer, 3, this->address);
      if(buffer[0] != this->address)
      {
        // Serial.println("<Neighbourhood>::sensor not found: " + String(this->address));
        return false;
      }
        
      else
        raw = buffer[1] + buffer[2] * 256;
      state = raw < lowerBound || upperBound < raw;
      return true;
  }
};

class Neighbourhood
{
private:
  // String errorName = "<Neighbourhood>::";
  // String notConnectedError = "sensor not found: ";

  const String frameStart = "{\"sensorEvents\": [{\"neighbourhood\":{";
  const String frameStop = "}}]}";

   CustomSerial *wireInterface = new CustomSerial(SDA, SCL, nullptr, nullptr, 0, 100);
  
 
  String processContact(String name, Contact* c)
  { 
    
    int data = 0;
    String s = "\"" + name + "\":" ;
    if(c->isConnected() && c->process(&data))
    {
      
      s+=  "{\"connected\" : " + String((c->getState() ? "true" : "false")) + ",";
      s+= "\"offset\" : " + String(c->getOffset()) + ",";
      s+= "\"raw\" : " + String(c->getRaw()) + "}";

    }
    else
    {
      s+="null";
    }

    return s;
  }
 
  int minData = 120;

  bool connected = false;

  Contact left;
  Contact right;
  Contact front;
  Contact back;

  int contactLeftAvg = 0;
  int contactRightAvg = 0;
  int contactFrontAvg = 0;
  int contactBackAvg = 0;

public:
  Neighbourhood(int addLeft =0x0B , int addRight = 0x0A, int addFront = 0x0C, int addBack = 0x0D) : left(addLeft),
                                                                                            right(addRight),
                                                                                            front(addFront),
                                                                                            back(addBack)
  {

  }

  bool isConnected() { return connected; }

  void init()
  {
    
     Serial.println("INITIALIZING NEIGHBOURHOOD");
    // // this->wireInterface = wireInterface;
    // // wireInterface->begin();
    Serial.println("INITIALIZING LEFT");
    left.init(wireInterface);
    delay(5);
    Serial.println("INITIALIZING BACK");
    back.init(wireInterface);
    delay(5);
    Serial.println("INITIALIZING FRONT");
    front.init(wireInterface);
    delay(5);
    Serial.println("INITIALIZING RIGHT");
    right.init(wireInterface);
    delay(5);
    connected = true; // TODO: check each connectionm state ?
  }

  void process()
  {
    if (!connected)
      return;

    Serial.print(frameStart);
    Serial.print(processContact("left", &left));
    Serial.print(",");
    Serial.print(processContact("right", &right));
    Serial.print(",");
    Serial.print(processContact("front", &front));
    Serial.print(",");
    Serial.print(processContact("back", &back));
    Serial.println(frameStop);
  }
};

#endif //__NEIGHBOURHOOD__