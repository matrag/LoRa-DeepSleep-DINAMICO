// Gamma Sensor's Example Interface
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
/*
 * We will be using the I2C hardware interface on the Arduino in
 * combination with the built-in Wire library to interface.
 * Arduino analog input 5 - I2C SCL
 * Arduino analog input 4 - I2C SDA
 *
 * Command List
 * 0xA0 :: Reset
 * 0xB0 :: Read Status
 * 0xB1 :: Read Measuring Time
 * 0xB2 :: Read Measuring Value (10min avg / 1min update)
 * 0xB3 :: Read Measuring Value (1min avg / 1min update)
 * 0xB4 :: Read Firmware Version
 *
 * Address Assignment
 * Default Address :: 0x18
 * A0 Open, A1 Short :: 0x19
 * A0 Short, A1 Open :: 0x1A
 * A0 Open, A1 Open :: 0x1B
 */
int addr = 0x1A;
int day,hour,minute,sec = 0;
byte buffer[2] = {0,0};
int status = 0;

/*
 * Calculation Measuring Time
 * Format :: 0d 00:00:00 ( (day)d (hour):(min):(sec) )
 */
void Cal_Measuring_Time(){
 if(sec == 60) { sec = 0; minute++; }
 if(minute == 60) { minute = 0; hour++; }
 if(hour == 24) { hour = 0; day++; }

 Serial.print("Measuring Time\t\t\t");
 Serial.print(day); Serial.print("d ");
 if(hour < 10) Serial.print("0");
 Serial.print(hour); Serial.print(":");
 if(minute < 10) Serial.print("0");
 Serial.print(minute); Serial.print(":"); 
 if(sec < 10) Serial.print("0");
 Serial.println(sec);
}

void Print_Result(int cmd){
 float value = 0.0f;
 switch(cmd){
    case 0xA0:
        Serial.print("Reset Response\t\t\t");
        if(buffer[0]== 1) Serial.println("Reset Success.");
        else Serial.println("Reset Fail(Status - Ready).");
    break;

    case 0xB0:
        Serial.print("Status\t\t\t\t");
        switch(buffer[0]){
            case 0: {
                Serial.print(buffer[0]);
                Serial.println(" :Ready");
            }
            break;
            case 1: Serial.println("10min Waiting"); break;
            case 2: Serial.println("Normal"); break;
        }
        status = buffer[0];

        Serial.print("VIB Status\t\t\t");
        switch(buffer[1]){
        case 0: Serial.println("OFF"); break;
        case 1: Serial.println("ON"); break;
        }
    break;

    case 0xB1:
        if(status > 0){
        sec++;
        Cal_Measuring_Time();
    }
    break;

    case 0xB2:
        Serial.print("Measuring Value(10min avg)\t");
        value = buffer[0] + (float)buffer[1]/100;
        Serial.print(value); Serial.println(" uSv/hr");
    break;

    case 0xB3:
        Serial.print("Measuring Value(1min avg)\t");
        value = buffer[0] + (float)buffer[1]/100;
        Serial.print(value); Serial.println(" uSv/hr");
    break; 

    case 0xB4:
        Serial.print("FW Version\t\t\t");
        Serial.print("V"); Serial.print(buffer[0]);
        Serial.print("."); Serial.println(buffer[1]);
    break; 
 }
}

void Gamma_Mod_Read(int cmd){
 /* Begin Write Sequence */
 Wire.beginTransmission(addr);
 Wire.write(cmd);
 Wire.endTransmission();
 /* End Write Sequence */
 delay(10);
 /* Begin Read Sequence */
 Wire.requestFrom(addr, 2);
 byte i = 0;
 while(Wire.available())
 {
 buffer[i] = Wire.read();
 i++;
 }
 /* End Read Sequence */
 
 /* View Results */
 Print_Result(cmd);
}

void Gamma_Mod_Read_Value(){
 Gamma_Mod_Read(0xB0); // Read Status
 Gamma_Mod_Read(0xB1); // Read Measuring Time
 Gamma_Mod_Read(0xB2); // Read Measuring Value (10min avg / 1min update)
 Gamma_Mod_Read(0xB3); // Read Measuring Value (1min avg / 1min update)
 Gamma_Mod_Read(0xB4);
}


void setup() {
 //Arduino Initialize
 Serial.begin(9600);
 Wire.begin();

 digitalWrite(LED_BUILTIN,1);
 delay(1000);
 digitalWrite(LED_BUILTIN, 0);
 delay(1000);
 digitalWrite(LED_BUILTIN, 1);
 delay(1000);
 digitalWrite(LED_BUILTIN, 0);
 Serial.println("Gamma Sensor Sensing Start");

 //Read Firmware version
 Gamma_Mod_Read(0xB4);
 //Reset before operating the sensor
 Gamma_Mod_Read(0xA0);
 Serial.println("================================================");
}

void loop() {
 delay(10000);
 //Read Statue, Measuring Time, Measuring Value
 Gamma_Mod_Read_Value();
 Serial.println("================================================");
}

