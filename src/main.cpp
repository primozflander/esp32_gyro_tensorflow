#include <Arduino.h>
#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

long timer = 0;

void getImuOffsets()
{
    Serial.println("Calculating offsets, do not move MPU6050");
    mpu.calcOffsets();
    Serial.println("ax:" + String(mpu.getAccXoffset()) + " ay:" + String(mpu.getAccYoffset()) + " az:" + String(mpu.getAccZoffset()));
    Serial.println("gx:" + String(mpu.getGyroXoffset()) + " gy:" + String(mpu.getGyroYoffset()) + " gz:" + String(mpu.getGyroZoffset()));
}

void setup()
{
    Serial.begin(9600);
    Wire.begin();
    mpu.begin();
    // Serial.print(F("MPU6050 status: "));
    // Serial.println(status);
    // while (status != 0) {}
    //getImuOffsets();
    mpu.setAccOffsets(0.05, 0.00, 0.00);
    mpu.setGyroOffsets(0.86, -2.30, -1.69);
}

void loop()
{
    mpu.update();

    if (millis() - timer > 1000)
    { 

        // Serial.print("ax"mpu.getAccX());
        // Serial.print("\tY: ");
        // Serial.print(mpu.getAccY());
        // Serial.print("\tZ: ");
        // Serial.println(mpu.getAccZ());

        // Serial.print(F("GYRO      X: "));
        // Serial.print(mpu.getGyroX());
        // Serial.print("\tY: ");
        // Serial.print(mpu.getGyroY());
        // Serial.print("\tZ: ");
        // Serial.println(mpu.getGyroZ());

        // Serial.print(F("ACC ANGLE X: "));
        // Serial.print(mpu.getAccAngleX());
        // Serial.print("\tY: ");
        // Serial.println(mpu.getAccAngleY());

        // Serial.print(F("ANGLE     X: "));
        // Serial.print(mpu.getAngleX());
        // Serial.print("\tY: ");
        // Serial.print(mpu.getAngleY());
        // Serial.print("\tZ: ");
        // Serial.println(mpu.getAngleZ());
        // Serial.println(F("=====================================================\n"));
        Serial.println("ax:" + String(mpu.getAccX()) + " ay:" + String(mpu.getAccY()) + " az:" + String(mpu.getAccZ())
         + " gx:" + String(mpu.getGyroX()) + " gy:" + String(mpu.getGyroY()) + " gz:" + String(mpu.getGyroZ())
         + " yaw:" + String(mpu.getAngleX()) + " pitch:" + String(mpu.getAngleY()) + " roll:" + String(mpu.getAngleZ()));
        timer = millis();
    }
}