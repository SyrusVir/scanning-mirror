#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

typedef struct SOSPoller
{
    uint8_t exit_flag;
    int sos_detector_pin;
    uint32_t delay_usec;
    pthread_mutex_t laser_mutex;
} sos_poller_t;

sos_poller_t* sosPollerInit(int sos_detector_pin)
{
    sos_poller_t* poller = malloc(sizeof(sos_poller_t));
    poller->sos_detector_pin = sos_detector_pin;
    poller->exit_flag = 0;
    pthread_mutex_init(&poller->laser_mutex,NULL);
    return poller;
} //end sosPollerInit()

int sosPollerDestroy(sos_poller_t* poller)
{
    if (pthread_mutex_destroy(&poller->laser_mutex) == EBUSY) return EBUSY;
    free(poller);
} //end sosPollerDestroy

void* sosPoller(void* sos_poller_arg)
{
    sos_poller_t* poller = (sos_poller_t*) sos_poller_arg;
    int pin = poller->sos_detector_pin; //local copy for quicker acccess within main loop
    
    //main polling loop
    while(!poller->exit_flag)
    {
        while (gpioRead(pin)) {} //poll until LO
        pthread_mutex_lock(&poller->laser_mutex);   //lock mutex, stopping emissioon
        gpioDelay(poller->delay_usec);              //delay for specified microseconds
        pthread_mutex_unlock(&poller->laser_mutex); //unlock mutex, allowing emission
    } //end while(!poller->exit_flag)

    return NULL;
}