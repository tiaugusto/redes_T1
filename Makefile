##############################################################################
#  Makefile – Projeto "redes_T1"
##############################################################################

##########################
# 1. Ferramentas & flags #
##########################
CC      := gcc
AR      := ar
ARFLAGS := rcs

CSTD    := -std=c11
WARN    := -Wall -Wextra -pedantic
DBG     := -g
OPT     := -O2
DEF     := -D_DEFAULT_SOURCE

CFLAGS_SHARED := $(CSTD) $(WARN) $(OPT) $(DEF) $(DBG) -Ishared/include
CFLAGS_CLIENT := $(CSTD) $(WARN) $(OPT) $(DEF) $(DBG) -Iclient/include -Ishared/include
CFLAGS_SERVER := $(CSTD) $(WARN) $(OPT) $(DEF) $(DBG) -Iserver/include -Ishared/include

LIBS := -Lshared -lshared -lncurses -ltinfo -lpthread

##########################
# 2. Fontes & objetos    #
##########################
SHARED_SRC  := $(wildcard shared/src/*.c)
SHARED_OBJS := $(patsubst shared/src/%.c,shared/src/%.o,$(SHARED_SRC))
SHARED_LIB  := shared/libshared.a

SERVER_SRC  := $(wildcard server/src/*.c)
SERVER_OBJS := $(patsubst server/src/%.c,server/src/%.o,$(SERVER_SRC))
SERVER_BIN  := servidor        # ← binário na raiz

CLIENT_SRC  := $(wildcard client/src/*.c)
CLIENT_OBJS := $(patsubst client/src/%.c,client/src/%.o,$(CLIENT_SRC))
CLIENT_BIN  := cliente         # ← binário na raiz

##########################
# 3. Alvos principais    #
##########################
.PHONY: all clean client server
all: $(SERVER_BIN) $(CLIENT_BIN)

# Atalhos para quem digitar “make client” ou “make server”
client: $(CLIENT_BIN)
server: $(SERVER_BIN)

########################################
# 4. Regras de compilação / linkagem   #
########################################
$(SHARED_LIB): $(SHARED_OBJS)
	$(AR) $(ARFLAGS) $@ $^

$(SERVER_BIN): $(SERVER_OBJS) $(SHARED_LIB)
	$(CC) $(CFLAGS_SERVER) -o $@ $(SERVER_OBJS) $(LIBS)

$(CLIENT_BIN): $(CLIENT_OBJS) $(SHARED_LIB)
	$(CC) $(CFLAGS_CLIENT) -o $@ $(CLIENT_OBJS) $(LIBS)

##################################
# 5. Regras genéricas para .o    #
##################################
shared/src/%.o: shared/src/%.c
	$(CC) $(CFLAGS_SHARED) -c -o $@ $<

server/src/%.o: server/src/%.c
	$(CC) $(CFLAGS_SERVER) -c -o $@ $<

client/src/%.o: client/src/%.c
	$(CC) $(CFLAGS_CLIENT) -c -o $@ $<

##########################
# 6. Limpeza             #
##########################
clean:
	$(RM) $(SERVER_OBJS) $(CLIENT_OBJS) $(SHARED_OBJS) \
	      $(SERVER_BIN)  $(CLIENT_BIN) $(SHARED_LIB)
