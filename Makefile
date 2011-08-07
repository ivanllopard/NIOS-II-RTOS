CC=gcc
CFLAGS=-Wall
SRCS=main.c list.c user.c kernel.c exception.c 
OBJS=$(addsuffix .o, $(basename ${SRCS}))
TARGET=prog

default: $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

%.o: %.s
	$(CC) $(CFLAGS) -o $*.o

%.o: %.c
	$(CC) $(CFLAGS) -c $?

clean:
	rm -f *.o $(TARGET)