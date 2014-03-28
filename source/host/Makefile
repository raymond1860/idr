
#uncomment below line to enable debug
#CFLAGS+= -g
CC=gcc

LDIR =../lib

LIBS=-ldl -lpthread

DEPS = IdentityCard.h platform.h option.h

OBJS = option.o \
	platform.o \
	platform_linux.o \
	IdentityCard.o \
	packet.o \
	hex2bin.o \
	transfer.o \
	download.o \
	main.o


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: idr
idr: $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ core *~ 
