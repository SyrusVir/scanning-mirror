#define _GNU_SOURCE
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h> 
#include <ncurses.h>
#include "sos_poller.c"
#include <sched.h>
#include <time.h>

#define THREAD_IN 23        //physical pin 16; pin that the threaded poller will poll
#define THREAD_OUT 24       //physical pin 18; This pin will be pulled LO when THREAD_IN is polled as LO
#define THREAD_CONTROL 25   //physical pin 22 output pin wired to THREAD_IN

#define DELAY_USEC 1000 //microseconds after THREAD_IN or ALERT_IN is triggered to reset *_OUT pin to HI

#define AUTO_TRIG_MSEC 200
//Function to simulate mutex operation in main. Pulse GPIO if mutex obtained
void* threadOutFunc(void* arg_lock)
{
    pthread_spinlock_t* lock = (pthread_spinlock_t*)arg_lock;
    printf("threadOUt ID=%ld\tthreadOutlock=%p\n\r",pthread_self(),lock);
    while(1)
    {
        uint8_t x = 1;
        while (pthread_spin_trylock(lock) == 0)
        {
            pthread_spin_unlock(lock);    
            if (x--) 
            {
                gpioWrite(THREAD_OUT, 0);
            }
        }
        gpioWrite(THREAD_OUT,1);
    }
}

void timerCallback()
{
    gpioTrigger(THREAD_CONTROL,2,0);
}

int main() 
{
    //Pigpiio intialization
    gpioCfgClock(1,1,0);
    gpioInitialise();

    //config pins
    gpioSetMode(THREAD_CONTROL, PI_OUTPUT);
    gpioSetMode(THREAD_IN, PI_INPUT);
    gpioSetMode(THREAD_OUT, PI_OUTPUT);
    
    //initialize output pin;
    gpioWrite(THREAD_CONTROL, 1);
    gpioWrite(THREAD_OUT,0);

    //initialize poller
    pthread_spinlock_t lock;
    pthread_spin_init(&lock, 0);
    sos_poller_t* poller = sosPollerInit(&lock, THREAD_IN, 0, DELAY_USEC);
    
    //configuring threads
    pthread_t poller_thread_id;
    pthread_t pulse_thread_id;
    pthread_attr_t attr;
    clockid_t cid;

    pthread_attr_init(&attr);

    //set thread affinities
    cpu_set_t cpu_mask;
    
    CPU_ZERO(&cpu_mask);
    CPU_SET(1, &cpu_mask);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_mask), &cpu_mask);
    pthread_create(&poller_thread_id, &attr, &sosPoller, poller);
    pthread_getcpuclockid(poller_thread_id, &cid);
    printf("poller CID = %lX:\n\r", cid);

    CPU_ZERO(&cpu_mask);
    CPU_SET(2, &cpu_mask);
    pthread_attr_setaffinity_np(&attr,sizeof(cpu_mask), &cpu_mask);
    pthread_create(&pulse_thread_id, &attr, &threadOutFunc, (void*)&lock);
    pthread_getcpuclockid(pulse_thread_id, &cid);
    printf("pulse CID = %lX:\n\r", cid);

    CPU_ZERO(&cpu_mask);
    CPU_SET(0, &cpu_mask);
    pthread_setaffinity_np(pthread_self(),sizeof(cpu_mask), &cpu_mask);
    printf("main CID = %lX:\n\r", cid);
    printf("main TID = %ld\n\r", pthread_self());

    //ncurses init
    /* initscr();
    nodelay(stdscr,TRUE);
    noecho(); */
    gpioSetTimerFunc(1, AUTO_TRIG_MSEC, timerCallback);
    char c;// = getch();
    do
    {
        scanf("%c",&c);// = getch();
        /* if (c != ERR && c != 'q') 
        {
            printf("trigger_tick=%lu\n\r",gpioTick());
            gpioTrigger(THREAD_CONTROL,5,0);
        } */
    } while (c != 'q');

    printf("exiting\n\r");
    poller->exit_flag = 1;
    pthread_join(poller_thread_id, NULL);
    pthread_cancel(pulse_thread_id);
    sosPollerDestroy(poller);
    pthread_spin_destroy(&lock);
    gpioTerminate();
    nodelay(stdscr, FALSE);
    endwin();
}