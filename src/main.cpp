#include <Arduino.h>


#include "Imu.h"
#include "RelativeMotion.h"
#include "Neighbourhood.h"
#include <ArduinoJson.h>


const String MSG_CALIBRATE_ACCGYRO = "CALIBRATE_ACCGYRO";
const String MSG_CALIBRATE_MAG = "CALIBRATE_MAG";
// const String MSG_INITIALIZE_IMU = "INITIALIZE_IMU";

// const String MSG_IMU_INITIALIZED = "IMU_INITIALIZED";

const String MSG_START_IMU = "START_IMU";
const String MSG_STOP_IMU = "STOP_IMU";

const String MSG_SUFFIX_ACK = "ACK";
const String MSG_SUFFIX_DONE = "DONE";
const String MSG_SUFFIX_FAIL = "FAIL";

const int IMU_RESET_PIN = 8;



void processMessage(void * parameter);
void printMessage(const String& msg, const String& suffix = "");

int targetFrameTime= 10; //ms => 100Hz
int frameStart = 0;
int frameStop = 0;
int counter = 0;

int targetFrameTimeNeighbourhood = 500 ; // ms => 10Hz
int neighbourhoodTimeEpsilon = targetFrameTimeNeighbourhood * 0.01;
int lastNeighbourhoodUpdate = 0;
int startTime = 0;

const double relativeMotionHeight = 17;



ImuProcessor* imuProcessor = new ImuProcessor(100, IMU_RESET_PIN);
RelativeMotion* relativeMotion = new RelativeMotion(100, relativeMotionHeight);
Neighbourhood* neighbourhood = new Neighbourhood();
TaskHandle_t  receiveHandle ; 

void setup()
{
    imuProcessor->reset();
    delay(200);
    Serial.println("------------------ SETUP --------------------"); 


    Serial.begin(230400);
    while (!Serial)
        delay(100); // will pause Zero, Leonardo, etc until serial console opens
    

    delay(500);
   
    // Wire1.setBufferSize(64);
    // Wire1.setTimeOut(1000);
    // Wire1.setClock(1000000);
    // Wire1.setClock(50000);
    Wire1.begin(SDA1, SCL1,100000);
    delay(300);

   
    
    
    imuProcessor->init(&Wire1, 5);
    if(!imuProcessor->isConnected())
    {
        printMessage("SensorBoard:setup: IMU not connected !!!");
    }

   
    relativeMotion->init(1);
    if(!relativeMotion->isConnected())
    {
        printMessage("SensorBoard:setup: Optical Flow not connected !!!");
    }


    neighbourhood->init();
    if(!neighbourhood->isConnected())
    {
        printMessage("SensorBoard:setup: Neighbourhood not connected !!!");
    }



    // imuProcessor->start();
    // xTaskCreate(processMessage,"RECEIVE_SERIAL",1000,NULL,0,&receiveHandle);
    // Wire.begin();
    Serial.println("------------------ SETUP END --------------------"); 
    //  Wire1.setClock(50000);
    startTime = millis();

     Wire1.setClock(100000);
}

void loop()
{
    imuProcessor->process();
    //float frameTime = (frameStop - frameStart);
    // Serial.print("FrameTime: ");
    // Serial.println(frameTime,2);
    frameStart = millis();
    
    
    // relativeMotion->process();

    //int nhRemainder = (frameStart - startTime) % (targetFrameTimeNeighbourhood );
    // int frame = (frameStart - startTime) / (targetFrameTimeNeighbourhood );


    // // NEIGHBOURHOOD - START
    if(frameStart - targetFrameTimeNeighbourhood > lastNeighbourhoodUpdate- neighbourhoodTimeEpsilon)
    {
        // int diff = frameStart - lastNeighbourhoodUpdate;
        lastNeighbourhoodUpdate = frameStart;
        // Serial.print("NeighbourhoodUpdate:" );
        // Serial.println(diff);
        neighbourhood->process();
    }
    // // NEIGHBOURHOOD - STOP
   
   
    //delay(5);
}


void printMessage(const String& msg, const String& suffix )
{
    Serial.print(msg);
    if(suffix.isEmpty())
        Serial.println();
    
    else
    {
        Serial.print("|");
        Serial.println(suffix);
    }
    
}

void processMessage(void * parameter)
{
    String msg;
    while(1)
    {
        printMessage("SensorBoard:processMessage",msg);
        while (Serial.available() > 0) {
            
            msg = Serial.readStringUntil('\n');
            printMessage("SensorBoard:RECEIVED MESSAGE",msg);
            if(msg.indexOf(MSG_CALIBRATE_ACCGYRO) > -1)
            {
                printMessage(MSG_CALIBRATE_ACCGYRO,MSG_SUFFIX_ACK);
                // imuProcessor->calibrateAccGyro();
                printMessage(MSG_CALIBRATE_ACCGYRO,MSG_SUFFIX_DONE);
            }

            if(msg.indexOf(MSG_CALIBRATE_MAG) > -1)
            {
                printMessage(MSG_CALIBRATE_MAG,MSG_SUFFIX_ACK);
                // imuProcessor->calibrateMag();
                printMessage(MSG_CALIBRATE_MAG,MSG_SUFFIX_DONE);
            }

            if(msg.indexOf(MSG_START_IMU) > -1)
            {
            
                printMessage(MSG_START_IMU,MSG_SUFFIX_ACK);
                imuProcessor->init(&Wire1);
                // if(imuProcessor->start())
                // {
                //     delay(500);
                //     printMessage(MSG_START_IMU,MSG_SUFFIX_DONE);

                // }
                // else printMessage(MSG_START_IMU,MSG_SUFFIX_FAIL);
            }

            if(msg.indexOf(MSG_STOP_IMU) > -1)
            {
                // Serial.println("Got MSG_STOP_IMU");
                printMessage(MSG_STOP_IMU,MSG_SUFFIX_ACK);
                // imuProcessor->stop();
                printMessage(MSG_STOP_IMU,MSG_SUFFIX_DONE);
        
            }
            // if(msg.indexOf(MSG_INITIALIZE_IMU) > -1)
            // {
            //     // Serial.println("Got MSG_STOP_IMU");
            //     printMessage(MSG_INITIALIZE_IMU,MSG_SUFFIX_ACK);
            //     // if(imuProcessor == NULL)
            //     // {
            //     //     imuProcessor = new ImuProcessor(100);
            //     //     imuProcessor->init();
                
            //     // }
            //     printMessage(MSG_INITIALIZE_IMU,MSG_SUFFIX_DONE);
        
            // }
           
        }
        delay(1000);
    }
    
   
}