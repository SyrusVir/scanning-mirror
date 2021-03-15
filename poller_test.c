#define _GNU_SOURCE
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h> 
#include <ncurses.h>
#include "sos_poller.c"
#include <sched.h>

#define THREAD_IN 23        //physical pin 16; pin that the threaded poller will poll
#define THREAD_OUT 24       //physical pin 18; This pin will be pulled LO when THREAD_IN is polled as LO
#define THREAD_CONTROL 25   //physical pin 22 output pin wired to THREAD_IN

#define DELAY_USEC 10000 //microseconds after THREAD_IN or ALERT_IN is triggered to reset *_OUT pin to HI

//Function to simulate mutex operation in main. Pulse GPIO if mutex obtained
void* threadOutFunc(void* arg_lock)
{
    pthread_mutex_t* lock = (pthread_mutex_t*)arg_lock;
    printf("threadOUt ID=%lX\tthreadOutlock=%p\n\r",pthread_self(),lock);
    while(1)
    {
        if (pthread_mutex_trylock(lock) != 0)
        {
            pthread_mutex_unlock(lock);
            gpioWrite(THREAD_OUT,1);
        }
        else gpioWrite(THREAD_OUT,0);
    }
}

int main() 
{
    //ncurses init
    initscr();
    nodelay(stdscr,TRUE);
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
    gpioWrite(THREAD_OUT,0);

    //initialize poller
    pthread_t poller_thread_id;
    pthread_t pulse_thread_id;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);

    sos_poller_t* poller = sosPollerInit(&lock, THREAD_IN, 0, DELAY_USEC);

    //set thread affinities
    cpu_set_t cpu_mask;
    
    CPU_ZERO(&cpu_mask);
    CPU_SET(1, &cpu_mask);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_mask), &cpu_mask);
    printf("here\n\r");
    pthread_create(&poller_thread_id, &attr, &sosPoller, poller);

    CPU_ZERO(&cpu_mask);
    CPU_SET(2, &cpu_mask);
    if (pthread_attr_setaffinity_np(&attr,sizeof(cpu_mask), &cpu_mask) != 0)
    {
        perror("pulse");
    }
    pthread_create(&pulse_thread_id, &attr, &threadOutFunc, &lock);

    CPU_ZERO(&cpu_mask);
    CPU_SET(0, &cpu_mask);
    if(pthread_setaffinity_np(pthread_self(),sizeof(cpu_mask), &cpu_mask) != 0)
    {
        perror("main");
    }



    int c = getch();
    while(c != 'q')
    {
        c = getch();
        if (c != ERR && c != 'q') 
        {
            printf("trigger_tick=%ld\n\r",gpioTick());
            gpioTrigger(THREAD_CONTROL,2,0);
        }
        
        /* if(pthread_mutex_trylock(&poller->laser_mutex) == 0)
        {
            printf("locked...\n\r");
            pthread_mutex_unlock(&poller->laser_mutex);
        } 
        else printf("lock failed...\n\r"); */
    } 

    printf("exiting\n\r");
    poller->exit_flag = 1;
    pthread_join(poller_thread_id, NULL);
    pthread_cancel(pulse_thread_id);
    sosPollerDestroy(poller);
    pthread_mutex_destroy(&lock);
    gpioTerminate();
    nodelay(stdscr, FALSE);
    endwin();
}