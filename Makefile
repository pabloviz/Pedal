

main.o: main.c
	gcc  main.c -o main.o -lasound -lm
clean:
	rm edit main.o
