#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include "pinpoller.h"

#define POLL_PIN 13         // physical pin 33
#define POLL_DELAY_USEC 50  

#define PULSE_PIN 5        // physical pin 29
#define PULSE_FREQ 1000     // pulse frequencies
#define PULSE_WIDTH_USEC 10  // width of logical HI pulse
#define PULSE_COUNT (uint64_t)15E6     // number of pulses to emit
#define PULSE_POLARITY 1    // inverted pulse logic if zero

int main() 
{
    uint64_t drop_cnt = 0;

    gpioCfgClock(1,1,0);
    gpioInitialise();

    gpioSetMode(PULSE_PIN, PI_OUTPUT);
    printf("GPIO configed\n");

    pthread_t poll_tid;
    cpu_set_t cpu_mask;
    pthread_spinlock_t lock;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_spin_init(&lock, 0);
    pin_poller_t* poller = pinPollerInit(&lock, POLL_PIN, PULSE_POLARITY, POLL_DELAY_USEC);
    
    CPU_ZERO(&cpu_mask);
    CPU_SET(2, &cpu_mask);
    printf("here\n");
    if (pthread_attr_setaffinity_np(&attr, sizeof(cpu_mask), &cpu_mask) != 0)
    {
        perror("pthread_sestaffinity_np");
        return;
    }
    pthread_create(&poll_tid, &attr, &pinPollerMain, poller);
    printf("poller started\n");

    CPU_ZERO(&cpu_mask);
    CPU_SET(1, &cpu_mask);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_mask), &cpu_mask);
    printf("parent thread affinity set\n");

    printf("loop started\n");
    uint64_t i;
    for (i = 0; i < PULSE_COUNT; i++)
    {
        gpioTrigger(PULSE_PIN,PULSE_WIDTH_USEC,PULSE_POLARITY);
        if (pinPollerCheckIn(poller) == 0) drop_cnt++;
        gpioDelay(POLL_DELAY_USEC);
    }
    printf("%llu\n",i);
    printf("Dropped %llu out of %llu\n", drop_cnt, PULSE_COUNT);
    pinPollerExit(poller);
    pthread_join(poll_tid, NULL);
    pinPollerDestroy(poller);
    gpioTerminate();
}