
CC=gcc

LDIR =../lib

LIBS=-ldl -lpthread

DEPS = IdentityCard.h

OBJS = main.o IdentityCard.o 


%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: idr
idr: $(OBJS)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o *~ core *~ 
