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

#define MODE_BUTTON_PIN 22    // Physical pin 16, active high
#define NEOPIXEL_PIN 18      // Physical pin 12 (GPIO 18)
#define LED_COUNT 12         // 12-LED NeoPixel ring

// NeoPixel configuration
#define LED_FREQ     800000  // LED frequency
#define LED_DMA      10      // DMA channel
#define LED_INVERT   0       // Don't invert signal
#define LED_CHANNEL  0       // PWM channel
#define LED_STRIP    WS2811_STRIP_GRB

typedef enum {
    MODE_SSH = 0,
    MODE_LOCAL_KB = 1,
    MODE_AUTO = 2,
    MODE_COUNT
} VFD_Mode;

typedef struct {
    ws2811_t ws2811;
    VFD_Mode current_mode;
    int button_debounce;
    volatile int mode_changed;
} GPIO_Control;

// Color definitions for NeoPixel
#define COLOR_SSH    0x00FF00    // Green
#define COLOR_LOCAL  0x0000FF    // Blue
#define COLOR_AUTO   0xFF0000    // Red
#define COLOR_OFF    0x000000    // Off

// Function declarations
GPIO_Control* gpio_init(void);
void gpio_cleanup(GPIO_Control* gpio);
void update_neopixel_mode(GPIO_Control* gpio);
void button_callback(int gpio, int level, uint32_t tick);
const char* get_mode_name(VFD_Mode mode);
void show_mode_instructions(VFD_Display* vfd, VFD_Mode mode);

#endif
