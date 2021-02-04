#include <pigpio.h>
#include <stdint.h>

typedef struct Mirror {
    uint8_t FREQ_PIN;
    int ENABLE_PIN;
    int ATSPEED_PIN;
} mirror_t;

int configMirror(mirror_t mirror);
int enableMirror(mirror_t mirror);
int disableMirror(mirror_t mirror);
int setMirrorRPM(mirror_t mirror, uint16_t rpm);
int checkMirrorAtSpeed(mirror_t mirror);
