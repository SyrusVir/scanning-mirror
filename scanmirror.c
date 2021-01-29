#include <scanmirror.h>

uint8_t enableMirror(mirror_t m) {
    return gpioWrite(m.ENABLE_PIN,0);
}

uint8_t diableMirror(mirror_t m) {
    return gpioWrite(m.ENABLE_PIN,1);
}

uint8_t setMirrorRPM(mirror_t m, uint16_t rpm) {
    if (rpm < 1000 || rpm > 4000) {
        printf("%u RPM provided. GECKO-FOUR mirror supports RPM's in range 1000 to 4000.\n", rpm);
        if (rpm < 1000) {
            rpm = 1000;
            printf("RPM set to %u RPM\n",rpm);
        }
        else {
            rpm = 4000;
            printf("RPM set to %u RPM\n",rpm);
        }
    }
    unsigned int freq = rpm/60*24; //equation provided in mirror docs
    return gpioHardwarePWM(m.FREQ_PIN,freq,(unsigned)MIRROR_FULL_DUTY/2);
}

uint8_t checkMirrorAtSpeed(mirror_t m) {
    /**Returns 1 if mirror is at desired speed and 0 if not. **/
    return 1 - gpioRead(m.ATSPEED_PIN); //1 - because atspeed pin is LOW when at speed
}