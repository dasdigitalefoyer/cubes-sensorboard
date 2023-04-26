/*
 * Author: Thilo Reinhard / Patrick Pogscheba
 * Organisation: Hochschule Düsseldorf
 * Handles comminikation with the neighbourhood sensores
 */

#ifndef __NEIGHBOURHOOD__
#define __NEIGHBOURHOOD__

#include <Wire.h>
#include <Arduino.h>

class Contact
{
private:
  int address = 0;
  int minData = 0;  //120
  bool connected = false;
  TwoWire *wireInterface;

  
  int calibrationSteps = 10;
  float average = 0;
  bool state = false;

public:
  Contact(int address = 10)
  {
    this->address = address;
  }
  void init(TwoWire *wireInterface)
  {
    // Serial.println("INITIALIZING CONTACT");
    this->wireInterface = wireInterface;
    wireInterface->begin();

    wireInterface->requestFrom(address, 4);
    unsigned long time = millis() + 500;
    while (!wireInterface->available() && time > millis())
    {
    }
    if (wireInterface->available())
    {
       while (wireInterface->available())
         (wireInterface->read() + 48);
      Serial.println("CONTACT CONNECTED");
      connected = true;
    }
  }
  bool isConnected() { return connected; }

  float getAverage() {return average; }
  bool getState() {return state; }

  bool process(int *data)
  {
    if (!connected)
      return false;
    
    unsigned long time = micros() + 100;

    String dataChar = "";
    wireInterface->requestFrom(address, 4);
    while (!wireInterface->available() && time > micros())
      ;

    while (wireInterface->available())
      dataChar += (char)(wireInterface->read() + 48);

    int d = dataChar.toInt();


    if (d != 0)
      d = abs(d - 512);
    else return false;
    if(calibrationSteps > 0)
    {
      calibrationSteps = calibrationSteps -1;
      minData = max(d+1,minData) ;
       Serial.print("CALIBRATING CONTACT: ");
       Serial.println(calibrationSteps);
      return true;
    }
    d = max((d - minData), 0);
    average = 0.75 * average + 0.25*d;
    if(average > 12) state = true;
    else if(average < 3) state = false;
    *data=d;
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
  
 
  String processContact(String name, Contact* c)
  { 
    //if(!c.isConnected()) return "";
    int data = 0;
    String s = "\"" + name + "\":" ;
    if(c->isConnected() && c->process(&data))
    {
      
      s+=  "{\"connected\" : " + String((c->getState() ? "true" : "false")) + ",";
      s+= "\"raw\" : " + String(data) + "}";
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
  Neighbourhood(int addLeft = 10, int addRight = 11, int addFront = 12, int addBack = 13) : left(addLeft),
                                                                                            right(addRight),
                                                                                            front(addFront),
                                                                                            back(addBack)
  {

  }

  bool isConnected() { return connected; }

  void init(TwoWire *wireInterface)
  {
    Serial.println("INITIALIZING NEIGHBOURHOOD");
    // this->wireInterface = wireInterface;
    // wireInterface->begin();
    Serial.println("INITIALIZING LEFT");
    left.init(wireInterface);
    Serial.println("INITIALIZING BACK");
    back.init(wireInterface);
    Serial.println("INITIALIZING FRONT");
    front.init(wireInterface);
    Serial.println("INITIALIZING RIGHT");
    right.init(wireInterface);
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