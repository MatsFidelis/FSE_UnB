#ifndef UART_CONTROLLER_H
#define UART_CONTROLLER_H

#include <stdint.h>

// Modbus definitions
#define MODBUS_SLAVE_ADDR      0x01
#define MODBUS_READ_HOLDING    0x03
#define MODBUS_WRITE_SINGLE    0x06

// Register addresses for commands
#define REG_MOVE_X             0x00
#define REG_MOVE_Y             0x01
#define REG_PREDEF_POS         0x02
#define REG_PROGRAM_POS        0x03
#define REG_CALIBRATE          0x04

// Register addresses for display
#define REG_VELOCITY_X         0x05
#define REG_VELOCITY_Y         0x09
#define REG_POSITION_X         0x0D
#define REG_POSITION_Y         0x11
#define REG_TEMPERATURE        0x15
#define REG_PRESSURE           0x19
#define REG_MACHINE_STATE      0x1D

// UART configuration
#define UART_DEVICE            "/dev/ttyAMA0"
#define UART_BAUD_RATE         B115200
#define UART_TIMEOUT_MS        50

// Function declarations
int uart_init(void);
void uart_cleanup(void);

// Modbus communication functions
int uart_read_holding_registers(uint16_t start_addr, uint16_t quantity, uint8_t *data);
int uart_write_single_register(uint16_t addr, uint16_t value);

// Command handling functions
int uart_handle_command(uint8_t *command, int length);
int uart_send_status(float velocity_x, float velocity_y, 
                     float position_x, float position_y,
                     float temperature, float pressure,
                     uint8_t machine_state);

// CRC calculation
uint16_t calculate_crc(uint8_t *data, int length);

#endif // UART_CONTROLLER_H 