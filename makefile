CC = gcc
	
all: macD

macD: macD.c macDfunctions.c
	$(CC) -g -fsanitize=leak -fsanitize=address -o macD macD.c macDfunctions.c -Wall

clean:
	rm -f macD
