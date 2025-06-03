#ifndef GPIO_CONTROLLER_H
#define GPIO_CONTROLLER_H

#include <stdint.h>

// GPIO Pin Definitions (BCM numbering)
#define MOTOR_X_PWM    17
#define MOTOR_X_DIR1   27
#define MOTOR_X_DIR2   22
#define MOTOR_Y_PWM    23
#define MOTOR_Y_DIR1   24
#define MOTOR_Y_DIR2   25
#define ENCODER_X_A    5
#define ENCODER_X_B    6
#define ENCODER_Y_A    12
#define ENCODER_Y_B    13
#define LIMIT_X_MIN    26
#define LIMIT_X_MAX    19
#define LIMIT_Y_MIN    20
#define LIMIT_Y_MAX    21
#define XRAY_CAPTURE   18
#define BTN_UP         16
#define BTN_DOWN       1
#define BTN_LEFT       7
#define BTN_RIGHT      8
#define BTN_EMERGENCY  11

// Motor direction definitions
typedef enum {
    MOTOR_FREE = 0,
    MOTOR_CW = 1,
    MOTOR_CCW = 2,
    MOTOR_BRAKE = 3
} MotorDirection;

// Function declarations
int gpio_init(void);
void gpio_cleanup(void);

// Motor control functions
int gpio_set_motor_direction(int motor_x, int motor_y, MotorDirection dir_x, MotorDirection dir_y);
int gpio_set_motor_pwm(int motor_x, int motor_y, uint8_t pwm_x, uint8_t pwm_y);

// Encoder functions
int gpio_read_encoder_x(int *position);
int gpio_read_encoder_y(int *position);

// Limit switch functions
int gpio_read_limit_switches(uint8_t *limits);

// Button functions
int gpio_read_buttons(uint8_t *buttons);

// X-ray control
int gpio_trigger_xray_capture(void);

#endif // GPIO_CONTROLLER_H 