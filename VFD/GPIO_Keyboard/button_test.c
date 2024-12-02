//===============================
// FILE: button_test.c
//===============================
#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <rpi_ws281x/ws2811.h>
#include "vfd_display.h"

// Pin definitions
#define MODE_BUTTON_PIN 22    // Physical pin 16
#define NEW_BUTTON_PIN 6      // Physical pin 31
#define NEOPIXEL_PIN 18      // Physical pin 12 (GPIO 18)
#define LED_COUNT 12         // 12-LED NeoPixel ring

// NeoPixel configuration
#define LED_FREQ     800000
#define LED_DMA      10
#define LED_INVERT   0
#define LED_CHANNEL  0
#define LED_STRIP    WS2811_STRIP_GRB

// Color definitions
#define COLOR_ON     0x00FF00    // Green
#define COLOR_OFF    0x000000    // Off

ws2811_t ledstring = {
    .freq = LED_FREQ,
    .dmanum = LED_DMA,
    .channel = {
        [0] = {
            .gpionum = NEOPIXEL_PIN,
            .count = LED_COUNT,
            .invert = LED_INVERT,
            .brightness = 128,
            .strip_type = LED_STRIP,
        },
        [1] = {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};

VFD_Display* vfd = NULL;

void cleanup(int signum) {
    // Clear all LEDs
    for (int i = 0; i < LED_COUNT; i++) {
        ledstring.channel[0].leds[i] = COLOR_OFF;
    }
    ws2811_render(&ledstring);
    ws2811_fini(&ledstring);
    
    // Cleanup VFD
    if (vfd) {
        vfd_clear(vfd);
        vfd_write(vfd, "Program terminated.\n");
        vfd_close(vfd);
    }
    
    // Cleanup pigpio
    gpioTerminate();
    exit(signum);
}

void button_callback(int gpio_pin, int level, uint32_t tick) {
    static uint32_t last_tick = 0;
    
    // Debug output for all GPIO changes
    printf("GPIO %d changed to %d at tick %u\n", gpio_pin, level, tick);
    
    // Simple debounce (200ms)
    if (tick - last_tick < 200000) return;
    last_tick = tick;
    
    if (gpio_pin == NEW_BUTTON_PIN) {
        if (level == 0) {  // Active LOW - button pressed (pulling down from high)
            printf("New Button PRESSED at tick %u\n", tick);
            if (vfd) {
                vfd_clear(vfd);
                vfd_write(vfd, "Button on GPIO 6\nwas PRESSED!\n");
            }
            // Turn LEDs blue when pressed
            for (int i = 0; i < LED_COUNT; i++) {
                ledstring.channel[0].leds[i] = 0x0000FF;  // Blue
            }
        } else {
            printf("New Button RELEASED at tick %u\n", tick);
            if (vfd) {
                vfd_clear(vfd);
                vfd_write(vfd, "Button on GPIO 6\nwas RELEASED!\n");
            }
            // Turn LEDs off when released
            for (int i = 0; i < LED_COUNT; i++) {
                ledstring.channel[0].leds[i] = COLOR_OFF;
            }
        }
        ws2811_render(&ledstring);
    } else if (gpio_pin == MODE_BUTTON_PIN) {
        if (level == 1) {  // Active HIGH - button pressed (pulling up from low)
            printf("Original Button PRESSED at tick %u\n", tick);
            if (vfd) {
                vfd_clear(vfd);
                vfd_write(vfd, "Button on GPIO 22\nwas PRESSED!\n");
            }
            // Turn LEDs green when pressed
            for (int i = 0; i < LED_COUNT; i++) {
                ledstring.channel[0].leds[i] = COLOR_ON;
            }
        } else {
            printf("Original Button RELEASED at tick %u\n", tick);
            if (vfd) {
                vfd_clear(vfd);
                vfd_write(vfd, "Button on GPIO 22\nwas RELEASED!\n");
            }
            // Turn LEDs off when released
            for (int i = 0; i < LED_COUNT; i++) {
                ledstring.channel[0].leds[i] = COLOR_OFF;
            }
        }
        ws2811_render(&ledstring);
    }
}

int main() {
    printf("Starting button test program...\n");
    
    // Set up signal handlers
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    // Initialize pigpio
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize pigpio\n");
        return 1;
    }

    // Initialize VFD
    vfd = vfd_init("/dev/ttyUSB0");
    if (!vfd) {
        fprintf(stderr, "Failed to initialize VFD display\n");
        gpioTerminate();
        return 1;
    }
    
    // Initialize WS2811
    if (ws2811_init(&ledstring) != WS2811_SUCCESS) {
        fprintf(stderr, "Failed to initialize WS2811\n");
        if (vfd) vfd_close(vfd);
        gpioTerminate();
        return 1;
    }

    // Configure GPIO 22 with pull-down (starts low, button pulls high)
    if (gpioSetMode(MODE_BUTTON_PIN, PI_INPUT) != 0 ||
        gpioSetPullUpDown(MODE_BUTTON_PIN, PI_PUD_DOWN) != 0) {
        fprintf(stderr, "Failed to configure original button\n");
        cleanup(1);
        return 1;
    }

    // Configure GPIO 6 with pull-up (starts high, button pulls low)
    if (gpioSetMode(NEW_BUTTON_PIN, PI_INPUT) != 0 ||
        gpioSetPullUpDown(NEW_BUTTON_PIN, PI_PUD_UP) != 0) {
        fprintf(stderr, "Failed to configure new button\n");
        cleanup(1);
        return 1;
    }

    // Set up GPIO monitoring with callback for both buttons
    if (gpioSetAlertFunc(MODE_BUTTON_PIN, button_callback) != 0 ||
        gpioSetAlertFunc(NEW_BUTTON_PIN, button_callback) != 0) {
        fprintf(stderr, "Failed to set alert functions\n");
        cleanup(1);
        return 1;
    }

    // Turn off all NeoPixels initially
    for (int i = 0; i < LED_COUNT; i++) {
        ledstring.channel[0].leds[i] = COLOR_OFF;
    }
    ws2811_render(&ledstring);

    // Clear VFD and show initial message
    vfd_clear(vfd);
    vfd_write(vfd, "Button Test Ready!\nWaiting for input...\n");

    // Print current GPIO states
    printf("Initial GPIO states:\n");
    printf("GPIO 22 (pin 16): %d\n", gpioRead(MODE_BUTTON_PIN));
    printf("GPIO 6 (pin 31): %d\n", gpioRead(NEW_BUTTON_PIN));

    printf("Button test program running. Press Ctrl+C to exit.\n");
    printf("Monitoring buttons on GPIO 22 and GPIO 6...\n");

    // Main loop with GPIO state monitoring
    while (1) {
        sleep(1);
        // Print GPIO states every second for debugging
        printf("GPIO States - 22: %d, 6: %d\n", 
               gpioRead(MODE_BUTTON_PIN),
               gpioRead(NEW_BUTTON_PIN));
    }

    cleanup(0);
    return 0;
}