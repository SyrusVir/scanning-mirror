#include <pigpio.h>
#include <stdint.h>

typedef struct Mirror {
    uint8_t FREQ_PIN;
    int ENABLE_PIN;
    int ATSPEED_PIN;
} mirror_t;

int mirrorConfig(mirror_t m);
int mirrorEnable(mirror_t m);
int mirrorDisable(mirror_t m);
int mirrorSetRPM(mirror_t m, uint16_t rpm);
int mirrorCheckAtSpeed(mirror_t m);

