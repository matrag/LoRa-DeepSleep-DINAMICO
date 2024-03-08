#include "main.h"

Adafruit_BME680 bme;

void init_bme680(void)
{
  Wire.begin();
  if (!bme.begin(BMEADDR)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    return;
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  //bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void bme680_get(uint8_t * t_int_pld, uint8_t * t_dec_pld, uint8_t * hum_int_pld, uint8_t * hum_dec_pld, uint16_t * press_pld)
{
  bme.performReading();
  double temp = bme.temperature;
  *t_int_pld = temp; //put integer part into container
  *t_dec_pld = (temp- (*t_int_pld)) * 100; //put decimal part into container

  double hum = bme.humidity;
  *hum_int_pld = hum; //put integer part into container
  *hum_dec_pld = (hum- (*hum_int_pld)) * 100; //put decimal part into container

  double press = bme.pressure/100;
  *press_pld = press; //put integer part into container
  
  #if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
	//String data = "Tem:" + String(temp) + "C " + "Hum:" + String(hum) + "% " + "Pres:" + String(press) + "KPa ";
	myLog_d("Tem: %2.2f C; Hum: %2.2f RH; Press: %2.2f hPa", temp, hum, press);
 	// data = "Tem int:" + String(*t_int_pld) + "C " + "Hum int:" + String(*hum_int_pld) + "% " + "Pres:" + String(*press_pld) + "KPa ";
	// Serial.println(data);
	// data = "Tem dec:" + String(*t_dec_pld) + "C " + "Hum dec:" + String(*hum_dec_pld);
	// Serial.println(data);
  #endif
}