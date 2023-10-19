/*
 * Author: Thilo Reinhard / Patrick Pogscheba
 * Organisation: Hochschule Düsseldorf
 * Handles comminikation with the neighbourhood sensores
 */

#ifndef __NEIGHBOURHOOD__
#define __NEIGHBOURHOOD__

#include <Wire.h>
#include <Arduino.h>
#include "PN532_I2C.h"
#include "PN532.h"
#include <PWFusion_TCA9548A.h>

#include <iomanip>
#include <sstream>

#define PN532DEBUG
#define MIFAREDEBUG

class Contact
{
private:
  PN532_I2C *pn532i2c = NULL; //(Wire);

  PN532 *nfc = NULL; //(pn532i2c);

  bool connected = false;
  String name = "Contact";
  uint8_t buf[4];
  uint8_t uid[7];
  uint8_t uidLength;
  String miscData = "";
  std::string tagUID = "";
  bool state = false;

  bool toggleState = false;

  int connectCount = 1;
  int currentCount = 0;
  int disconnectCount = 3;

  std::string hexStr(unsigned char *data, int len)
  {
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < len - 1; ++i)
      ss << std::setw(2) << std::setfill('0') << std::uppercase << (int)data[i] << ":";
    ss << std::setw(2) << std::setfill('0') << std::uppercase << (int)data[len - 1];

    return ss.str();
  }

public:
  Contact(const String &name) : name(name)
  {
  }
  void init(TwoWire &wireInterface)
  {
    Serial.printf("INITIALIZING CONTACT: %s \r\n", name.c_str());

    pn532i2c = new PN532_I2C(wireInterface);
    nfc = new PN532(*pn532i2c);

    nfc->begin();
    uint32_t versiondata = nfc->getFirmwareVersion();
    if (!versiondata)
    {
      Serial.println("Didn't find PN53x board");
      return;
    }
    nfc->setPassiveActivationRetries(0x00);

    connected = true;

    Serial.print("Found chip PN5");
    Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);

    // configure board to read RFID tags
    nfc->SAMConfig();
  

    
    // nfc->startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
    Serial.printf("%s: CONNECTED \r\n", name.c_str());
  }
  bool isConnected() { return connected; }

  bool getState() { return state; }
  String getName() { return name; }
  String getData() { return String(tagUID.c_str()); }
  String getUID() { return String(tagUID.c_str()); }


  
  bool process(int *data)
  {
    if (!connected)
      return false;


    if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 0, false))
    {
     
        toggleState = false;
        state = true;      
        miscData = "";
        tagUID = hexStr(uid, uidLength);
        nfc->setPassiveActivationRetries(0x01);
        // nfc->inRelease(0x00);
    }
    else
    {
      // Serial.printf("%s: Found no card! \n",name.c_str());
      if (!toggleState && state)
        toggleState = true;
      else {
        toggleState = false;
        state = false;
        tagUID = "";
        nfc->setPassiveActivationRetries(0x00);
      }
        
       
  
    }
  

    if(tagUID.empty())
    {
      tagUID = hexStr(uid, uidLength);
    
    }

    return true;
  }
};

//   bool process(int *data)
//   {
//     if (!connected)
//       return false;


//     if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 0, true))
//     {
     
//       currentCount = min(disconnectCount, currentCount + 1);
//       Serial.printf("%s : Found a card!  count  %i\n",name.c_str(), currentCount);
//       if(currentCount < connectCount)
//         return true;
      

//         state = true;      
//         miscData = "";
//         tagUID = hexStr(uid, uidLength);
     
//     }
//     else
//     {
//       currentCount = max(0, currentCount - 1);
//       Serial.printf("%s: Found no card!  count  %i\n",name.c_str(), currentCount);
//       //  Serial.println("Found no card! ");
//       if (currentCount > 0)
//         // toggleState = true;
//         return true;
//       else {
//         // toggleState = false;
//         state = false;
//         tagUID = "";
//       }
        
       
  
//     }

//     {
//       tagUID = hexStr(uid, uidLength);
//       // for (byte i = 0; i < uidLength;i++) {
//       //     tagUID+= uid[i] < 0x10 ? " 0" : ":";
//       //     tagUID+=String(uid[i],HEX);

//       // }
//     }

//     return true;
//   }
// };

class Neighbourhood
{
private:
  // String errorName = "<Neighbourhood>::";
  // String notConnectedError = "sensor not found: ";

  const String frameStart = "{\"sensorEvents\": [{\"neighbourhood\":{";
  const String frameStop = "}}]}";
  int index = 0;
  String serialString = "";

  // TwoWire *wireInterface;

  String processContact(Contact *c)
  {

    int data = 0;
    String name = c->getName();
    name.toLowerCase();
    String s = "\"" + name + "\":";
    if (c->isConnected() && c->process(&data))
    {

      s += "{\"connected\": " + String((c->getState() ? "true" : "false"));
      if (c->getState())
      {
        s += ",";
        s += "\"id\": \"" + String(c->getUID()) + "\""; // + ",";
        // s+= "\"raw\" : " + String(c->getRaw()) ;
      }
      s += "}";
    }
    else
    {
      s += "null";
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

  TCA9548A i2cMux;

public:
  Neighbourhood(int channelLeft = 0, int channelRight = 1, int channelFront = 2, int channelBack = 3) : left("LEFT"),
                                                                                                        right("RIGHT"),
                                                                                                        front("FRONT"),
                                                                                                        back("BACK")

  {
  }

  bool isConnected() { return connected; }

  void init(TwoWire &wireInterface)
  {
    delay(50);
    i2cMux.begin(0U, wireInterface);
    Serial.println("INITIALIZING NEIGHBOURHOOD");

    // // this->wireInterface = wireInterface;
    // // wireInterface->begin();
    Serial.println("INITIALIZING LEFT");
    i2cMux.setChannel(CHAN0);
    delay(50);
    left.init(wireInterface);
    delay(50);
    Serial.println("INITIALIZING RIGHT");
    i2cMux.setChannel(CHAN1);
    delay(50);
    right.init(wireInterface);
    delay(50);
    Serial.println("INITIALIZING FRONT");
    i2cMux.setChannel(CHAN2);
    delay(50);
    front.init(wireInterface);
    delay(50);
    Serial.println("INITIALIZING BACK");
    i2cMux.setChannel(CHAN3);
     delay(50);
    back.init(wireInterface);
    delay(50);
    connected = true; // TODO: check each connectionm state ?
  }

  void process()
  {
    if (!connected)
      return;

    switch (index)
    {
    case 0:

      serialString += frameStart;
      i2cMux.setChannel(CHAN0);
      serialString += processContact(&left) + ",";


      break;
    case 1:
      i2cMux.setChannel(CHAN1);
      serialString += processContact(&right) + ",";
      break;
    case 2:
      i2cMux.setChannel(CHAN2);
      serialString += processContact(&front) + ",";
      break;
    case 3:
      i2cMux.setChannel(CHAN3);
      serialString += processContact(&back);
      serialString += frameStop;
      Serial.println(serialString);
      serialString = "";
      index = 0;
      return;
    }

    ++index;
  }
};

#endif //__NEIGHBOURHOOD__