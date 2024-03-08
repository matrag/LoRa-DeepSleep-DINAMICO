#include "main.h"

Melopero_RV3028 rtc;

void initRTC(){
  myLog_d("Date: %s", __DATE__);
  myLog_d("Time: %s", __TIME__);
  rtc.initI2C();
  rtc.writeToRegister(0x35,0x00);
  rtc.writeToRegister(0x37,0xB4); //Direct Switching Mode (DSM): when VDD < VBACKUP, switchover occurs from VDD to VBACKUP
  rtc.set24HourMode();  // Set the device to use the 24hour format (default) instead of the 12 hour format

  // Set the date and time
  // year, month, weekday, date, hour, minute, second
  // Note: time is always set in 24h format
  // Note: month value ranges from 1 (Jan) to 12 (Dec)
  // Note: date value ranges from 1 to 31
  rtc.setTime(2024, 1, 1, 22, 16, 3, 0);
  myLog_d("RTC:%i,%i,%i,%i,%i,%i", rtc.getYear(), rtc.getMonth(), rtc.getDate(), rtc.getHour(), rtc.getMinute(), rtc.getSecond() );
}