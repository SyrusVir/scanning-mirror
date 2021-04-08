CC = gcc
CFLAGS = -g -O -Winline -Wall -Wextra
LFLAGS = -lpigpio -pthread

.PHONY: obj
obj: scanmirror.o pinpoller.o

scanmirror.o: scanmirror.c scanmirror.h
	$(CC) $(CFLAGS) -c $< -I. $(LFLAGS)

pinpoller.o: pinpoller.c pinpoller.h
	$(CC) $(CFLAGS) -c $< -I. $(LFLAGS)

poller_hw_test: scanmirror.c pinpoller.c
	gcc -O $@.c $^ -o $@.out -Winline -I. -lpigpio -lncurses -pthread

libscanmirror: scanmirror.o
	ar -r -c $@.a scanmirror.o 

poller_test:
	gcc $@.c -o $@.out -lpigpio -pthread -lncurses

pwm:
	gcc pwm.c -o pwm -lpigpio -lcurses 

.PHONY: clean
clean: 
	rm -f *.o *.a

cleanpwm:
	rm pwm