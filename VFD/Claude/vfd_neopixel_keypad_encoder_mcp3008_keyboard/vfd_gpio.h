//===============================
// FILE: vfd_gpio.h
//===============================
#ifndef VFD_GPIO_H
#define VFD_GPIO_H

#include <pigpio.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <rpi_ws281x/ws2811.h>
#include "vfd_display.h"
#include "vfd_keypad.h"
#include "vfd_adc.h"  // Added this include

// Existing pin definitions
#define MODE_BUTTON_PIN 22    // Physical pin 16, active high
#define NEOPIXEL_PIN 18      // Physical pin 12 (GPIO 18)
#define LED_COUNT 12         // 12-LED NeoPixel ring

// New encoder pins
#define ENCODER_A_PIN 12
#define ENCODER_B_PIN 16
#define ENCODER_STEP_SIZE 5

// NeoPixel configuration
#define LED_FREQ     800000  // LED frequency
#define LED_DMA      10      // DMA channel
#define LED_INVERT   0       // Don't invert signal
#define LED_CHANNEL  0       // PWM channel
#define LED_STRIP    WS2811_STRIP_GRB

// Define VFD modes
typedef enum {
    MODE_SSH = 0,
    MODE_LOCAL_KB = 1,
    MODE_AUTO = 2,
    MODE_COUNT
} VFD_Mode;

// Color definitions for NeoPixel
#define COLOR_SSH    0x00FF00    // Green
#define COLOR_LOCAL  0x0000FF    // Blue
#define COLOR_AUTO   0xFF0000    // Red
#define COLOR_OFF    0x000000    // Off

// GPIO Control structure
typedef struct {
    ws2811_t ws2811;
    VFD_Mode current_mode;
    int button_debounce;
    volatile int mode_changed;
    // Encoder members
    volatile int encoder_value;
    volatile int last_encoder_a;
    volatile int last_encoder_b;
    volatile int encoder_changed;
    // Keypad member
    Keypad_Control* keypad;
    // ADC member
    ADC_Control* adc;
} GPIO_Control;

// Function declarations
GPIO_Control* gpio_init(void);
void gpio_cleanup(GPIO_Control* gpio);
void update_neopixel_mode(GPIO_Control* gpio);
void button_callback(int gpio, int level, uint32_t tick);
void encoder_callback_a(int gpio, int level, uint32_t tick);
void encoder_callback_b(int gpio, int level, uint32_t tick);
const char* get_mode_name(VFD_Mode mode);
void show_mode_instructions(VFD_Display* vfd, VFD_Mode mode);
void show_encoder_value(VFD_Display* vfd, int value);

#endif