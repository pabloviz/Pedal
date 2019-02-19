
#openusb.o: openusb.c openusb.h
#	gcc openusb.c -o openusb.o

main.o: main.c notes.h effects.c effects.h utils.c utils.h defines.h lookuptable.h lookuptable.c \
	gpioControl.h gpioControl.c
	gcc  main.c effects.c utils.c lookuptable.c gpioControl.c -o main.o -lasound -lm -lusb-1.0 -fopenmp
clean:
	rm main.o
	rm .*.c.swp
	rm .*.swp
