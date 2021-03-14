#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h> 
#include <ncurses.h>
#include "sos_poller.c"

#define THREAD_IN 23        //physical pin 16; pin that the threaded poller will poll
#define THREAD_OUT 24       //physical pin 18; This pin will be pulled LO when THREAD_IN is polled as LO
#define THREAD_CONTROL 25   //physical pin 22 output pin wired to THREAD_IN

#define DELAY_USEC 500 //microseconds after THREAD_IN or ALERT_IN is triggered to reset *_OUT pin to HI

int main() 
{
    //ncurses init
    initscr();
    nodelay(stdscr,FALSE);
    noecho();

    //Pigpiio intialization
    gpioCfgClock(2,1,0);
    gpioInitialise();

    //config pins
    gpioSetMode(THREAD_CONTROL, PI_OUTPUT);
    gpioSetMode(THREAD_IN, PI_INPUT);
    gpioSetMode(THREAD_OUT, PI_OUTPUT);
    
    //initialize output pin;
    gpioWrite(THREAD_CONTROL, 1);
    gpioWrite(THREAD_OUT,1);

    //initialize poller
    pthread_t poller_thread_id;
    sos_poller_t* poller = sosPollerInit(THREAD_IN, 0);

    pthread_create(&poller_thread_id, NULL, &sosPoller, poller);

    while(getch() != 'q')
    {
        printf("looping...");
    } 
    
    poller->exit_flag = 1;
    pthread_join(poller_thread_id, NULL);
    gpioTerminate();
    echo();
    endwin();
}