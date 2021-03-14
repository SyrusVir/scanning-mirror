#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

typedef struct SOSPoller
{
    uint8_t exit_flag;  //raise flag to exit poller and poller thread
    uint8_t trigger_level;  //1 or 0; GPIO level poller looks for
    uint8_t sos_detector_pin;   //RPi pin to poll for [trigger_level]
    pthread_mutex_t laser_mutex;    //mutex to lock to indicate [trigger_level] was detected at pin [sos_detector_pin]
    uint32_t delay_usec;    // after delay_usec, laser_mutex is unlocked 
} sos_poller_t;

sos_poller_t* sosPollerInit(int sos_detector_pin, int trigger_level)
{
    sos_poller_t* poller = malloc(sizeof(sos_poller_t));
    poller->sos_detector_pin = sos_detector_pin;
    poller->exit_flag = 0;
    poller->trigger_level = (trigger_level ? 1 : 0); //return 1 if [trigger_level] non_zero, 0 otherwise
    pthread_mutex_init(&poller->laser_mutex,NULL);
    return poller;
} //end sosPollerInit()

int sosPollerDestroy(sos_poller_t* poller)
{
    if (pthread_mutex_destroy(&poller->laser_mutex) == EBUSY) return EBUSY; //return with error if attempting to destroy a mutex locked elsewhere
    free(poller);

    return 0;
} //end sosPollerDestroy

void* sosPoller(void* sos_poller_arg)
{
    sos_poller_t* poller = (sos_poller_t*) sos_poller_arg;

    //local copies of variables for quicker access within main loop
    uint8_t pin = poller->sos_detector_pin;
    uint8_t trigger_level = poller->trigger_level; 
    uint32_t delay_usec = poller->delay_usec;
    pthread_mutex_t* lock = &poller->laser_mutex; 

    //main polling loop
    while(!poller->exit_flag)
    {
        while (gpioRead(pin) != trigger_level) {} //poll until [trigger_level]
        pthread_mutex_lock(lock);   //lock mutex, stopping emissioon
        gpioDelay(delay_usec);              //delay for specified microseconds
        pthread_mutex_unlock(lock); //unlock mutex, allowing emission
    } //end while(!poller->exit_flag)

    return NULL;
}