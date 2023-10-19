#ifndef __IMU__
#define __IMU__
// #include <SparkFun_BNO080_Arduino_Library.h>
#include "AdafruitBno08x.h"
#include <ArduinoJson.h>
#include <bitset>
#define BNO08X_RESET -1



class ImuProcessor {
public:
    // enum class State { UNINITIALIZED, INITIALIZING, INITIALIZED, STARTED, STOPPED };


    ImuProcessor(int frameRate = 50, int resetPin = -1)  : mpu(resetPin)
    {
        // i2c_semaphore = xSemaphoreCreateMutex();
        this->frameRate = frameRate;

        
    }

    

    

    void calibrateAccGyro()
    {
        Serial.println("Calibrating AccGyro");
        // mpu.calibrateAccelGyro();
    }

    void calibrateMag()
    {
        Serial.println("Calibrating Mag");
        // mpu.calibrateMag();
    }

    

   

    // State getState() { return state; }

   
    void init( TwoWire *wire , int tries = 2)
    {
        
        Serial.println("INITIALIZING IMU");
        
     
        int i=0;
        
        while (i++<tries) { 
            if (!mpu.begin_I2C(BNO08x_I2CADDR_DEFAULT,wire)) {
            //if (!bno08x.begin_UART(&Serial1)) {  // Requires a device with > 300 byte UART buffer!
            //if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
                Serial.println("Failed to find BNO08x chip");
                // wire->flush();
                // wire->end();
                // wire->begin();
                // ESP.restart();
                this->reset();
                 delay(200);
            }
            else{
                connected =true;
                break;
            }
                
            delay(500); 
        }
        if(!connected)
            return;
            
   
        for (int n = 0; n < mpu.prodIds.numEntries; n++) {
            Serial.print("Part ");
            Serial.print(mpu.prodIds.entry[n].swPartNumber);
            Serial.print(": Version :");
            Serial.print(mpu.prodIds.entry[n].swVersionMajor);
            Serial.print(".");
            Serial.print(mpu.prodIds.entry[n].swVersionMinor);
            Serial.print(".");
            Serial.print(mpu.prodIds.entry[n].swVersionPatch);
            Serial.print(" Build ");
            Serial.println(mpu.prodIds.entry[n].swBuildNumber);
        }

        int ret = mpu.configureMotionEngine(0.1f,0.05f,0,1);
        // int ret = mpu.configureMotionEngine();3
        if(ret == SH2_OK)
            Serial.println("configuring MotionEngine succeeded");
        else if(ret == SH2_NO_CHANGE)
            Serial.println("configuring MotionEngine  did not change");
        else
        {
            Serial.print("configuring MotionEngine failed: ");
            Serial.println(ret);
        }

        ret = mpu.configureStabilityDetector(0.5f, 0.5f);
        // ret = mpu.configureStabilityDetector();
        if(ret == SH2_OK)
            Serial.println("configuring StabilityDetector succeeded");
        else if(ret == SH2_NO_CHANGE)
            Serial.println("configuring StabilityDetector  did not change");
        else
        {
            Serial.print("configuring StabilityDetector failed: ");
            Serial.println(ret);
        }
            
        setReports();
    
        // delay(500);       
       
     

    }

    void reset()
    {
        mpu.hardwareReset();
        delay(100);
    }

    bool isConnected() { return connected;}

    void process()
    {
        if(!connected)
            return;
        if (mpu.wasReset()) {
            Serial.print("sensor was reset ");
            setReports();

            timeRot = millis();
            timeAcc = millis();
            timeGyro = millis();
            timeStability = millis();
            return;
        }
        // Serial.println("PROCESSING ");
        // Serial.flush();
        // return;
        
        // StaticJsonDocument<capacity> imuJson;
        // JsonArray sensorEvents = imuJson.createNestedArray("sensorEvents");
       
        uint16_t reportId = 0;
        int time = millis();  
      

        
        int counter = 0;
        //counter ++;
       // int t0 = millis();
        // Serial.print("{\"sensorEvents\": [");
        String s =  "{\"sensorEvents\": [";
    
    
        while(mpu.getSensorEvent(&sensorValue))
        {
            //  Serial.println(sensorValue.sensorId);
            //  return;
       
            // JsonObject event = sensorEvents.createNestedObject();
            if(counter++ > 0)
            {
                //  Serial.print(",");
                 s+=",";
            }
            // imuJson["imu"]["dt"] = dt;
            // event["timestamp"] = sensorValue.timestamp;
            switch(sensorValue.sensorId)
            {
                
                // case SH2_GAME_ROTATION_VECTOR:
                case SH2_ROTATION_VECTOR:
                {
                    // JsonObject data = event.createNestedObject("euler");
                    //Serial.println("SH2_ROTATION_VECTOR");
                    // event["type"] = "euler";
                    quaternionToEuler(
                        sensorValue.un.rotationVector.real,
                        sensorValue.un.rotationVector.i,
                        sensorValue.un.rotationVector.j,
                        sensorValue.un.rotationVector.k,
                        &ypr,
                        true
                    );

                    // data["x"] = ypr.roll;
                    // data["y"] = ypr.pitch;
                    // data["z"] = ypr.yaw;
                    // data["dt"] = 1.0f/ frRot;
                    s+="{\"rotation\": {";
                    s+="\"yaw\":";
                    s+=String(ypr.yaw,8);
                    s+=",\"pitch\":";
                    s+=String(ypr.pitch,8);
                    s+=",\"roll\":";
                    s+=String(ypr.roll,8);
                    s+=",\"i\":";
                    s+=String(sensorValue.un.rotationVector.i,8);
                    s+=",\"j\":";
                    s+=String(sensorValue.un.rotationVector.j,8);
                    s+=",\"k\":";
                    s+=String(sensorValue.un.rotationVector.k,8);
                    s+=",\"real\":";
                    s+=String(sensorValue.un.rotationVector.real,8);
                     s+=",\"dt\":";
                    s+=String((time-timeRot) * 0.001,5);

                    timeRot= time;
                    // s+=",\"count\":";
                    // s+=String(countRot++);
                    s+="}}";
                    break;
                }

                case SH2_ACCELEROMETER:
                {
                    // //Serial.println("SH2_ACCELEROMETER");
                    // JsonObject data = event.createNestedObject("acc");
                    // // event["type"] = "linAcc";
                    // data["x"] = sensorValue.un.accelerometer.x;
                    // data["y"] = sensorValue.un.accelerometer.y;
                    // data["z"] = sensorValue.un.accelerometer.z;
                    // data["dt"] =1.0f/  frLinAcc;
                    // data["count"] = countAcc++;
                    break;
                }

                case SH2_LINEAR_ACCELERATION:
                {
                    //Serial.println("SH2_LINEAR_ACCELERATION");
                    // JsonObject data = event.createNestedObject("linAcc");
                    // // event["type"] = "linAcc";
                    // data["x"] = sensorValue.un.linearAcceleration.x;
                    // data["y"] = sensorValue.un.linearAcceleration.y;
                    // data["z"] = sensorValue.un.linearAcceleration.z;
                    // data["dt"] =1.0f/  frLinAcc;


                   
                    s+="{\"linAcc\": {";
                    s+="\"x\":";
                    s+=String(sensorValue.un.linearAcceleration.x,8);
                    s+=",\"y\":";
                    s+=String(sensorValue.un.linearAcceleration.y,8);
                    s+=",\"z\":";
                    s+=String(sensorValue.un.linearAcceleration.z,8);                    
                    s+=",\"dt\":";
                    s+=String((time-timeAcc) * 0.001,5);
                    // s+=",\"count\":";
                    // s+=String(countAcc++);
                    s+="}}";

                    timeAcc = time;
                    break;
                }
                case SH2_TEMPERATURE:
                {
                    Serial.println("SH2_TEMPERATURE");
                    // JsonObject data = event.createNestedObject("temperature");
                    // // event["type"] = "temperature";
                    // data["temperature"] = sensorValue.un.temperature.value;
                    break;
                }

                case SH2_GRAVITY:
                {
                    //Serial.println("SH2_GRAVITY");
                    // JsonObject data = event.createNestedObject("gravity");
                    // event["type"] = "temperature";
                    Serial.print("{\"gravity\": {");
                    Serial.print("\"x\":");
                    // data["x"] = sensorValue.un.gravity.x;
                    Serial.print(sensorValue.un.gravity.x);
                    Serial.print(",\"y\":");
                    // data["y"] = sensorValue.un.gravity.y;
                    Serial.print(sensorValue.un.gravity.y);
                    Serial.print(",\"z\":");
                    // data["z"] = sensorValue.un.gravity.z;
                    Serial.print(sensorValue.un.gravity.z);                    
                    Serial.print(",\"dt\":");
                    Serial.print(1.0f/ frameRate);
                    // data["dt"] = 1.0f/ frLinAcc;
                    // data["count"] = countGrav++;
                    Serial.print(",\"count\":");
                    Serial.print(countGrav);
                    Serial.print("}}");
                    break;
                }

                case SH2_GYROSCOPE_CALIBRATED:
                {
                    // Serial.println("SH2_GYROSCOPE_CALIBRATED");
                    // JsonObject data = event.createNestedObject("gyro");
                    // // event["type"] = "gyro";
                    // data["x"] = sensorValue.un.gyroscope.x;
                    // data["y"] = sensorValue.un.gyroscope.y;
                    // data["z"] = sensorValue.un.gyroscope.z;
                    // data["dt"] = 1.0f/ frLinAcc;

                    s+="{\"gyro\": {";
                    s+="\"x\":";
                    s+=String(sensorValue.un.gyroscope.x,8);
                    s+=",\"y\":";
                    s+=String(sensorValue.un.gyroscope.y,8);
                    s+=",\"z\":";
                    s+=String(sensorValue.un.gyroscope.z,8);                    
                    s+=",\"dt\":";
                    s+=String((time-timeGyro) * 0.001,5);
                    // s+=",\"count\":";
                    // s+=String(countGyro++);
                    s+="}}";
                    timeGyro = time;
                    break;
                }

                case SH2_SHAKE_DETECTOR:
                {
                    // Serial.println("SH2_SHAKE_DETECTOR");
                    // JsonObject data = event.createNestedObject("shake");
                    // // event["type"] = "shake";
                    // data["x"] = (bool) (sensorValue.un.shakeDetector.shake & (1<<(0)));
                    // data["y"] = (bool) (sensorValue.un.shakeDetector.shake  & (1<<(1)));
                    // data["z"] = (bool) (sensorValue.un.shakeDetector.shake & (1<<(2)));
                    // data["raw"] = sensorValue.un.shakeDetector.shake;
                    break;
                }

                case SH2_TAP_DETECTOR:
                {
                    // // Serial.println("SH2_TAP_DETECTOR");
                    // JsonObject data = event.createNestedObject("tap");
                    // std::bitset<8> bits(sensorValue.un.tapDetector.flags);
                    // // event["type"] = "tap";
                    // data["double"] = bits.test(6);
                    // data["y"] =  int( bits.test(0)) * ((bits.test(1)) ? 1  : -1) ;
                    // //if(sensorValue.un.tapDetector.flags  & (1<<(1))) // direction

                    // data["x"] =int( bits.test(2)) * ((bits.test(3)) ? -1  : 1);
                    // data["z"] = int(bits.test(4)) * ((bits.test(5)) ? 1  : -1);
                    // data["raw"] = sensorValue.un.tapDetector.flags;
                    // data["flags"] = std::bitset<8>(sensorValue.un.tapDetector.flags).to_string();
                    break;
                }

                case SH2_TILT_DETECTOR:
                {
                    // Serial.println("SH2_TILT_DETECTOR");
                    
                    // JsonObject data = event.createNestedObject("tilt");
                   
                    // // event["type"] = "tap";
                    // data["tilt"] =  sensorValue.un.tiltDetector.tilt;
                    break;
                }

                case SH2_SIGNIFICANT_MOTION:
                {
                    Serial.println("SH2_SIGNIFICANT_MOTION");
                    
                    // event["sigMotion"] = (bool) (sensorValue.un.sigMotion.motion);
                    // event["type"] = "sigMotion";
                    // data["sigMotion"] = (bool) sensorValue.un.sigMotion.motion;
                    
                    break;
                }
                
                case  SH2_STABILITY_CLASSIFIER:
                {
                    //Serial.println("SH2_STABILITY_CLASSIFIER");
                    // JsonObject data = event.createNestedObject("stability");
                    // event["type"] = "stabilityClass";
                    sh2_StabilityClassifier_t stability = sensorValue.un.stabilityClassifier;
                    // data["raw"] = stability.classification;
                     s+="{\"stability\": {";
                    switch (stability.classification) {
                        case STABILITY_CLASSIFIER_UNKNOWN:
                            // data["class"] = "UNKNOWN";
                            s+="\"class\":";
                            s+="\"UNKNOWN\"";
                            break;
                        case STABILITY_CLASSIFIER_ON_TABLE:
                            // data["class"] = "ON_TABLE";
                            s+="\"class\":";
                            s+="\"ON_TABLE\"";
                            break;
                        case STABILITY_CLASSIFIER_STATIONARY:
                            // data["class"] = "STATIONARY";
                            s+="\"class\":";
                            s+="\"STATIONARY\"";
                            break;
                        case STABILITY_CLASSIFIER_STABLE:
                            // data["class"] = "STABLE";
                            s+="\"class\":";
                            s+="\"STABLE\"";
                            break;
                        case STABILITY_CLASSIFIER_MOTION:
                            // data["class"] = "MOTION";
                            s+="\"class\":";
                            s+="\"MOTION\"";
                        break;
                    }       
                    s+="}}";          
                    break;
                    
                   
                    
                 
                    
                }
                case SH2_STABILITY_DETECTOR:
                {
                    //Serial.println("SH2_STABILITY_DETECTOR");
                    // JsonObject data = event.createNestedObject("stability");
                    // data["raw"] = (int)sensorValue.un.stabilityDetector.stability;
                    // // data["test"] = 0;
                    // // std::bitset<16> bits{sensorValue.un.stabilityDetector.stability};
                    
                    bool newStable = isStable;
                    if(newStable)
                    {
                        newStable = (bitRead(sensorValue.un.stabilityDetector.stability,1)) ? false : true;
                    }
                    else
                    {
                        newStable = (bitRead(sensorValue.un.stabilityDetector.stability,0)) ? true : false;
                    }
                    // data["test"] = 1;
                   
                    // std::bitset<16> bits(sensorValue.un.stabilityDetector.stability);
                    // data["enteredStable"] =  (sensorValue.un.stabilityDetector.stability == 1) ? true : false;
                    // data["exitedStable"] =  (sensorValue.un.stabilityDetector.stability == 3) ? true : false;
                    // data["stable"] =  newStable;
                    isStable = newStable;

                    break;
                }

                

                    
            }
            
            // sensorEvents.add(event);
            // Serial.flush();
            // Serial.println((int)(imuJson.size()));
            
            
            // Serial.println("PROCESS - after serialization");
            
           
        }
        // Serial.println("]}");
        s+="]}";
        if(s.length() > 40)
        {
            Serial.println(s);
        }
        
        // if(sensorEvents.size() > 0)
        {
            // t0 = millis();
            
            // serializeJson(imuJson, Serial);
           
            // Serial.println();
            // Serial.print("time with Data: ");
        }
        // else
        //     Serial.print("time: ");
        // Serial.print("time: ");
        // Serial.println(millis()-t0);
    }

private:
   
    Adafruit_BNO08x  mpu;
    sh2_SensorValue_t sensorValue;

    bool connected = false;
   
    // static const int capacity = JSON_OBJECT_SIZE(32);

  
    
    int frStability = 5;

    float timeAcc = 0;        // Hz
    float timeRot = 0;
    float timeGyro = 0;
    
    float timeStability = 0;

    int countAcc = 0;
    int countGrav = 0;
    int countRot = 0;
    int countGyro = 0;
    

   
    

    // StaticJsonDocument<capacity> imuJson;

   

    volatile bool isRunning = false;

    int frameRate = 50;

    bool isStable = false;

    // State state;

    struct euler_t {
        float yaw;
        float pitch;
        float roll;
    } ypr;

    // const long rvUpdateRate = 

    

    void setReports()
    {
        Serial.println("SET REPORTS");
        

      
        if (! mpu.enableReport(SH2_ROTATION_VECTOR, 1000000.0/ this->frameRate)) {
            Serial.println("Could not enable SH2_ROTATION_VECTOR");
        }
        
        if (! mpu.enableReport(SH2_LINEAR_ACCELERATION, 1000000.0/ this->frameRate * 1.1)) {
            Serial.println("Could not enable SH2_LINEAR_ACCELERATION");
        }
        /*
        // if (! mpu.enableReport(SH2_ACCELEROMETER, 1000000.0/frLinAcc)) {
        //     Serial.println("Could not enable SH2_ACCELEROMETER");
        // }

        // if (! mpu.enableReport(SH2_SIGNIFICANT_MOTION, 500000)) {
        //     Serial.println("Could not enable SH2_SIGNIFICANT_MOTION");
        // }       
        // if (! mpu.enableReport(SH2_DEAD_RECKONING_POSE, 1000000.0/frRot)) {
        //     Serial.println("Could not enable SH2_DEAD_RECKONING_POSE");
        // }

        // if (! mpu.enableReport(SH2_SHAKE_DETECTOR, 100000)) {
        //     Serial.println("Could not enable SH2_SHAKE_DETECTOR");
        // }
        */
        if (! mpu.enableReport(SH2_GYROSCOPE_CALIBRATED, 1000000.0/ this->frameRate* 2)) {
            Serial.println("Could not enable SH2_GYROSCOPE_CALIBRATED");
        }
        /*
        // if (! mpu.enableReport(SH2_TAP_DETECTOR)) {
        //     Serial.println("Could not enable SH2_TAP_DETECTOR");
        // }   
        

        // if (! mpu.enableReport(SH2_TEMPERATURE)) {
        //     Serial.println("Could not enable SH2_TEMPERATURE");
        // }

        // if (! mpu.enableReport(SH2_GRAVITY, 1000000.0/frLinAcc)) {
        //     Serial.println("Could not enable SH2_GRAVITY");
        // }
        */
        if (! mpu.enableReport(SH2_STABILITY_CLASSIFIER, 1000000.0/frStability)) {
            Serial.println("Could not enable SH2_STABILITY_CLASSIFIER");
        }

        // if (! mpu.enableReport(SH2_TILT_DETECTOR)) {
        //     Serial.println("Could not enable SH2_TILT_DETECTOR");
        // }

        // if (! mpu.enableReport(SH2_STABILITY_DETECTOR, 1000000.0/10)) {
        //     Serial.println("Could not enable SH2_STABILITY_DETECTOR");
        // }
    }


    

    void quaternionToEuler(float qr, float qi, float qj, float qk, euler_t* ypr, bool degrees = false) {

        float sqr = sq(qr);
        float sqi = sq(qi);
        float sqj = sq(qj);
        float sqk = sq(qk);

        ypr->yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr));
        ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
        ypr->roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

        if (degrees) {
            ypr->yaw *= RAD_TO_DEG;
            ypr->pitch *= RAD_TO_DEG;
            ypr->roll *= RAD_TO_DEG;
        }
    }

  
};
#endif