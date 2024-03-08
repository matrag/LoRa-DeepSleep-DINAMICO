#include "main.h"

#define OC_PIN    WB_IO3
#define RELAY_PIN WB_IO4
#define VS_PIN WB_IO2

void initRelay(){
    pinMode(OC_PIN,INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(VS_PIN, OUTPUT);
    digitalWrite(VS_PIN, HIGH);
}

void openRelay(){
    digitalWrite( RELAY_PIN , LOW);
}

void closeRelay(){
    digitalWrite( RELAY_PIN , HIGH);
}