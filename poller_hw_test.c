#define _GNU_SOURCE
#include <pthread.h>
#include <errno.h>
#include <scanmirror.h>
#include <stdio.h>
#include <sched.h>
#include <pigpio.h> 
#include "pin_poller.c"

//Mirror and SOS pins
#define MIRROR_SOS_PIN 17           // physical pin 11
#define MIRROR_SOS_DELAY_USEC 50    // usecs to delay once SOS signal detected by poller
#define MIRROR_SPEED_PIN 23         // physical pin 16
#define MIRROR_ENABLE_PIN 5         // physical pin 29
#define MIRROR_PWM_PIN 18           // physical pin 12

//laser pins
#define LASER_ENABLE_PIN
#define LASER_SHUTTER_PIN
#define LASER_PULSE_PIN
#define LASER_FREQ

int main() 
{
    // Pigpio Initialize
    gpioCfgClock(1,1,0);    // 1us sample rate; using PCM clock for hardware PWM
    gpioInitialise();

    // mirror initialization /////////////////////////////////////////////////////
    mirror_t mirror;
    mirror.ATSPEED_PIN = MIRROR_SPEED_PIN;
    mirror.FREQ_PIN = MIRROR_PWM_PIN;
    mirror.ENABLE_PIN = MIRROR_ENABLE_PIN;

    if (mirrorConfig(mirror) != 0)
    {
        perror("mirrorConfig()");
        return -1;
    }
    //////////////////////////////////////////////////////////////////////////////

    // Initializing SOS pin poller//////////////////////////////////////////////////////////////////
    pthread_spinlock_t sos_spinlock;
    pthread_spin_init(&sos_spinlock, 0);
    pin_poller_t* sos_poller = pinPollerInit(&sos_spinlock, MIRROR_SOS_PIN, 0, MIRROR_SOS_DELAY_USEC); // configure pin poller
    
    pthread_t sos_poller_tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    CPU_SET(2,&cpu_mask);
    pthread_setaffinity_np();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    return 0;
}