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

# Caminhos de inclusão
INC_SHARED := -Ishared/include

# CFLAGS por “módulo”
CFLAGS_SHARED := $(CSTD) $(WARN) $(OPT) $(DEF) $(DBG) $(INC_SHARED)
CFLAGS_CLIENT := $(CSTD) $(WARN) $(OPT) $(DEF) $(DBG) $(INC_SHARED)
CFLAGS_SERVER := $(CSTD) $(WARN) $(OPT) $(DEF) $(DBG) $(INC_SHARED)

# Bibliotecas de linkedição
LIBS := -Lshared -lshared -lncurses -ltinfo -lpthread

##########################
# 2. Fontes & objetos    #
##########################
# --- shared ---
SHARED_SRC  := $(wildcard shared/src/*.c)
SHARED_OBJS := $(patsubst shared/src/%.c,shared/src/%.o,$(SHARED_SRC))
SHARED_LIB  := shared/libshared.a

# --- raiz ---
SERVER_SRC  := server.c
SERVER_OBJS := $(patsubst %.c,%.o,$(SERVER_SRC))
SERVER_BIN  := servidor          # binário na raiz

CLIENT_SRC  := client.c
CLIENT_OBJS := $(patsubst %.c,%.o,$(CLIENT_SRC))
CLIENT_BIN  := cliente           # binário na raiz

##########################
# 3. Alvos principais    #
##########################
.PHONY: all clean client server
all: $(SERVER_BIN) $(CLIENT_BIN)

# Atalhos
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
# 5. Regras genéricas / específicas
##################################
# Objetos da pasta shared/src
shared/src/%.o: shared/src/%.c
	$(CC) $(CFLAGS_SHARED) -c -o $@ $<

# Objetos na raiz
server.o: server.c
	$(CC) $(CFLAGS_SERVER) -c -o $@ $<

client.o: client.c
	$(CC) $(CFLAGS_CLIENT) -c -o $@ $<

##########################
# 6. Limpeza             #
##########################
clean:
	$(RM) $(SERVER_OBJS) $(CLIENT_OBJS) $(SHARED_OBJS) \
	      $(SERVER_BIN)  $(CLIENT_BIN) $(SHARED_LIB)
