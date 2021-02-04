LIBFLAGS= -lpigpio -lcurses

scanmirror: scanmirror.o
	ar -r -c libscanmirror.a scanmirror.o 

scanmirror.o: scanmirror.c scanmirror.h
	gcc -c scanmirror.c -I . -lpigpio

pwm:
	gcc pwm.c -o pwm $(LIBFLAGS)

clean: 
	rm *.o *.a

cleanpwm:
	rm pwm