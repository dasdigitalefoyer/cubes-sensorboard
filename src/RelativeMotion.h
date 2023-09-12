#ifndef __RELATIVE_MOVEMENT__
#define __RELATIVE_MOVEMENT__

#include <ArduinoJson.h>
#include <Arduino.h>
#include <SPI.h>
#include "PAA5100JE.h"



class RelativeMotion
{
    private:
        int framerate = 100;
        int chipSelectPin = 16;
        bool useMM = true;
        PAA5100JE_OF sensor;
        double height = 0;
        uint32_t tTime;
        bool connected = false;

        double deltaXY[2] = {0, 0};
        // static const int capacity = JSON_OBJECT_SIZE(2);
        // StaticJsonDocument<capacity> movementJson;
        
        

    public:
        RelativeMotion(int framerate = 100, double height = 0, int chipSelectPin = 16) : height(height), framerate(framerate), chipSelectPin(chipSelectPin), sensor(chipSelectPin)
        {

        }

        void reset()
        {

        }

        void process()
        {
            if(!connected)
                return;
            uint32_t t = millis();
            uint32_t t_diff = t-tTime;
            if ((t_diff) >= (1000 / framerate))
            {
                if (sensor.getDistance(deltaXY) == false)
                    Serial.println("Error reading sensor 1 values");
                else
                {
                    String s =  "{\"sensorEvents\": [";  
                    s+="{\"motion\": ";
                    s+="{\"x\": ";
                    s+=String(deltaXY[0],10);
                    s+=",\"y\":";
                    s+=String(deltaXY[1],10);
                    
                    s+=",\"dt\":";
                    s+=String(t_diff/1000.0,3);
                    s+="}}";
                    s+="]}";
                    Serial.println(s);
                }
                tTime = t;
            }
        }

        void init(int tries = 5)
        {
             Serial.println("INITIALIZING RELATIVE MOTION");
             Serial.println("Connecting SPI");
            SPI.begin();
            int i=0;
           
            while (i++<tries) { 
                if (sensor.init() == false)
                {
                    // while (true)
                    {
                        delay(1000);
                        Serial.println("Error in sensors initialization");
                    }
                }
                else
                    connected =true;
            }
            if(!connected)
                return;
            delay(100);
            // Set sensor working height first.
            // This can be done realtime using distance sensor, if sensor/surface height is changing.
            sensor.setWorkingHeight(height);
            sensor.setOrientation(false, true, true);
    
            delay(100);

            // JsonArray sensorEvents = movementJson.createNestedArray("sensorEvents");
        
        }

        bool isConnected() { return connected;}
};

#endif