#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<curses.h>
#include<ctype.h>

#include<pigpio.h>

#define SOS_PIN 17 //RPi pin 11
#define SPEED_PIN 23 //RPi pin 16
#define ENABLE_PIN 5 //RPi pin 29
#define PWM_PIN 18 //RPi pin 12
#define HW_PWM_RANGE 1000000

int main(int argc, char *argv[]) //usage: ./pwm [PWM frequency] [seconds to run] [additional microseconds to run]
{
	char filepath[] = "./pwm_test_logs.txt";
	FILE *f = fopen(filepath, "a");
	fprintf(f, "-------------------------------\n");
	fprintf(f,"USEC_ELAPSED,ATSPEED,SOS\n");
	//terminal manipulations for non-blocking getch()
	initscr();
	nodelay(stdscr,true);
	noecho();

	//default variables ////
	unsigned target_freq = 400;
	//if run_time vars are negative, then stop_tick < current tick so indefinite runtime
	int run_time_sec = 30; 
	int run_time_usec = 0;
	////////////////////////

	//parse command line arguments///
	if (argc > 1)
	{
		target_freq = atoi(argv[1]);
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
	gpioSetMode(PWM_PIN, PI_ALT5) ;
	gpioSetMode(ENABLE_PIN, PI_OUTPUT);
	gpioSetMode(SPEED_PIN, PI_INPUT);
	gpioSetPullUpDown(SPEED_PIN, PI_PUD_UP);

	if (gpioHardwarePWM(PWM_PIN, target_freq, (unsigned)HW_PWM_RANGE/2) != 0) //Hardware PWM has range of 1M; 
	{
		return 1;
	}

	gpioWrite(ENABLE_PIN,1); //start disabled
	
	//gpioSleep(PI_TIME_RELATIVE,run_time_sec,run_time_usec); //sleep for 30 seconds and 1000 microseconds.
	char ch;

	uint32_t start_tick = gpioTick();
	uint64_t stop_tick = start_tick + run_time_sec*1000000 + run_time_usec;
	printf("%u, %llu", gpioTick(), stop_tick);
	uint8_t atspeed;
	uint8_t sos;
	while (gpioTick() < stop_tick)
	{
		ch = getch();
		if (toupper(ch) == 'Q') { //early termination if 'q' is pressed
			printf("QUITTING\n\r");
			break;
		}
		else if (toupper(ch) == 'E') {
			printf("ENABLING\n\r");			
			gpioWrite(ENABLE_PIN,0); //enable
			gpioHardwarePWM(PWM_PIN, target_freq, (unsigned)HW_PWM_RANGE/2); //turn on pwm
		}
		else if (toupper(ch) == 'D') {
			printf("DISABLING\n\r");
			gpioHardwarePWM(PWM_PIN, 0,0); //turn off pwm
			gpioWrite(ENABLE_PIN,1); //disable

		}
		atspeed = gpioRead(SPEED_PIN);
		sos = gpioRead(SOS_PIN);
		
		printf("ATSPEED LEVEL: %d\tSOS LEVEL: %d\n\r", atspeed,sos);
		fprintf(f,"%f,%d,%d\n",(gpioTick()-start_tick)/1000.0,atspeed,sos);
		gpioDelay(1000);
	}

	gpioHardwarePWM(PWM_PIN,0,0); //turn off PWM
	gpioWrite(ENABLE_PIN,0);

	gpioTerminate();
	endwin(); //restore terminal configuration
	fclose(f);
	return 0;
}
