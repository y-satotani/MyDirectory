CC=gcc
CFLAGS=-Wall -Og
OBJ=mystring.o buffer.o profile.o date.o

.PHONY : all clean

all: standalone client server

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)

standalone: standalone.o $(OBJ)
	$(CC) -o standalone.out $^ $(CFLAGS)

client: client.o $(OBJ)
	$(CC) -o client.out $^ $(CFLAGS)

server: server.o $(OBJ)
	$(CC) -o server.out $^ $(CFLAGS)

clean:
	rm *.o *.out
