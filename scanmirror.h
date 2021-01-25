#include<pigpio.h>

int startMirror(int pin,int freq);
int stopMirror(int pin);
int enableMirror(int pin);
int disableMirror(int pin);
int checkMirrorSpeed(int pin);
