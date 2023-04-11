TARGET=main
OBJS=main.o GPIB_prof.o serial.o selector.o file.o commands.o
CFLAGS=-static -Wall -pthread -lpthread -g

all: ${TARGET}



${TARGET}: ${OBJS}
	${CC} -o ${TARGET}.exe ${OBJS}

clean:
	rm -f ${TARGET} ${OBJS} core*
