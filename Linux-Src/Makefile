CC=gcc
TARGET=main
OBJS=main.o GPIB_prof.o serial.o selector.o file.o commands.o
CFLAGS=-g -pthread
LFLAGS=-static -g -lpthread

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${LFLAGS} -o $@ $^

main.o: main.c
GPIB_prof.o: GPIB_prof.c
serial.o: serial.c
selector.o: selector.c
file.o: file.c
commands.o: commands.c

.PHONY : clean

clean:
	rm -f ${TARGET} core*
	rm -f *.o core*
