# UART/Modbus Communication Program

This program implements UART/Modbus RTU communication between a Raspberry Pi and ESP32. It provides functionality for reading and writing Modbus registers with CRC verification.

## Features

- UART communication at 115200 baud rate (8N1)
- Modbus RTU protocol implementation
- CRC calculation and verification
- Reading holding registers (function 0x03)
- Writing to single registers (function 0x06)
- Debug prints for data reception and CRC verification

## Requirements

- Linux-based system (tested on Raspberry Pi)
- GCC compiler
- UART device connected to the system

## Compilation

To compile the program, simply run:

```bash
make
```

To clean the compiled files:

```bash
make clean
```

## Usage

1. Make sure your UART device is properly connected to the Raspberry Pi
2. Update the `UART_DEVICE` constant in `modbus_uart.c` if your UART device is not `/dev/ttyS0`
3. Run the program:

```bash
./modbus_uart
```

## Program Output

The program will:
1. Initialize the UART connection
2. Test reading holding registers (function 0x03)
3. Test writing to a register (function 0x06)
4. Print all sent and received data in hexadecimal format
5. Verify CRC for all received messages

## Error Handling

The program includes error handling for:
- UART initialization failures
- Communication errors
- CRC verification failures
- Invalid response lengths

## Notes

- The program uses the Modbus slave address 0x01 by default
- The UART is configured for 115200 baud rate, 8 data bits, no parity, and 1 stop bit
- Timeout is set to 1 second for reading responses 