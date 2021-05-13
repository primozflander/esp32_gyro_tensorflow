#include <Arduino.h>
#include "Wire.h"
#include <MPU6050_light.h>
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "model.h"

#define SAMPLE_RATE_MS 1
#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))
const float accelerationThreshold = 2.5;
const int numSamples = 100;
int samplesRead = numSamples;
long timer = 0;
MPU6050 mpu(Wire);
tflite::MicroErrorReporter tflErrorReporter;
tflite::ops::micro::AllOpsResolver tflOpsResolver;
const tflite::Model *tflModel = nullptr;
tflite::MicroInterpreter *tflInterpreter = nullptr;
TfLiteTensor *tflInputTensor = nullptr;
TfLiteTensor *tflOutputTensor = nullptr;
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize];
const char *GESTURES[] = {
    "punch",
    "hook"};

void getImuOffsets()
{
    Serial.println("Calculating offsets, do not move MPU6050");
    mpu.calcOffsets();
    Serial.println("ax:" + String(mpu.getAccXoffset()) + " ay:" + String(mpu.getAccYoffset()) + " az:" + String(mpu.getAccZoffset()));
    Serial.println("gx:" + String(mpu.getGyroXoffset()) + " gy:" + String(mpu.getGyroYoffset()) + " gz:" + String(mpu.getGyroZoffset()));
}

void showImuData()
{
    Serial.println("ax:" + String(mpu.getAccX()) + " ay:" + String(mpu.getAccY()) + " az:" + String(mpu.getAccZ()) +
                   " gx:" + String(mpu.getGyroX()) + " gy:" + String(mpu.getGyroY()) + " gz:" + String(mpu.getGyroZ()) +
                   " yaw:" + String(mpu.getAngleX()) + " pitch:" + String(mpu.getAngleY()) + " roll:" + String(mpu.getAngleZ()));
}

void classifyData()
{
    while (1)
    {
        while (samplesRead == numSamples)
        {
            mpu.update();
            float aSum = fabs(mpu.getAccX()) + fabs(mpu.getAccY()) + fabs(mpu.getAccZ());
            if (aSum >= accelerationThreshold)
            {
                samplesRead = 0;
                break;
            }
        }
        while (samplesRead < numSamples)
        {
            tflInputTensor->data.f[samplesRead * 6 + 0] = (mpu.getAccX() + 4.0) / 8.0;
            tflInputTensor->data.f[samplesRead * 6 + 1] = (mpu.getAccY() + 4.0) / 8.0;
            tflInputTensor->data.f[samplesRead * 6 + 2] = (mpu.getAccZ() + 4.0) / 8.0;
            tflInputTensor->data.f[samplesRead * 6 + 3] = (mpu.getGyroX() + 2000.0) / 4000.0;
            tflInputTensor->data.f[samplesRead * 6 + 4] = (mpu.getGyroY() + 2000.0) / 4000.0;
            tflInputTensor->data.f[samplesRead * 6 + 5] = (mpu.getGyroZ() + 2000.0) / 4000.0;
            samplesRead++;
            if (samplesRead == numSamples)
            {
                TfLiteStatus invokeStatus = tflInterpreter->Invoke();
                if (invokeStatus != kTfLiteOk)
                {
                    Serial.println("Invoke failed!");
                    while (1);
                    return;
                }
                int count = 0;
                for (int i = 0; i < NUM_GESTURES; i++)
                {
                    Serial.print(GESTURES[i]);
                    Serial.print(": ");
                    Serial.println(tflOutputTensor->data.f[i], 6);
                    count++;
                }
                Serial.println();
                delay(1000);
            }
        }
    }
}

void captureData()
{
    Serial.println("aX,aY,aZ,gX,gY,gZ");
    while (1)
    {
        while (samplesRead == numSamples)
        {
            mpu.update();
            float aSum = fabs(mpu.getAccX()) + fabs(mpu.getAccY()) + fabs(mpu.getAccZ());
            if (aSum >= accelerationThreshold)
            {
                samplesRead = 0;
                break;
            }
        }
        while (samplesRead < numSamples)
        {
            //  if(millis() - timer > 10){
            mpu.update();
            samplesRead++;
            Serial.println(String(mpu.getAccX(), 3) + "," + String(mpu.getAccY(), 3) + "," + String(mpu.getAccZ(), 3) +
                           "," + String(mpu.getGyroX(), 3) + "," + String(mpu.getGyroY(), 3) + "," + String(mpu.getGyroZ(), 3));
            if (samplesRead == numSamples)
            {
                Serial.println();
            }
            //      timer = millis();
            // }
        }
    }
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(400000);
    mpu.begin();
    //getImuOffsets();
    mpu.setAccOffsets(0.05, 0.00, 0.00);
    mpu.setGyroOffsets(0.86, -2.30, -1.69);
    tflModel = tflite::GetModel(model);
    if (tflModel->version() != TFLITE_SCHEMA_VERSION)
    {
        Serial.println("Model schema mismatch!");
        while (1)
            ;
    }
    tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);
    tflInterpreter->AllocateTensors();
    tflInputTensor = tflInterpreter->input(0);
    tflOutputTensor = tflInterpreter->output(0);
    Serial.println("Ready!");
}

void loop()
{
    // captureData();
    classifyData();
}