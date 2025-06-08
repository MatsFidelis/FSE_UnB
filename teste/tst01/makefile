CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude -Icrc

SRC=src
CRC=crc
OBJ=obj
BIN=botoes

SRCS=$(SRC)/main.c $(SRC)/modbus.c $(SRC)/uart.c $(CRC)/crc16.c
OBJS=$(SRCS:.c=.o)

all: $(BIN)

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(BIN)

clean:
	rm -f $(BIN)