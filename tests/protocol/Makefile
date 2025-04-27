CC      = gcc
INCDIR  = $(SRCDIR)
CFLAGS  = -std=c11 -Wall -Wextra -O2 -I$(INCDIR)
SRCDIR  = ../../src

all: test_protocol_extensive

# linka objeto de src com o objeto do teste
test_protocol_extensive: protocol_ext.o $(SRCDIR)/protocol.o
	$(CC) $(CFLAGS) -o $@ $(SRCDIR)/protocol.o protocol_ext.o

# compila apenas o teste em um objeto
protocol_ext.o: test_protocol_extensive.c $(SRCDIR)/protocol.h
	$(CC) $(CFLAGS) -c test_protocol_extensive.c -o protocol_ext.o

clean:
	rm -f protocol_ext.o test_protocol_extensive
