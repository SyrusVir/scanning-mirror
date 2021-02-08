#include"scanmirror.h"
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<curses.h>
#include<ctype.h>


#define SOS_PIN 17 //RPi pin 11
#define SPEED_PIN 23 //RPi pin 16
#define EN_PIN 5 //RPi pin 29
#define PWM_PIN 18 //RPi pin 12
#define HW_PWM_RANGE 1000000

int main(int argc, char *argv[]) //usage: ./pwm [mirror RPM] [seconds to run] [additional microseconds to run]
{
	char filepath[] = "./pwm_test_logs.txt";
	FILE *f = fopen(filepath, "a");
	fprintf(f, "-------------------------------\n");
	fprintf(f,"MSEC_ELAPSED,ATSPEED,SOS\n");
	//terminal manipulations for non-blocking getch()
	initscr();
	nodelay(stdscr,true);
	noecho();

	//default variables ////
	unsigned target_freq = 400;
    uint16_t target_rpm = 1000;
	//if run_time vars are negative, then stop_tick < current tick so indefinite runtime
	int run_time_sec = 30; 
	int run_time_usec = 0;
	////////////////////////

	//parse command line arguments///
	if (argc > 1)
	{
		target_rpm = atoi(argv[1]);
		printf("freq = %u\n\r", (uint16_t)(target_rpm*24.0/60));
		if (argc > 2)
		{
			run_time_sec = atoi(argv[2]);
			if (argc > 3)
			{
				run_time_usec = atoi(argv[3]);
			}
		}
	}

	//GPIO Routines
	if(gpioInitialise() < 0) //returns version number on success; defaults to 5us sample rate. Effects available PWM frequencies
	{
		return 1;
	}


	gpioSetMode(SOS_PIN, PI_INPUT);
    
    mirror_t mirror;
    mirror.ATSPEED_PIN = SPEED_PIN;
    mirror.ENABLE_PIN = EN_PIN;
    mirror.FREQ_PIN = PWM_PIN;
    if (mirrorConfig(mirror) < 0) printf("Error configuring mirror pins\n");
	
    /**
    gpioSetMode(PWM_PIN, PI_ALT5) ;
	gpioSetMode(EN_PIN, PI_OUTPUT);
	gpioSetMode(SPEED_PIN, PI_INPUT);
	gpioSetPullUpDown(SPEED_PIN, PI_PUD_UP);
    **/
	
    if (mirrorEnable(mirror)) printf("Error enabling mirror\n");
    if (mirrorSetRPM(mirror,1000)) printf("Error setting RPM\n");
    /**
    if (gpioHardwarePWM(PWM_PIN, target_freq, (unsigned)HW_PWM_RANGE/2) != 0) //Hardware PWM has range of 1M; 
	{
		return 1;
	}

	gpioWrite(EN_PIN,1); //start disabled
	**/
	
    char ch;
	char state = 'D';
	uint8_t atspeed;
	uint8_t sos;
	uint32_t start_tick = gpioTick();
	uint64_t stop_tick = start_tick + run_time_sec*1000000 + run_time_usec;
	//printf("stop_tick= %llu\n\r", stop_tick);
	//printf("start_tick = %lu\n\r", start_tick);
	//printf("tick = %lu\n\r", gpioTick());
	while (gpioTick() < stop_tick) {

		ch = getch();
		if (toupper(ch) == 'Q') { //early termination if 'q' is pressed
			printf("QUITTING\n\r");
			break;
		}
		else if (toupper(ch) == 'E') {
			state = 'E';
			printf("ENABLING\n\r");
            if(mirrorEnable(mirror)) printf("Error enabling\n");
            if(mirrorSetRPM(mirror, target_rpm)) printf("Error setting rpm\n");			

			/**
            gpioWrite(EN_PIN,0); //enable
			gpioHardwarePWM(PWM_PIN, target_freq, (unsigned)HW_PWM_RANGE/2); //turn on pwm
            **/
        }
		else if (toupper(ch) == 'D') {
			state = 'D';
			printf("DISABLING\n\r");
            if(mirrorSetRPM(mirror, 0)) printf("Error setting rpm\n");
			if(mirrorDisable(mirror)) printf("Error disbaling\n");
            /**
            gpioHardwarePWM(PWM_PIN, 0,0); //turn off pwm
			gpioWrite(EN_PIN,1); //disable
            **/
		}// end ifs on getch
		
		switch (state) {
			case 'E':
				atspeed = mirrorCheckAtSpeed(mirror); //gpioRead(SPEED_PIN);
				sos = gpioRead(SOS_PIN);
				
				printf("ATSPEED LEVEL: %d\tSOS LEVEL: %d\n\r", atspeed,sos);
				fprintf(f,"%f,%d,%d\n",(gpioTick()-start_tick)/1000.0,atspeed,sos);
				break;
			default:
				break;
		}// end switch(state)
	}// end while

    mirrorSetRPM(mirror,0);
    mirrorDisable(mirror);
    /**
	gpioHardwarePWM(PWM_PIN,0,0); //turn off PWM
	gpioWrite(EN_PIN,0);
    **/

	gpioTerminate();
	endwin(); //restore terminal configuration
	fclose(f);
	return 0;
}
