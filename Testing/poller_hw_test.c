#define _GNU_SOURCE
#include <pigpio.h> 
#include <pthread.h>
#include <ncurses.h>
#include <errno.h>
#include "scanmirror.h"
#include <stdio.h>
#include <sched.h>
#include "pinpoller.h"

//Mirror and SOS pins
#define MIRROR_PWM_PIN 18           // physical pin 12; mirror control PWM output
// #define MIRROR_SPEED_PIN 23         // physical pin 16; mirror ATSPEED input
#define MIRROR_SOS_PIN 7           // physical pin 26; sos detector input
// #define MIRROR_ENABLE_PIN 25        // physical pin 22; mirror ENABLE output
#define MIRROR_SOS_DELAY_USEC 5000    // usecs to delay once SOS signal detected by poller
#define MIRROR_RPM 4000

//laser pins
#define LASER_ENABLE_PIN 26      // physical pin 37
#define LASER_SHUTTER_PIN 6      // physical pin 31
#define LASER_PULSE_PIN 23       // physical pin 16
#define LASER_PULSE_FREQ 9000    
#define LASER_PULSE_PERIOD_USEC 1000000/LASER_PULSE_FREQ   
#define LASER_PULSE_WIDTH_USEC 10
#define LASER_PULSE_DUTY (unsigned int)(LASER_PULSE_WIDTH_USEC / 1000000.0 * LASER_PULSE_FREQ * PI_HW_PWM_RANGE)

typedef enum state
{
    WAIT,
    START,
    RUN,
    STOP
} test_state_t;


//prep laser for emission
void enableLaser()
{
    gpioWrite(LASER_SHUTTER_PIN,1);
    gpioDelay(500*1000);

    gpioWrite(LASER_ENABLE_PIN,0);

    gpioSetMode(LASER_PULSE_PIN, PI_OUTPUT);
    // gpioHardwarePWM(LASER_PULSE_PIN, LASER_PULSE_FREQ, LASER_PULSE_DUTY);
    return;
}

void disableLaser()
{
    gpioWrite(LASER_SHUTTER_PIN,0);
    gpioWrite(LASER_ENABLE_PIN,0);
    gpioHardwarePWM(LASER_PULSE_PIN, 0, 0);

}

int main() 
{
    // Pigpio Initialize
    gpioCfgClock(1,1,0);    // 1 us sample rate; using PCM clock for hardware PWM
    if (gpioInitialise() < 0) 
    {
        perror("gpioInitialise()");
        return -1;
    }
    //ncurses initialization
    initscr();
    nodelay(stdscr, true);
    noecho();

    //laser pin configuration/////////////////////////////////
    gpioSetMode(LASER_SHUTTER_PIN,PI_OUTPUT);
    gpioWrite(LASER_SHUTTER_PIN,0); // start with laser shuttered
    
    gpioSetMode(LASER_ENABLE_PIN,PI_OUTPUT);
    gpioWrite(LASER_ENABLE_PIN, 0); // start with laser disabled    
    ////////////////////////////////////////////////////////////

    // mirror initialization /////////////////////////////////////////////////////
    mirror_t mirror = {
        // .ATSPEED_PIN = MIRROR_SPEED_PIN;
        .FREQ_PIN = MIRROR_PWM_PIN
        // .ENABLE_PIN = MIRROR_ENABLE_PIN;
    };

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
    gpioDelay(10); //delay to diagnose SOS triggering at code beginning
    
    pthread_t sos_poller_tid;   //thread ID of the sos polling thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    //assign poller to isolated core 2 and start////////////////////////////////////////////
    cpu_set_t cpu_mask;
    CPU_ZERO(&cpu_mask);
    CPU_SET(2,&cpu_mask);
    pthread_setaffinity_np(sos_poller_tid, sizeof(cpu_mask), &cpu_mask);
    pthread_create(&sos_poller_tid, &attr, &pinPollerMain, sos_poller);
    ////////////////////////////////////////////////////////////////////////////////////////////////
    
    //assign main thread to isolated core 1////////////////////////////////////////////
    CPU_ZERO(&cpu_mask);
    CPU_SET(1,&cpu_mask);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_mask), &cpu_mask);
    ////////////////////////////////////////////////////////////////////////////////////////////////
    
    /**** main test code ****/
    test_state_t state = WAIT;
    int c;
    while (state != STOP)
    {
        switch (state)
        {
            case WAIT:
                printw("\'q\' to quit.\n\r\'S\' to start.\n\r");
                do
                {
                    c = getch();
                } while (c != 'q' && c != 'S');
                
                state = (c == 'q') ? STOP : START;

                break;
            case START:
                //prep laser for emission
                enableLaser();

                //start with mirror startup
                mirrorSetRPM(mirror, MIRROR_RPM); //start rotation
                // mirrorEnable(mirror);       //enable mirror

                // printf("Waiting for mirror at speed...\n\r");
                // if (MIRROR_RPM != 0) 
                // {
                //     while (!mirrorCheckAtSpeed(mirror));    //wait until at speed
                // }

                //Do not start laser until after first occurance of SOS detection
                printf("Waiting for poller event...\n\r");
                while (pinPollerCheckIn(sos_poller) == 0); //wait until first SOS detection
                printf("Waiting for first lock...\n\r");
                while (pinPollerCheckIn(sos_poller) != 1); //wait until lock was found available
                printf("STARTING...\n\r");
                gpioWrite(LASER_ENABLE_PIN, 1); //start laser emission
                
                c = getch();
                while (c != 'q')
                {
                    //uint8_t x = 1;
                    //wait until poller releases lock
                    while (/* pthread_spin_trylock(&sos_spinlock) == 0 */pinPollerCheckIn(sos_poller) !=  1)
                    {  
                        // pthread_spin_unlock(&sos_spinlock);
                        //While the poller lock is free, keep laser enabled
                        // if (x--)
                        // {
                        //     gpioWrite(LASER_ENABLE_PIN, 0);
                        // }
                    gpioWrite(LASER_PULSE_PIN,1);
                    gpioDelay(500);    
                    gpioWrite(LASER_PULSE_PIN,0);
                    gpioDelay(500);    
                    }
                    //else disable laser
                    
                    c = getch();
                }
                printf("STOPPING\r");
                state = STOP;
                break;
            default:
                break;
        } //end switch (state)
    } //end while (state != STOP)
    //disable Laser
    disableLaser();
    printf("laser disabled\n\r");

    //stop poller
    pinPollerExit(sos_poller);
    pthread_join(sos_poller_tid, NULL);
    printf("joined thread\n\r");
    pinPollerDestroy(sos_poller);
    pthread_spin_destroy(&sos_spinlock);
    printf("poller destroyed\n\r");
    
    //stop mirror
    mirrorSetRPM(mirror, 0);
    // mirrorDisable(mirror);
    printf("mirror stopped\n\r");


    gpioTerminate();
    nodelay(stdscr, false);
    endwin();
    return 0;
} //end main