#include <main.h>

void setup(){
    pinMode(WB_IO2, OUTPUT);
    pinMode(LED_BUILTIN,OUTPUT);
}

void loop(){
    
    digitalWrite(WB_IO2,1);
	delay(5000);
	digitalWrite(WB_IO2,0);
	delay(5000);

}