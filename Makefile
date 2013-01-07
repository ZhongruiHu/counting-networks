HEADERS = balancer.h bitonic.h
SRCFILES = balancer.c bitonic.c main.c
OBJFILES = $(SRCFILES:.c=.o)

CFLAGS = -Wall -g
LDFLAGS = -lpthread

all: network

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

network: $(OBJFILES) 
	$(CC) -o network $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o network
