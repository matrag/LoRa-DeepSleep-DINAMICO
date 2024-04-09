#include <algorithm>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_SleepyDog.h>

//BME functions
	#include <Adafruit_Sensor.h>
	#include <Adafruit_BME680.h>
	#define BMEADDR 0x76
	#define PRESS_DIV 1000

// ACC functions
	#include <SparkFunLIS3DH.h>
	#define INT1_PIN 21
	extern LIS3DH accSensor;
	bool initACC(void);
	void clearAccInt(void);
	void calculateTilt(float xacc, float yacc, float zacc, uint8_t * xinc, uint8_t * yinc, uint8_t * zinc);
	extern SemaphoreHandle_t loopEnable;

// Battery functions
	/** Definition of the Analog input that is connected to the battery voltage divider */
	#define PIN_VBAT A0
	/** Definition of milliVolt per LSB => 3.0V ADC range and 12-bit ADC resolution = 3000mV/4096 */
	#define VBAT_MV_PER_LSB (0.73242188F)
	/** Voltage divider value => 1.5M + 1M voltage divider on VBAT = (1.5M / (1M + 1.5M)) */
	#define VBAT_DIVIDER (0.4F)
	/** Compensation factor for the VBAT divider */
	#define VBAT_DIVIDER_COMP (1.73)
	/** Fixed calculation of milliVolt from compensation value */
	#define REAL_VBAT_MV_PER_LSB (VBAT_DIVIDER_COMP * VBAT_MV_PER_LSB)
	float readVBAT(void);
	void initReadVBAT(void);
	uint8_t readBatt(void);
	uint8_t lorawanBattLevel(void);
	extern uint8_t battLevel;

//GDK101 rad sensor
	#include <gdk101_i2c.h>
	void initGDK();
	void getGDK(uint16_t * ten_avg_int, uint8_t * ten_avg_dec, uint8_t * min_avg_int, uint8_t * min_avg_dec);

//RTC stuff
	#include "Melopero_RV3028.h"
	extern Melopero_RV3028 rtc;
	void initRTC();

// Debug
#include <myLog.h>
//#define MYLOG_LOG_LEVEL MYLOG_LOG_LEVEL_NONE //Uncomment to remove debug and LED

#include <SX126x-RAK4630.h>

//default wait time for prints and stuff
	#define DEFWAIT 10
//wait for RAD node (ms)
	#define RAD_WAIT 660000

//chain elements definitions
	//node IDentifier
	#define NODEID 3
	// LoRa stuff
	#if NODEID == 1 	//check if the node is a relay or not
		#define TX_ONLY
	#else
		#define IS_CHAIN_ELEMENT //UNCOMMENT if the node is a chain element (excl. first  node)
	#endif

	#define RAD_NODE //define if the node is a radiation monitor or normal node

	/* Time the device is sleeping in milliseconds for chain elements */
	#define SLEEP_TIME_CHAIN 600 * 1000
	/* Time the device is sleeping in milliseconds for FIRST node element */
	#define SLEEP_TIME 600 * 1000
	/* Time the device is sleeping in milliseconds for RAD node element */
	#define SLEEP_TIME_RAD 600 * 1000

struct __attribute__((packed)) TxdPayload{
		uint8_t id = NODEID;	             // Device ID
		uint8_t bat_perc;		 // Battery percentage
		uint8_t temp_int;        // Temperature integer
		uint8_t temp_dec;		 // Temperature tenths/hundredths
		uint8_t humdity_int;	 // Humidity integer
		uint8_t humdity_dec;	 // Humidity ones/tens/hundreds
		uint16_t bar_press;		 // Barometric pressure in hPa
		uint8_t inc_x;
		uint8_t inc_y;
		uint8_t inc_z;
		uint8_t rssi_last = 255; // Strength of last received signal
		uint8_t snr_last = 255;
		uint16_t sentPackets;
		uint16_t receivedPackets;
		//uint32_t nodeUnixTime;
	};

//Payload Array
extern TxdPayload PldArray [NODEID];
extern TxdPayload txPayload;
extern TxdPayload rxPayload [];
extern int rcvSize;

struct __attribute__((packed)) PldWrapper {
	int wrSize = 0;
	TxdPayload* wrBuffer;
};
extern PldWrapper pldWrap;

extern uint16_t nodeSentPackets;
extern uint16_t nodeReceivedPackets;

// LoRa stuff
bool initLoRa(void);
void sendLoRa(void);

//BME stuff
extern Adafruit_BME680 bme;
void init_bme680();
void bme680_get(uint8_t * t_int_pld, uint8_t * t_dec_pld, uint8_t * hum_int_pld, uint8_t * hum_dec_pld, uint16_t * press_pld);

// Main loop stuff
void periodicWakeup(TimerHandle_t unused);
extern SemaphoreHandle_t taskEvent;
extern uint8_t rcvdLoRaData[];
extern uint8_t rcvdDataLen;
extern uint8_t eventType;
extern SoftwareTimer taskWakeupTimer;
extern void handleLoopActions();




