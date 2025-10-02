#include <main.h>

//BME
	#define DEF_ADDR 0x1A
	GDK101_I2C gdk101(DEF_ADDR);
	float avg10min;
	float avg1min;
	float fw_vers;
	int _status;
	bool vibration;
	byte mea_min;
	byte mea_sec;
	//output
	char out_buff[48];

void initGDK(){
    gdk101.init();
  	//gdk101.reset();
  	fw_vers = gdk101.get_fw_version();
}

void getGDK(uint16_t * ten_avg_int, uint8_t * ten_avg_dec, uint8_t * min_avg_int, uint8_t * min_avg_dec){
    avg10min = gdk101.get_10min_avg();
	avg1min = gdk101.get_1min_avg();
	mea_min = gdk101.get_measuring_time_min();
	mea_sec = gdk101.get_measuring_time_sec();
	_status = gdk101.get_status();
	vibration = gdk101.get_vib();

    *ten_avg_int= avg10min; //put integer part into container
    *ten_avg_dec = (avg10min- (*min_avg_int)) * 100; //put decimal part into container
    *min_avg_int= avg1min; //put integer part into container
    *min_avg_dec = (avg1min- (*min_avg_int)) * 100; //put decimal part into container

    #if MYLOG_LOG_LEVEL > MYLOG_LOG_LEVEL_NONE
        sprintf(out_buff, "Time: %i:%i", mea_min, mea_sec);
        myLog_d(out_buff);
        sprintf(out_buff, "Radiation uSv/h 10min: %0.2f \t min: %0.2f ", avg10min, avg1min);
        myLog_d(out_buff);
        sprintf(out_buff, "Status: %i vibration: %i version %0.1f", _status, vibration, fw_vers);
        myLog_d(out_buff);
    #endif

}