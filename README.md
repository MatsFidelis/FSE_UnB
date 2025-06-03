# FSE_UnB

trabalho-02/
├── include/                    # Header files
│   ├── gpio_controller.h      # GPIO layer interface
│   ├── uart_controller.h      # UART/Modbus layer interface
│   ├── i2c_controller.h       # I2C/Temperature sensor layer interface
│   ├── motor_controller.h     # Motor control interface
│   ├── encoder_controller.h   # Encoder interface
│   ├── pid_controller.h       # PID control interface
│   └── system_controller.h    # Top-level system interface
├── src/                       # Source files
│   ├── gpio_controller.c
│   ├── uart_controller.c
│   ├── i2c_controller.c
│   ├── motor_controller.c
│   ├── encoder_controller.c
│   ├── pid_controller.c
│   └── system_controller.c
├── main.c                     # Main program
└── Makefile

