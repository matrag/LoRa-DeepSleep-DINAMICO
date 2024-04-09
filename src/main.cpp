/**
 * @file main.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief LoRa-DeepSleep setup() and loop()
 * @version 0.1
 * @date 2021-01-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "main.h"


/** Semaphore used by events to wake up loop task */
SemaphoreHandle_t taskEvent = NULL;
/** Timer to wakeup task frequently and send message */
SoftwareTimer taskWakeupTimer;

/** Buffer for received LoRaWan data */
uint8_t rcvdLoRaData[256];
/** Length of received data */
uint8_t rcvdDataLen = 0;

TxdPayload txPayload;
TxdPayload PldArray [NODEID] = {};
PldWrapper pldWrap;
uint16_t nodeSentPackets = 0;
uint16_t nodeReceivedPackets = 0;

//A0 Short, A1 Short : 0x18
//A0 Open,  A1 Short : 0x19
//A0 Short, A1 Open  : 0x1A
//A0 Open,  A1 Open  : 0x1B


/**
 * @brief Flag for the event type
 * -1 => no event
 * 0 => LoRaWan data received
 * 1 => Timer wakeup
 * 2 => tbd
 * ...
 */
uint8_t eventType = -1;

/**
 * @brief Timer event that wakes up the loop task frequently
 * 
 * @param unused 
 */
void periodicWakeup(TimerHandle_t unused)
{
	// Switch on blue LED to show we are awake
	#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
		digitalWrite(LED_CONN, HIGH);
	#endif
	eventType = 1;
	// Give the semaphore, so the loop task will wake up
	xSemaphoreGiveFromISR(taskEvent, pdFALSE);
}


void setup()
{	
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2,1);		
	
	txPayload.id = NODEID; //set the node id

	// Setup the build in LED
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(LED_CONN, OUTPUT);
	digitalWrite(LED_CONN, LOW);

	#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
		digitalWrite(LED_BUILTIN, HIGH);
		// Start serial
		Serial.begin(115200);

		// Wait seconds for a terminal to connect
		time_t timeout = millis();
		while (!Serial)
		{
			//Blink led while serial connects
			if ((millis() - timeout) < 2000) 
			{
				delay(200);
				digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
			}
			else
			{
				break;
			}
		}
		myLog_d("====================================");
		myLog_d("LoRa P2P deep sleep implementation");
		myLog_d("====================================");
	#endif

	// Switch off LED
	digitalWrite(LED_BUILTIN, LOW);

	// Create the semaphore for the loop task
	myLog_d("Create task semaphore");
	delay(100); // Give Serial time to send
	taskEvent = xSemaphoreCreateBinary();

	// Give the semaphore, seems to be required to initialize it
	myLog_d("Initialize task Semaphore");
	delay(100); // Give Serial time to send
	xSemaphoreGive(taskEvent);

	// Take the semaphore, so loop will be stopped waiting to get it
	myLog_d("Take task Semaphore - loop task stopped");
	delay(100); // Give Serial time to send
	xSemaphoreTake(taskEvent, 10);

	// Start LoRa
	if (!initLoRa())
	{
		myLog_e("Init LoRa failed");
		while (1)
		{
			digitalWrite(LED_CONN,1);
			delay(50);
			digitalWrite(LED_CONN,0);
			delay(300);
		}
	}
	myLog_d("Init LoRa success");

	/* bme680 init */
  	init_bme680();

	/* vbat adc init */
  	initReadVBAT();

	/* acc init */
  	if (initACC())
		myLog_d("Init acc success");

	//initRTC();

	// Now we are connected, start the timer that will wakeup the loop frequently
	myLog_d("Start Wakeup Timer");

	//If the programmed node is an intermediate chain element, sleep for different time than the first node
	#ifdef IS_CHAIN_ELEMENT
		taskWakeupTimer.begin(SLEEP_TIME_CHAIN, periodicWakeup);
	#else
		taskWakeupTimer.begin(SLEEP_TIME, periodicWakeup);
	#endif

	#ifdef RAD_NODE
		taskWakeupTimer.begin(SLEEP_TIME_RAD, periodicWakeup);
	#endif

	uint32_t wdtMS = Watchdog.enable(10000); //enable watchdog for MS
    myLog_d("Enabled the watchdog with max countdown of %i", wdtMS);

	taskWakeupTimer.start();

	// Give Serial some time to send everything
	delay(100);
	//Turn off i2c sensors
	digitalWrite(WB_IO2,0);
}

void loop()
{
	// Sleep until we are woken up by an event
	if (xSemaphoreTake(taskEvent, portMAX_DELAY) == pdTRUE)
	{
	Watchdog.reset(); // Reset watchdog with every loop to make sure the sketch keeps running.
	// Switch on green LED to show we are awake
	#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
			digitalWrite(LED_BUILTIN, HIGH);
			delay(500); // Only so we can see the green LED
	#endif

		// Check the wake up reason
		switch (eventType)
		{
		case 0: // Wakeup reason is package downlink arrived
			myLog_d("Received package over LoRa");
			nodeReceivedPackets ++;
			break;
		case 1: // Wakeup reason is timer
		{
			myLog_d("Timer wakeup");
			
			int rcvTxPayloadIndex = pldWrap.wrSize / sizeof(txPayload);
			myLog_d("dimensions of received: %i, index of received: %i", pldWrap.wrSize, rcvTxPayloadIndex);
			handleLoopActions();
			PldArray[rcvTxPayloadIndex] = txPayload; //fill the payload with the node data
			pldWrap.wrSize = sizeof(txPayload) * (rcvTxPayloadIndex + 1);
			myLog_d("payload size of tx data: %i", pldWrap.wrSize);

			#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE 
				myLog_d("Payload filled in loop: ");
				char rcvdData[pldWrap.wrSize * 4] = {0};
				uint8_t PldPrintBuffer [pldWrap.wrSize] = {0};
				memcpy(PldPrintBuffer, &PldArray[0], pldWrap.wrSize);
				int index = 0;
				for (int idx = 0; idx < pldWrap.wrSize * 3; idx += 3)
				{
					sprintf(&rcvdData[idx], "%02x ", PldPrintBuffer[index++]);
				}
				myLog_d(rcvdData);
				delay(DEFWAIT);	
			#endif

			pldWrap.wrBuffer = PldArray;

			#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE 
				myLog_d("Payload copied in wrapper: ");
				memcpy(PldPrintBuffer, pldWrap.wrBuffer, pldWrap.wrSize);
				index = 0;
				for (int idx = 0; idx < (pldWrap.wrSize * 3); idx += 3)
				{
					sprintf(&rcvdData[idx], "%02x ", PldPrintBuffer[index++]);
				}
				myLog_d(rcvdData);
				delay(DEFWAIT);	
			#endif
			// Send the data package
			myLog_d("Initiate sending");

			// #if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
			// 	//sprintf(&rcvdData[0], "%i", PldArray[NODEID-1].pckLength);
			// 	//myLog_d("last packet received Length: %d", PldArray[NODEID-1].pckLength ); 
			// 	myLog_d("last packet received RSSI: -%d dBm", PldArray[NODEID-1].rssi_last );
			// 	myLog_d("last packet received snr: %d dB", PldArray[NODEID-1].snr_last );
			// 	delay(DEFWAIT);
			// #endif

			sendLoRa();
			break;
		}
		default:
			myLog_d("This should never happen ;-)");
			NVIC_SystemReset();
			break;
		}

		myLog_d("Loop goes back to sleep.\n");
		// Go back to sleep - take the loop semaphore
		xSemaphoreTake(taskEvent, 10);

	#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
			digitalWrite(LED_BUILTIN, LOW);
	#endif
	}
}

/* update txPayload with sensor data*/
void handleLoopActions(){
	txPayload.id = NODEID;
	digitalWrite(WB_IO2,1); //Enable sensor supply
	delay(DEFWAIT);
	bme680_get(&txPayload.temp_int, &txPayload.temp_dec, &txPayload.humdity_int, &txPayload.humdity_dec, &txPayload.bar_press);
	txPayload.bat_perc = readBatt();

	#ifdef RAD_NODE
		digitalWrite(LED_BUILTIN, 0);
		digitalWrite(LED_BLUE,0);
		delay(RAD_WAIT);
		/*Use press, incx, incy, incz containers for rad avg 1/10 min data*/
		getGDK(&txPayload.bar_press, &txPayload.inc_x, &txPayload.inc_y, &txPayload.inc_z);
		digitalWrite(LED_BUILTIN, 1);
	#else
		float accx = accSensor.readFloatAccelX();
		float accy = accSensor.readFloatAccelY();
		float accz = accSensor.readFloatAccelZ();

		#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
			myLog_d("Acc X: %f", accx ); 
			myLog_d("Acc y: %f", accy );
			myLog_d("Acc z: %f",accz );
			delay(DEFWAIT);
		#endif
		calculateTilt(accx, accy, accz, &txPayload.inc_x, &txPayload.inc_y, &txPayload.inc_z);
		#if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
			myLog_d("Inc X: %i", txPayload.inc_x ); 
			myLog_d("Inc y: %i", txPayload.inc_y );
			myLog_d("Inc z: %i", txPayload.inc_z );
			delay(DEFWAIT);
		#endif

	#endif

	txPayload.sentPackets = nodeSentPackets;
	txPayload.receivedPackets = nodeReceivedPackets;
	//txPayload.nodeUnixTime = rtc.getUnixTime();
	//myLog_d("Unix time: %li", rtc.getUnixTime());
	//Serial.println(rtc.getUnixTime());

	digitalWrite(WB_IO2,0); //Turn off sensor supply (GDK off)
}