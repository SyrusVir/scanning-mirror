#include <pigpio.h>
#include <stdint.h>

unsigned int const MIRROR_FULL_DUTY = 1000000;
typedef struct Mirror {
    uint8_t FREQ_PIN;
    uint8_t ENABLE_PIN;
    uint8_t ATSPEED_PIN;
} mirror_t;

uint8_t enableMirror(mirror_t);
uint8_t disableMirror(mirror_t);
uint8_t setMirrorRPM(mirror_t, uint16_t);
uint8_t checkMirrorAtSpeed(mirror_t);
