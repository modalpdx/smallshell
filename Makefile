CC = gcc
CFLAGS = -g -Wall -Werror
BIN = smallsh

all: smallsh

default: smallsh

smallsh: smallsh_func.o main.o
	$(CC) $(CFLAGS) -o $(BIN) smallsh_func.o main.o 

smallsh_func.o:
	$(CC) $(CFLAGS) -c smallsh_func.c

main.o: 
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f *.o $(BIN)

