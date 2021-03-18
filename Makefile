poller_hw_test: scanmirror.c pinpoller.c
	gcc -O $@.c $^ -o $@.out -Winline -I. -lpigpio -lncurses -pthread

libscanmirror: scanmirror.o
	ar -r -c $@.a scanmirror.o 

poller_test:
	gcc $@.c -o $@.out -lpigpio -pthread -lncurses

scanmirror.o: scanmirror.c scanmirror.h
	gcc -c scanmirror.c -I . -lpigpio

pwm:
	gcc pwm.c -o pwm -lpigpio -lcurses 

clean: 
	rm *.o *.a

cleanpwm:
	rm pwm