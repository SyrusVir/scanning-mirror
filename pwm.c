#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

#include<pigpio.h>

#define PWM_PIN 18
#define HW_PWM_RANGE 1000000

int main(int argc, char *argv[]) //usage: ./pwm [PWM frequency] [seconds to run] [additional microseconds to run]
{
	unsigned target_freq = 400;
	int run_time_sec = 30;
	int run_time_usec = 1000;

	if argc > 1
	{
		target_freq = atoi(argv[1]);
		if argc > 2
		{
			run_time_sec = atoi(argv[2]);
			if argc > 3
			{
				run_time_usec = atoi(argv[3]);
			}
		}
		

	}


	if(gpioInitialise() < 0) //returns version number on success; defaults to 5us sample rate. Effects available PWM frequencies
	{
		return 1;
	}

	if (gpioSetMode(PWM_PIN, PI_ALT5) != 0)
	{
		return 1;
	}


	if (gpioHardwarePWM(PWM_PIN, target_freq, (unsigned)HW_PWM_RANGE/2) != 0) //Hardware PWM has range of 1M; 
	{
		return 1;
	}

	gpioSleep(PI_TIME_RELATIVE,30,1000); //sleep for 30 seconds and 1000 microseconds.
	gpioHardwarePWM(PWM_PIN,0,0); //turn off PWM
	gpioTerminate();
	return 0;
}
