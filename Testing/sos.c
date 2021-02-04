#include<pigpio.h>
#include<curses.h>


#define IN_PIN 17 //Rpi pin 11
#define SAMPLE_PERIOD 500000

int main() {
	//Ncurses initialization
	initscr();
	nodelay(stdscr,true);
	
	gpioInitialise();
	gpioSetMode(IN_PIN, PI_INPUT);
	
	char ch = getch();
	while (ch != 'q') {
		printf("GPIO%d Level: %d\n\r",IN_PIN, gpioRead(IN_PIN));
		gpioDelay(SAMPLE_PERIOD);
		ch = getch();
	}
	

	gpioTerminate();
	endwin();
}
