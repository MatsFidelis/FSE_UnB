#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include "uart_controller.h"

static int uart_fd = -1;

// CRC-16 Modbus table
static const uint16_t crc16_table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    // ... (rest of the CRC table)
};

int uart_init(void) {
    struct termios options;
    
    // Open UART device
    uart_fd = open(UART_DEVICE, O_RDWR | O_NOCTTY);
    if (uart_fd < 0) {
        printf("Error opening UART: %s\n", strerror(errno));
        return -1;
    }
    
    // Get current options
    tcgetattr(uart_fd, &options);
    
    // Set baud rate
    cfsetispeed(&options, UART_BAUD_RATE);
    cfsetospeed(&options, UART_BAUD_RATE);
    
    // Set other options
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;  // No parity
    options.c_cflag &= ~CSTOPB;  // 1 stop bit
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;      // 8 bits
    
    // Set raw input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    // Set raw output
    options.c_oflag &= ~OPOST;
    
    // Set timeout
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = UART_TIMEOUT_MS / 100;  // Convert to deciseconds
    
    // Apply options
    tcsetattr(uart_fd, TCSANOW, &options);
    
    return 0;
}

void uart_cleanup(void) {
    if (uart_fd >= 0) {
        close(uart_fd);
        uart_fd = -1;
    }
}

uint16_t calculate_crc(uint8_t *data, int length) {
    uint16_t crc = 0xFFFF;
    
    for (int i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}

int uart_read_holding_registers(uint16_t start_addr, uint16_t quantity, uint8_t *data) {
    uint8_t request[8];
    uint8_t response[256];
    
    // Prepare request
    request[0] = MODBUS_SLAVE_ADDR;
    request[1] = MODBUS_READ_HOLDING;
    request[2] = (start_addr >> 8) & 0xFF;
    request[3] = start_addr & 0xFF;
    request[4] = (quantity >> 8) & 0xFF;
    request[5] = quantity & 0xFF;
    
    // Calculate and append CRC
    uint16_t crc = calculate_crc(request, 6);
    request[6] = (crc >> 8) & 0xFF;
    request[7] = crc & 0xFF;
    
    // Send request
    if (write(uart_fd, request, 8) != 8) {
        return -1;
    }
    
    // Read response
    int bytes_read = read(uart_fd, response, 5 + quantity * 2);
    if (bytes_read < 5) {
        return -1;
    }
    
    // Verify CRC
    crc = calculate_crc(response, bytes_read - 2);
    uint16_t received_crc = (response[bytes_read - 2] << 8) | response[bytes_read - 1];
    if (crc != received_crc) {
        return -1;
    }
    
    // Copy data
    memcpy(data, &response[3], quantity * 2);
    
    return 0;
}

int uart_write_single_register(uint16_t addr, uint16_t value) {
    uint8_t request[8];
    uint8_t response[8];
    
    // Prepare request
    request[0] = MODBUS_SLAVE_ADDR;
    request[1] = MODBUS_WRITE_SINGLE;
    request[2] = (addr >> 8) & 0xFF;
    request[3] = addr & 0xFF;
    request[4] = (value >> 8) & 0xFF;
    request[5] = value & 0xFF;
    
    // Calculate and append CRC
    uint16_t crc = calculate_crc(request, 6);
    request[6] = (crc >> 8) & 0xFF;
    request[7] = crc & 0xFF;
    
    // Send request
    if (write(uart_fd, request, 8) != 8) {
        return -1;
    }
    
    // Read response
    int bytes_read = read(uart_fd, response, 8);
    if (bytes_read != 8) {
        return -1;
    }
    
    // Verify CRC
    crc = calculate_crc(response, 6);
    uint16_t received_crc = (response[6] << 8) | response[7];
    if (crc != received_crc) {
        return -1;
    }
    
    return 0;
}

int uart_send_status(float velocity_x, float velocity_y, 
                    float position_x, float position_y,
                    float temperature, float pressure,
                    uint8_t machine_state) {
    // Convert float values to bytes
    uint8_t *vel_x_bytes = (uint8_t *)&velocity_x;
    uint8_t *vel_y_bytes = (uint8_t *)&velocity_y;
    uint8_t *pos_x_bytes = (uint8_t *)&position_x;
    uint8_t *pos_y_bytes = (uint8_t *)&position_y;
    uint8_t *temp_bytes = (uint8_t *)&temperature;
    uint8_t *press_bytes = (uint8_t *)&pressure;
    
    // Write each value to its register
    if (uart_write_single_register(REG_VELOCITY_X, *(uint16_t *)vel_x_bytes) < 0) return -1;
    if (uart_write_single_register(REG_VELOCITY_Y, *(uint16_t *)vel_y_bytes) < 0) return -1;
    if (uart_write_single_register(REG_POSITION_X, *(uint16_t *)pos_x_bytes) < 0) return -1;
    if (uart_write_single_register(REG_POSITION_Y, *(uint16_t *)pos_y_bytes) < 0) return -1;
    if (uart_write_single_register(REG_TEMPERATURE, *(uint16_t *)temp_bytes) < 0) return -1;
    if (uart_write_single_register(REG_PRESSURE, *(uint16_t *)press_bytes) < 0) return -1;
    if (uart_write_single_register(REG_MACHINE_STATE, machine_state) < 0) return -1;
    
    return 0;
}

int uart_handle_command(uint8_t *command, int length) {
    if (length < 8) return -1;  // Minimum Modbus message length
    
    // Verify slave address
    if (command[0] != MODBUS_SLAVE_ADDR) return -1;
    
    // Verify CRC
    uint16_t received_crc = (command[length-2] << 8) | command[length-1];
    uint16_t calculated_crc = calculate_crc(command, length-2);
    if (received_crc != calculated_crc) return -1;
    
    // Process command based on function code
    switch (command[1]) {
        case MODBUS_READ_HOLDING:
            // Handle read holding registers
            break;
            
        case MODBUS_WRITE_SINGLE:
            // Handle write single register
            break;
            
        default:
            return -1;
    }
    
    return 0;
} 