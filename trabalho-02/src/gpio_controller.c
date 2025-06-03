#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softPwm.h>
#include "gpio_controller.h"

// PWM configuration
#define PWM_RANGE 100
#define PWM_FREQ 1000  // 1kHz

// Encoder state tracking
static volatile int encoder_x_position = 0;
static volatile int encoder_y_position = 0;
static volatile int encoder_x_last_state = 0;
static volatile int encoder_y_last_state = 0;

// Interrupt handlers for encoders
void encoder_x_isr(void) {
    int MSB = digitalRead(ENCODER_X_A);
    int LSB = digitalRead(ENCODER_X_B);
    int current_state = (MSB << 1) | LSB;
    
    if (current_state != encoder_x_last_state) {
        if ((encoder_x_last_state == 0 && current_state == 1) ||
            (encoder_x_last_state == 1 && current_state == 3) ||
            (encoder_x_last_state == 3 && current_state == 2) ||
            (encoder_x_last_state == 2 && current_state == 0)) {
            encoder_x_position++;
        } else {
            encoder_x_position--;
        }
        encoder_x_last_state = current_state;
    }
}

void encoder_y_isr(void) {
    int MSB = digitalRead(ENCODER_Y_A);
    int LSB = digitalRead(ENCODER_Y_B);
    int current_state = (MSB << 1) | LSB;
    
    if (current_state != encoder_y_last_state) {
        if ((encoder_y_last_state == 0 && current_state == 1) ||
            (encoder_y_last_state == 1 && current_state == 3) ||
            (encoder_y_last_state == 3 && current_state == 2) ||
            (encoder_y_last_state == 2 && current_state == 0)) {
            encoder_y_position++;
        } else {
            encoder_y_position--;
        }
        encoder_y_last_state = current_state;
    }
}

int gpio_init(void) {
    // Initialize WiringPi
    if (wiringPiSetup() == -1) {
        printf("Failed to initialize WiringPi\n");
        return -1;
    }

    // Configure motor control pins
    pinMode(MOTOR_X_PWM, OUTPUT);
    pinMode(MOTOR_X_DIR1, OUTPUT);
    pinMode(MOTOR_X_DIR2, OUTPUT);
    pinMode(MOTOR_Y_PWM, OUTPUT);
    pinMode(MOTOR_Y_DIR1, OUTPUT);
    pinMode(MOTOR_Y_DIR2, OUTPUT);

    // Configure encoder pins
    pinMode(ENCODER_X_A, INPUT);
    pinMode(ENCODER_X_B, INPUT);
    pinMode(ENCODER_Y_A, INPUT);
    pinMode(ENCODER_Y_B, INPUT);

    // Configure limit switch pins
    pinMode(LIMIT_X_MIN, INPUT);
    pinMode(LIMIT_X_MAX, INPUT);
    pinMode(LIMIT_Y_MIN, INPUT);
    pinMode(LIMIT_Y_MAX, INPUT);

    // Configure button pins
    pinMode(BTN_UP, INPUT);
    pinMode(BTN_DOWN, INPUT);
    pinMode(BTN_LEFT, INPUT);
    pinMode(BTN_RIGHT, INPUT);
    pinMode(BTN_EMERGENCY, INPUT);

    // Configure X-ray capture pin
    pinMode(XRAY_CAPTURE, OUTPUT);

    // Initialize PWM
    softPwmCreate(MOTOR_X_PWM, 0, PWM_RANGE);
    softPwmCreate(MOTOR_Y_PWM, 0, PWM_RANGE);

    // Set up encoder interrupts
    wiringPiISR(ENCODER_X_A, INT_EDGE_BOTH, &encoder_x_isr);
    wiringPiISR(ENCODER_X_B, INT_EDGE_BOTH, &encoder_x_isr);
    wiringPiISR(ENCODER_Y_A, INT_EDGE_BOTH, &encoder_y_isr);
    wiringPiISR(ENCODER_Y_B, INT_EDGE_BOTH, &encoder_y_isr);

    return 0;
}

void gpio_cleanup(void) {
    // Stop motors
    gpio_set_motor_direction(0, 0, MOTOR_FREE, MOTOR_FREE);
    gpio_set_motor_pwm(0, 0, 0, 0);
    
    // Clean up PWM
    softPwmStop(MOTOR_X_PWM);
    softPwmStop(MOTOR_Y_PWM);
}

int gpio_set_motor_direction(int motor_x, int motor_y, MotorDirection dir_x, MotorDirection dir_y) {
    // Set X motor direction
    switch (dir_x) {
        case MOTOR_FREE:
            digitalWrite(MOTOR_X_DIR1, LOW);
            digitalWrite(MOTOR_X_DIR2, LOW);
            break;
        case MOTOR_CW:
            digitalWrite(MOTOR_X_DIR1, HIGH);
            digitalWrite(MOTOR_X_DIR2, LOW);
            break;
        case MOTOR_CCW:
            digitalWrite(MOTOR_X_DIR1, LOW);
            digitalWrite(MOTOR_X_DIR2, HIGH);
            break;
        case MOTOR_BRAKE:
            digitalWrite(MOTOR_X_DIR1, HIGH);
            digitalWrite(MOTOR_X_DIR2, HIGH);
            break;
    }

    // Set Y motor direction
    switch (dir_y) {
        case MOTOR_FREE:
            digitalWrite(MOTOR_Y_DIR1, LOW);
            digitalWrite(MOTOR_Y_DIR2, LOW);
            break;
        case MOTOR_CW:
            digitalWrite(MOTOR_Y_DIR1, HIGH);
            digitalWrite(MOTOR_Y_DIR2, LOW);
            break;
        case MOTOR_CCW:
            digitalWrite(MOTOR_Y_DIR1, LOW);
            digitalWrite(MOTOR_Y_DIR2, HIGH);
            break;
        case MOTOR_BRAKE:
            digitalWrite(MOTOR_Y_DIR1, HIGH);
            digitalWrite(MOTOR_Y_DIR2, HIGH);
            break;
    }

    return 0;
}

int gpio_set_motor_pwm(int motor_x, int motor_y, uint8_t pwm_x, uint8_t pwm_y) {
    if (pwm_x > 100) pwm_x = 100;
    if (pwm_y > 100) pwm_y = 100;

    softPwmWrite(MOTOR_X_PWM, pwm_x);
    softPwmWrite(MOTOR_Y_PWM, pwm_y);

    return 0;
}

int gpio_read_encoder_x(int *position) {
    *position = encoder_x_position;
    return 0;
}

int gpio_read_encoder_y(int *position) {
    *position = encoder_y_position;
    return 0;
}

int gpio_read_limit_switches(uint8_t *limits) {
    *limits = 0;
    
    if (digitalRead(LIMIT_X_MIN)) *limits |= 0x01;
    if (digitalRead(LIMIT_X_MAX)) *limits |= 0x02;
    if (digitalRead(LIMIT_Y_MIN)) *limits |= 0x04;
    if (digitalRead(LIMIT_Y_MAX)) *limits |= 0x08;
    
    return 0;
}

int gpio_read_buttons(uint8_t *buttons) {
    *buttons = 0;
    
    if (digitalRead(BTN_UP)) *buttons |= 0x01;
    if (digitalRead(BTN_DOWN)) *buttons |= 0x02;
    if (digitalRead(BTN_LEFT)) *buttons |= 0x04;
    if (digitalRead(BTN_RIGHT)) *buttons |= 0x08;
    if (digitalRead(BTN_EMERGENCY)) *buttons |= 0x10;
    
    return 0;
}

int gpio_trigger_xray_capture(void) {
    digitalWrite(XRAY_CAPTURE, HIGH);
    delay(100);  // Pulse width of 100ms
    digitalWrite(XRAY_CAPTURE, LOW);
    return 0;
} 