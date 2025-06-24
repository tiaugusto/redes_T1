##############################################################################
#  Makefile – Projeto "redes_T1"
##############################################################################

# 1. Ferramentas & Flags
CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -g -Iinclude
LIBS    := -lncurses -ltinfo -lpthread

# 2. Fontes e binários
SRC_DIR := src
BIN_CLIENT := cliente
BIN_SERVER := servidor

# Arquivos específicos
CLIENT_SRC := $(SRC_DIR)/client.c $(SRC_DIR)/protocol_net.c \
              $(SRC_DIR)/socket.c $(SRC_DIR)/utils.c \
              $(SRC_DIR)/ui_client.c $(SRC_DIR)/protocol.c

SERVER_SRC := $(SRC_DIR)/server.c $(SRC_DIR)/protocol_net.c \
              $(SRC_DIR)/socket.c $(SRC_DIR)/utils.c \
              $(SRC_DIR)/ui_server.c $(SRC_DIR)/protocol.c


CLIENT_OBJS := $(CLIENT_SRC:.c=.o)
SERVER_OBJS := $(SERVER_SRC:.c=.o)

# 3. Alvos principais
.PHONY: all clean
all: $(BIN_CLIENT) $(BIN_SERVER)

$(BIN_CLIENT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BIN_SERVER): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 4. Compilação de objetos
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 5. Limpeza
clean:
	$(RM) $(CLIENT_OBJS) $(SERVER_OBJS) $(BIN_CLIENT) $(BIN_SERVER)
