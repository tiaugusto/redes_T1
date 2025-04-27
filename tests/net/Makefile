CC      = gcc
INCDIR  = $(SRCDIR)
CFLAGS  = -std=c11 -Wall -Wextra -O2 -I$(INCDIR)
SRCDIR  = ../../src

all: test_net_extensive

test_net_extensive: net_ext.o $(SRCDIR)/net.o
	$(CC) $(CFLAGS) -o $@ $(SRCDIR)/net.o net_ext.o

net_ext.o: test_net_extensive.c $(SRCDIR)/net.h
	$(CC) $(CFLAGS) -c test_net_extensive.c -o net_ext.o

clean:
	rm -f net_ext.o test_net_extensive
