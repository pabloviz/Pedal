

main.o: main.c fft.c fft.h notes.h
	gcc  main.c fft.c -o main.o -lasound -lm
clean:
	rm edit main.o
