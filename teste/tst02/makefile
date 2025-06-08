CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude -Icrc

SRC=src
CRC=crc
BIN=main

SRCS=$(SRC)/main.c $(SRC)/modbus.c $(SRC)/uart.c $(SRC)/botoes_fisicos.c $(SRC)/raiox.c $(CRC)/crc16.c $(SRC)/encoder.c $(SRC)/motores.c $(SRC)/dashboard.c $(SRC)/calibracao.c

all: $(BIN)

$(BIN): $(SRCS)
        $(CC) $(CFLAGS) $(SRCS) -lwiringPi -o $(BIN)

clean:
        rm -f $(BIN)