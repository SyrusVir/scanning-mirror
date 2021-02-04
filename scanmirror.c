#include <scanmirror.h>
#include <stdio.h>

unsigned int const MIRROR_FULL_DUTY = 1000000;

int configMirror(mirror_t m) {
    char err_msg[] = "configMirror: PIGPIO EXCEPTION %d WITH %s\n";
    int status;
    status = gpioSetMode(m.ENABLE_PIN, PI_OUTPUT);
    if (status < 0) {
        printf(err_msg, status, "gpioSetMode(m.ENABLE_PIN)");
        return status;
    }

    status = gpioSetMode(m.ATSPEED_PIN, PI_INPUT);
    if (status < 0) {
        printf(err_msg, status, "gpioSetMode(m.ATSPEED_PIN)");
        return status;
    }
    
    status = gpioSetPullUpDown(m.ATSPEED_PIN, PI_PUD_UP);
    if (status < 0) {
        printf(err_msg, status, "gpioSetPullUpDown(m.ATSPEED_PIN, PI_PUD_UP)");
        return status;
    }

    status = gpioSetMode(m.FREQ_PIN, PI_ALT5);
    if (status < 0) {
        printf(err_msg, status, "gpioSetMode(m.FREQ_PIN, PI_ALT5)");
        return status;
    }

    return 0;
}

int enableMirror(mirror_t m) {
    return gpioWrite(m.ENABLE_PIN,0);
}

int diableMirror(mirror_t m) {
    return gpioWrite(m.ENABLE_PIN,1);
}

int setMirrorRPM(mirror_t m, uint16_t rpm) {
    if (rpm > 4000) {
        printf("%u RPM provided. GECKO-FOUR mirror recommends RPM's in range 1000 to 4000.\n", rpm);
        rpm = 4000;
        printf("RPM set to %u RPM\n",rpm);
        }
    unsigned int freq = rpm / 60 * 24; //equation provided in mirror docs
    return gpioHardwarePWM(m.FREQ_PIN,freq,(unsigned)MIRROR_FULL_DUTY/2);
}

int checkMirrorAtSpeed(mirror_t m) {
    /**Returns 1 if mirror is at desired speed and 0 if not. **/
    int status;
    status = gpioRead(m.ATSPEED_PIN);
    if (status >= 0) {
        return (1 - gpioRead(m.ATSPEED_PIN)); //atspeed pin is LOW when at speed
    }
    else return status;
}