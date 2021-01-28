#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<curses.h>
#include<ctype.h>

#include<pigpio.h>

#define SOS_PIN 17 //RPi pin 11
#define SPEED_PIN 23 //RPi pin 16
#define OUT_PIN 5 //RPi pin 29
#define PWM_PIN 18 //RPi pin 12
#define HW_PWM_RANGE 1000000

int main(int argc, char *argv[]) //usage: ./pwm [PWM frequency] [seconds to run] [additional microseconds to run]
{
	char filepath[] = "./pwm_test_logs.txt";
	FILE *f = fopen(filepath, "a");
	fprintf(f, "-------------------------------\n");
	fprintf(f,"TIME_S,TIME_US,ATSPEED,SOS\n");
	//terminal manipulations for non-blocking getch()
	initscr();
	nodelay(stdscr,true);
	noecho();

	//default variables
	unsigned target_freq = 400;
	int run_time_sec = 30;
	int run_time_usec = 1000;

	//parse command line arguments
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
	gpioSetMode(OUT_PIN, PI_OUTPUT);
	gpioSetMode(SPEED_PIN, PI_INPUT);
	gpioSetPullUpDown(SPEED_PIN, PI_PUD_UP);

	if (gpioHardwarePWM(PWM_PIN, target_freq, (unsigned)HW_PWM_RANGE/2) != 0) //Hardware PWM has range of 1M; 
	{
		return 1;
	}

	gpioWrite(OUT_PIN,0);
	
	//gpioSleep(PI_TIME_RELATIVE,run_time_sec,run_time_usec); //sleep for 30 seconds and 1000 microseconds.
	int sec;
	int usec;
	char ch;
	gpioTime(PI_TIME_RELATIVE, &sec, &usec);
	int stop_sec = sec + run_time_sec;

	uint8_t atspeed;
	uint8_t sos;
	while (sec < stop_sec)
	{
		ch = getch();
		if (toupper(ch) == 'Q') //early termination if 'q' is pressed
		{
			break;
		}
		
		atspeed = gpioRead(SPEED_PIN);
		sos = gpioRead(SOS_PIN);
		
		printf("ATSPEED LEVEL: %d\tSOS LEVEL: %d\n\r", atspeed,sos);
		fprintf(f,"%f,%d,%d,%d\n",sec,usec,atspeed,sos);
		gpioDelay(50000);
		gpioTime(PI_TIME_RELATIVE, &sec, &usec);
	}

	gpioHardwarePWM(PWM_PIN,0,0); //turn off PWM
	gpioWrite(OUT_PIN,0);

	gpioTerminate();
	endwin(); //restore terminal configuration
	fclose(f);
	return 0;
}
