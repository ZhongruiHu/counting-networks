HEADERS = balancer.h bitonic.h network.h
SRCFILES = balancer.c bitonic.c network.c
OBJFILES = $(SRCFILES:.c=.o)

CFLAGS = -Wall -Werror -g -O2
LDFLAGS = -lpthread

all: network

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

network: main.c $(OBJFILES) 
	$(CC) -o network $^ $(LDFLAGS)

testprog: test.c $(OBJFILES)
	$(CC) -o testprog $^ $(LDFLAGS)

.PHONY: test
test: testprog
	./testprog

.PHONY: clean
clean:
	rm -f *.o network testprog
