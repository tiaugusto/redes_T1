CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
SRCDIR  = ../../src

all: test_integration

test_integration: int_ext.o $(SRCDIR)/protocol.o $(SRCDIR)/net.o
	$(CC) $(CFLAGS) -o $@ $(SRCDIR)/protocol.o $(SRCDIR)/net.o int_ext.o

int_ext.o: test_integration.c $(SRCDIR)/protocol.h $(SRCDIR)/net.h
	$(CC) $(CFLAGS) -c test_integration.c -o int_ext.o

clean:
	rm -f int_ext.o test_integration
