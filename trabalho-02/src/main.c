#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "gpio_controller.h"
#include "uart_controller.h"

// Global variables for thread control
static volatile int running = 1;
static pthread_mutex_t motor_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread function for UART communication
void* uart_thread(void* arg) {
    uint8_t command_buffer[256];
    uint8_t buttons;
    float velocity_x = 0.0, velocity_y = 0.0;
    float position_x = 0.0, position_y = 0.0;
    float temperature = 25.0, pressure = 1013.0;  // Default values
    uint8_t machine_state = 0;
    
    while (running) {
        // Read buttons
        gpio_read_buttons(&buttons);
        
        // Read encoder positions
        int encoder_x, encoder_y;
        gpio_read_encoder_x(&encoder_x);
        gpio_read_encoder_y(&encoder_y);
        
        // Convert encoder counts to position (assuming 100 counts per cm)
        position_x = encoder_x / 100.0;
        position_y = encoder_y / 100.0;
        
        // Read limit switches
        uint8_t limits;
        gpio_read_limit_switches(&limits);
        
        // Handle button commands
        if (buttons & 0x01) {  // UP
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(0, 1, MOTOR_FREE, MOTOR_CW);
            gpio_set_motor_pwm(0, 50, 0, 50);  // 50% PWM
            pthread_mutex_unlock(&motor_mutex);
        }
        else if (buttons & 0x02) {  // DOWN
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(0, 1, MOTOR_FREE, MOTOR_CCW);
            gpio_set_motor_pwm(0, 50, 0, 50);
            pthread_mutex_unlock(&motor_mutex);
        }
        else if (buttons & 0x04) {  // LEFT
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(1, 0, MOTOR_CCW, MOTOR_FREE);
            gpio_set_motor_pwm(50, 0, 50, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        else if (buttons & 0x08) {  // RIGHT
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(1, 0, MOTOR_CW, MOTOR_FREE);
            gpio_set_motor_pwm(50, 0, 50, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        else if (buttons & 0x10) {  // EMERGENCY
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(1, 1, MOTOR_BRAKE, MOTOR_BRAKE);
            gpio_set_motor_pwm(0, 0, 0, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        else {
            // Stop motors if no button is pressed
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(1, 1, MOTOR_FREE, MOTOR_FREE);
            gpio_set_motor_pwm(0, 0, 0, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        
        // Check limit switches and stop motors if needed
        if (limits & 0x01) {  // X MIN
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(1, 0, MOTOR_FREE, MOTOR_FREE);
            gpio_set_motor_pwm(0, 0, 0, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        if (limits & 0x02) {  // X MAX
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(1, 0, MOTOR_FREE, MOTOR_FREE);
            gpio_set_motor_pwm(0, 0, 0, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        if (limits & 0x04) {  // Y MIN
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(0, 1, MOTOR_FREE, MOTOR_FREE);
            gpio_set_motor_pwm(0, 0, 0, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        if (limits & 0x08) {  // Y MAX
            pthread_mutex_lock(&motor_mutex);
            gpio_set_motor_direction(0, 1, MOTOR_FREE, MOTOR_FREE);
            gpio_set_motor_pwm(0, 0, 0, 0);
            pthread_mutex_unlock(&motor_mutex);
        }
        
        // Send status to touch screen
        uart_send_status(velocity_x, velocity_y,
                        position_x, position_y,
                        temperature, pressure,
                        machine_state);
        
        // Sleep for 50ms (20Hz update rate)
        usleep(50000);
    }
    
    return NULL;
}

// Signal handler for clean shutdown
void signal_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nReceived SIGINT. Cleaning up...\n");
        running = 0;
    }
}

int main(void) {
    pthread_t uart_thread_id;
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    
    // Initialize GPIO
    if (gpio_init() < 0) {
        printf("Failed to initialize GPIO\n");
        return -1;
    }
    
    // Initialize UART
    if (uart_init() < 0) {
        printf("Failed to initialize UART\n");
        gpio_cleanup();
        return -1;
    }
    
    // Create UART thread
    if (pthread_create(&uart_thread_id, NULL, uart_thread, NULL) != 0) {
        printf("Failed to create UART thread\n");
        uart_cleanup();
        gpio_cleanup();
        return -1;
    }
    
    printf("X-Ray Control System Started\n");
    printf("Press Ctrl+C to exit\n");
    
    // Wait for UART thread to finish
    pthread_join(uart_thread_id, NULL);
    
    // Cleanup
    uart_cleanup();
    gpio_cleanup();
    
    printf("System shutdown complete\n");
    return 0;
} 