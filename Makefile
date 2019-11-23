OBJECTS1 = header.o part1.o
OBJECTS2 = header.o part2.o
OBJECTS3 = header.o part3.o
OBJECTS4 = header.o part4.o

CFLAGS = -W -Wall -g
CC = gcc
PROG_NAME = part
ARGS = -ts=3 input.txt

all: part1 part2 part3 part4 cpu io

part1: $(OBJECTS1)
	$(CC) $(OBJECTS1) $(CFLAGS) -o $(PROG_NAME)1

part2: $(OBJECTS2)
	$(CC) $(OBJECTS2) $(CFLAGS) -o $(PROG_NAME)2

part3: $(OBJECTS3)
	$(CC) $(OBJECTS3) $(CFLAGS) -o $(PROG_NAME)3

part4: $(OBJECTS4)
	$(CC) $(OBJECTS4) $(CFLAGS) -o $(PROG_NAME)4

cpu: cpubound.o
	$(CC) cpubound.o -o cpubound

io: iobound.o
	$(CC) iobound.o -o iobound

header.o: header.c header.h
part1.o: part1.c
part2.o: part2.c
part3.o: part3.c
part4.o: part4.c
cpubound.o: cpubound.c
iobound.o: iobound.c

clean:
	rm part1
	rm part2
	rm part3
	rm part4
	rm *.o
	rm cpubound
	rm iobound
