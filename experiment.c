#include<pigpio.h>
#include<stdio.h>
#include<curses.h>

#define OUT_PIN 23 //GPIO 23 = Pin 16
#define CYC_SEC 5

int new_delay(int run_sec)
{
	int start_sec;
	int start_usec;
	gpioTime(PI_TIME_RELATIVE,&start_sec,&start_usec);
	int stop_sec = start_sec + run_sec;

	char ch;
	while (start_sec < stop_sec)
	{
		ch = getch();
		if (ch == 'q')
		{
			return 1;
		}
		gpioDelay(5000); //5 ms delay
		gpioTime(PI_TIME_RELATIVE,&start_sec,&start_usec);
	}

	return 0;
}

int main()
{
	initscr();
	nodelay(stdscr,true);

	gpioInitialise();
	gpioSetMode(OUT_PIN, PI_OUTPUT);


	while(1)
	{
		printf("%ld\n\r",gpioTick());
		gpioDelay(5000);
	}

	gpioWrite(OUT_PIN,0);
	gpioTerminate();
	endwin();
	return 0;
}
