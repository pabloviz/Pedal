
#openusb.o: openusb.c openusb.h
#	gcc openusb.c -o openusb.o

main.o: main.c notes.h effects.c effects.h
	gcc  main.c effects.c -o main.o -lasound -lm -lusb-1.0
clean:
	rm main.o
