#include <Arduino.h>
#include <TaskScheduler.h>
#include "Imu.h"
#include "RelativeMotion.h"
#include "Neighbourhood.h"
#include "BatteryMeter.h"
#include <ArduinoJson.h>

#define _TASK_TIMECRITICAL
 //#define _TASK_SLEEP_ON_IDLE_RUN

const String MSG_CALIBRATE_ACCGYRO = "CALIBRATE_ACCGYRO";
const String MSG_CALIBRATE_MAG = "CALIBRATE_MAG";
// const String MSG_INITIALIZE_IMU = "INITIALIZE_IMU";

// const String MSG_IMU_INITIALIZED = "IMU_INITIALIZED";

const String MSG_START_IMU = "START_IMU";
const String MSG_STOP_IMU = "STOP_IMU";
const String MSG_RESET = "RESET";

const String MSG_SUFFIX_ACK = "ACK";
const String MSG_SUFFIX_DONE = "DONE";
const String MSG_SUFFIX_FAIL = "FAIL";

const int IMU_RESET_PIN = 8;

void processMessage(void *parameter);
void printMessage(const String &msg, const String &suffix = "");

void wire0Processing();
void wire1Processing();
void batteryMetering();

int targetFrameTime = 20; // 10ms => 100Hz
int frameStart = 0;
int frameStop = 0;
int counter = 0;

// int targetFrameTimeNeighbourhood = 500; // ms => 2Hz
// int neighbourhoodTimeEpsilon = targetFrameTimeNeighbourhood * 0.01;
// int lastNeighbourhoodUpdate = 0;
 int startTime = 0;

const double relativeMotionHeight = 25;

ImuProcessor *imuProcessor = new ImuProcessor(1000/targetFrameTime, IMU_RESET_PIN);
RelativeMotion *relativeMotion = new RelativeMotion(1000/targetFrameTime , relativeMotionHeight); // double framerate
Neighbourhood *neighbourhood = new Neighbourhood();
BatteryMeter *batteryMeter = new BatteryMeter(18);

Scheduler runner;
Task wire1Task(10, TASK_FOREVER, &wire1Processing);
Task wire0Task(250, TASK_FOREVER, &wire0Processing);

Task batteryTask(1000, TASK_FOREVER, &batteryMetering);


void setup()
{

    Serial.begin(230400   );
    while (!Serial)
        delay(100); // will pause Zero, Leonardo, etc until serial console opens

    Serial.println("------------------ SETUP --------------------");
    // pinMode(IMU_RESET_PIN, OUTPUT); 
    // digitalWrite(IMU_RESET_PIN, HIGH); 
    imuProcessor->reset();
   
    

    delay(200);

    Wire1.begin(SDA1, SCL1, 1000000);
    // delay(300);

    imuProcessor->init(&Wire1, 3);
    if (!imuProcessor->isConnected())
    {
        printMessage("SensorBoard:setup: IMU not connected !!!");
    }

    relativeMotion->init(3);
    if (!relativeMotion->isConnected())
    {
        printMessage("SensorBoard:setup: Optical Flow not connected !!!");
    }
    delay(100);
    Wire.flush();
    // pinMode(SDA, PULLUP);
    // pinMode(SCL, PULLUP);
    Wire.begin(SDA, SCL, 1000000);
    //ESP.restart();
    
   
    delay(100);

    neighbourhood->init(Wire);
    if (!neighbourhood->isConnected())
    {
        printMessage("SensorBoard:setup: Neighbourhood not connected !!!");
    }

    Serial.println("------------------ SETUP END --------------------");
    //  Wire.setTimeOut(2);
    startTime = millis();
    runner.init();
    delay(100);
   
    runner.addTask(wire0Task);
     wire0Task.enable();

    runner.addTask(wire1Task);
    wire1Task.enable();

    // runner.addTask(batteryTask);
    // batteryTask.enable();

}

void loop()
{
    //wire1Processing();
    //delay(10);
    runner.execute();
   
}

void wire1Processing()
{
    // Serial.println("wire1Processing");
    imuProcessor->process();

    relativeMotion->process();

}

void wire0Processing()
{
    // Serial.println("wire0Processing");
    neighbourhood->process();

    // Serial.printf("CLOCK: %i", Wire.getClock());
}

void batteryMetering()
{
    batteryMeter->process();
}

void printMessage(const String &msg, const String &suffix)
{
    Serial.print(msg);
    if (suffix.isEmpty())
        Serial.println();

    else
    {
        Serial.print("|");
        Serial.println(suffix);
    }
}

void processMessage(void *parameter)
{
    String msg;
    while (1)
    {
        printMessage("SensorBoard:processMessage", msg);
        while (Serial.available() > 0)
        {

            msg = Serial.readStringUntil('\n');
            printMessage("SensorBoard:RECEIVED MESSAGE", msg);
            if (msg.indexOf(MSG_CALIBRATE_ACCGYRO) > -1)
            {
                printMessage(MSG_CALIBRATE_ACCGYRO, MSG_SUFFIX_ACK);
                // imuProcessor->calibrateAccGyro();
                printMessage(MSG_CALIBRATE_ACCGYRO, MSG_SUFFIX_DONE);
            }

            if (msg.indexOf(MSG_CALIBRATE_MAG) > -1)
            {
                printMessage(MSG_CALIBRATE_MAG, MSG_SUFFIX_ACK);
                // imuProcessor->calibrateMag();
                printMessage(MSG_CALIBRATE_MAG, MSG_SUFFIX_DONE);
            }
            if (msg.indexOf(MSG_RESET) > -1)
            {
                imuProcessor->reset();
                
                printMessage(MSG_RESET, MSG_SUFFIX_ACK);
                ESP.restart();
            }
            if (msg.indexOf(MSG_START_IMU) > -1)
            {

                printMessage(MSG_START_IMU, MSG_SUFFIX_ACK);
                imuProcessor->init(&Wire1);
                // if(imuProcessor->start())
                // {
                //     delay(500);
                //     printMessage(MSG_START_IMU,MSG_SUFFIX_DONE);

                // }
                // else printMessage(MSG_START_IMU,MSG_SUFFIX_FAIL);
            }

            if (msg.indexOf(MSG_STOP_IMU) > -1)
            {
                // Serial.println("Got MSG_STOP_IMU");
                printMessage(MSG_STOP_IMU, MSG_SUFFIX_ACK);
                // imuProcessor->stop();
                printMessage(MSG_STOP_IMU, MSG_SUFFIX_DONE);
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