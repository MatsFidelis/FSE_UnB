CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = 

TARGET = modbus_uart
SRC = modbus_uart.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean 