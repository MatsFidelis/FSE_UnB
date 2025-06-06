#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdint.h>

// Modbus RTU constants
#define MODBUS_SLAVE_ADDR 0x01
#define MODBUS_READ_HOLDING_REG 0x03
#define MODBUS_WRITE_SINGLE_REG 0x06
#define UART_DEVICE "/dev/ttyS0"  // Change this according to your UART device
#define BAUD_RATE B115200

// Function prototypes
int init_uart(const char *device);
uint16_t calculate_crc(uint8_t *data, size_t length);
int send_modbus_request(int uart_fd, uint8_t function, uint16_t address, uint16_t quantity, uint16_t *data);
int receive_modbus_response(int uart_fd, uint8_t *response, size_t max_length);
void print_buffer(const char *prefix, uint8_t *buffer, size_t length);

int main() {
    int uart_fd;
    uint8_t response[256];
    uint16_t test_data = 0x1234;

    // Initialize UART
    uart_fd = init_uart(UART_DEVICE);
    if (uart_fd < 0) {
        printf("Failed to initialize UART\n");
        return -1;
    }

    printf("UART initialized successfully\n");

    // Test reading holding registers (function 0x03)
    printf("\nTesting Read Holding Registers (0x03):\n");
    if (send_modbus_request(uart_fd, MODBUS_READ_HOLDING_REG, 0x00, 1, NULL) < 0) {
        printf("Failed to send read request\n");
        close(uart_fd);
        return -1;
    }

    int bytes_received = receive_modbus_response(uart_fd, response, sizeof(response));
    if (bytes_received > 0) {
        printf("Received %d bytes\n", bytes_received);
        print_buffer("Response:", response, bytes_received);
    }

    // Test writing to a register (function 0x06)
    printf("\nTesting Write Single Register (0x06):\n");
    if (send_modbus_request(uart_fd, MODBUS_WRITE_SINGLE_REG, 0x00, 1, &test_data) < 0) {
        printf("Failed to send write request\n");
        close(uart_fd);
        return -1;
    }

    bytes_received = receive_modbus_response(uart_fd, response, sizeof(response));
    if (bytes_received > 0) {
        printf("Received %d bytes\n", bytes_received);
        print_buffer("Response:", response, bytes_received);
    }

    close(uart_fd);
    return 0;
}

int init_uart(const char *device) {
    int uart_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fd < 0) {
        perror("Error opening UART device");
        return -1;
    }

    struct termios options;
    tcgetattr(uart_fd, &options);
    
    // Set baud rate
    cfsetispeed(&options, BAUD_RATE);
    cfsetospeed(&options, BAUD_RATE);
    
    // Set other UART parameters
    options.c_cflag |= (CLOCAL | CREAD);    // Ignore modem controls
    options.c_cflag &= ~PARENB;             // No parity
    options.c_cflag &= ~CSTOPB;             // 1 stop bit
    options.c_cflag &= ~CSIZE;              // Clear size bits
    options.c_cflag |= CS8;                 // 8 bits
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // Raw input
    options.c_oflag &= ~OPOST;              // Raw output
    
    // Set read timeout
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10;
    
    tcflush(uart_fd, TCIFLUSH);
    if (tcsetattr(uart_fd, TCSANOW, &options) < 0) {
        perror("Error setting UART attributes");
        close(uart_fd);
        return -1;
    }
    
    return uart_fd;
}

uint16_t calculate_crc(uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i];
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

int send_modbus_request(int uart_fd, uint8_t function, uint16_t address, uint16_t quantity, uint16_t *data) {
    uint8_t request[256];
    size_t request_length = 0;
    
    // Build Modbus RTU frame
    request[request_length++] = MODBUS_SLAVE_ADDR;
    request[request_length++] = function;
    request[request_length++] = (address >> 8) & 0xFF;
    request[request_length++] = address & 0xFF;
    
    if (function == MODBUS_READ_HOLDING_REG) {
        request[request_length++] = (quantity >> 8) & 0xFF;
        request[request_length++] = quantity & 0xFF;
    } else if (function == MODBUS_WRITE_SINGLE_REG && data != NULL) {
        request[request_length++] = (data[0] >> 8) & 0xFF;
        request[request_length++] = data[0] & 0xFF;
    }
    
    // Calculate and append CRC
    uint16_t crc = calculate_crc(request, request_length);
    request[request_length++] = crc & 0xFF;
    request[request_length++] = (crc >> 8) & 0xFF;
    
    // Print request for debugging
    print_buffer("Sending:", request, request_length);
    
    // Send request
    ssize_t bytes_written = write(uart_fd, request, request_length);
    if (bytes_written != request_length) {
        perror("Error writing to UART");
        return -1;
    }
    
    return 0;
}

int receive_modbus_response(int uart_fd, uint8_t *response, size_t max_length) {
    size_t bytes_received = 0;
    uint8_t buffer[256];
    
    // Read response with timeout
    while (bytes_received < max_length) {
        ssize_t bytes_read = read(uart_fd, &buffer[bytes_received], 1);
        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data available, check if we've received anything
                if (bytes_received > 0) {
                    break;
                }
                usleep(10000); // Sleep for 10ms
                continue;
            }
            perror("Error reading from UART");
            return -1;
        } else if (bytes_read == 0) {
            // End of file
            break;
        }
        bytes_received++;
    }
    
    if (bytes_received < 5) { // Minimum valid Modbus response length
        printf("Response too short\n");
        return -1;
    }
    
    // Verify CRC
    uint16_t received_crc = (buffer[bytes_received-1] << 8) | buffer[bytes_received-2];
    uint16_t calculated_crc = calculate_crc(buffer, bytes_received-2);
    
    if (received_crc != calculated_crc) {
        printf("CRC verification failed. Received: 0x%04X, Calculated: 0x%04X\n", 
               received_crc, calculated_crc);
        return -1;
    }
    
    printf("CRC verification successful\n");
    memcpy(response, buffer, bytes_received);
    return bytes_received;
}

void print_buffer(const char *prefix, uint8_t *buffer, size_t length) {
    printf("%s ", prefix);
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
} 