#define _GNU_SOURCE
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h> 
#include <ncurses.h>
#include "pin_poller.c"
#include <sched.h>
#include <time.h>

#define THREAD_IN 23        //physical pin 16; pin that the threaded poller will poll
#define THREAD_OUT 24       //physical pin 18; This pin will be pulsed HI when THREAD_IN is polled as LO
#define THREAD_CONTROL 25   //physical pin 22 output pin wired to THREAD_IN

#define DELAY_USEC 50 //microseconds after THREAD_IN or ALERT_IN is triggered to reset *_OUT pin to HI

#define AUTO_TRIG_MSEC 0 //time between pulses the poller reads

//Function to simulate mutex operation in main. Pulse GPIO if mutex obtained
void* pollFeedback(void* arg_lock)
{
    //obtain the spinlock mutex from argument
    pthread_spinlock_t* lock = (pthread_spinlock_t*)arg_lock;

    /* main loop constantly try to lock the spinlock [lock];
     * If successful, immediately unlock and write a LO output only once.
     * If the lock fails, the poller has been triggered. Write a HI output.
     * The width of the resulting pulse should be close to DELAY_USEC
     */
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

//Callback function that emits a pulse on THREAD_CONTROL
void timerCallback()
{
    gpioTrigger(THREAD_CONTROL,2,0);
}

int main() 
{
    //Pigpiio intialization
    gpioCfgClock(1,1,0);    //configure for 1ms sample rate with PCM clock
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
    pin_poller_t* poller = pinPollerInit(&lock, THREAD_IN, 0, DELAY_USEC);
    
    //configuring threads
    pthread_t poller_thread_id;
    pthread_t pollfb_thread_id;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    //set thread cpu affinities////////////////////
    cpu_set_t cpu_mask;
    
    //assign poller to CPU 1
    CPU_ZERO(&cpu_mask);
    CPU_SET(1, &cpu_mask);
    pthread_attr_setaffinity_np(&attr, sizeof(cpu_mask), &cpu_mask);
    pthread_create(&poller_thread_id, &attr, &pinPollerMain, poller);

    //assign poller feedback to CPU 2
    CPU_ZERO(&cpu_mask);
    CPU_SET(2, &cpu_mask);
    pthread_attr_setaffinity_np(&attr,sizeof(cpu_mask), &cpu_mask);
    pthread_create(&pollfb_thread_id, &attr, &pollFeedback, (void*)&lock);

    //assign main thread to cpu 0
    CPU_ZERO(&cpu_mask);
    CPU_SET(0, &cpu_mask);
    pthread_setaffinity_np(pthread_self(),sizeof(cpu_mask), &cpu_mask);
    
    //configure timed function callback
    if (AUTO_TRIG_MSEC != 0)
    {
        gpioSetTimerFunc(1, AUTO_TRIG_MSEC, timerCallback);
    }
    
    //wait for user exit command 'q'
    char c;
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
    pthread_cancel(pollfb_thread_id);
    pinPollerDestroy(poller);
    pthread_spin_destroy(&lock);
    gpioTerminate();
    nodelay(stdscr, FALSE);
    endwin();
}