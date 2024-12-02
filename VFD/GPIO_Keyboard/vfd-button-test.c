#include <pigpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <rpi_ws281x/ws2811.h>
#include "vfd_display.h"

// NeoPixel configuration
#define LED_COUNT 12         // 12-LED NeoPixel ring
#define LED_FREQ  800000     // LED frequency
#define LED_DMA   10         // DMA channel
#define LED_INVERT 0         // Don't invert signal
#define LED_CHANNEL 0        // PWM channel
#define LED_STRIP WS2811_STRIP_GRB

// Color definitions
#define COLOR_ON     0x00FF00    // Green
#define COLOR_OFF    0x000000    // Off

// Button configuration structure
typedef struct {
    int gpio_pin;       // GPIO pin number
    int active_state;   // Active state: 1 for HIGH, 0 for LOW
    int pull;           // Pull-up/down configuration
    const char *name;   // Button name
} ButtonConfig;

// Global variables
ws2811_t ledstring = {
    .freq = LED_FREQ,
    .dmanum = LED_DMA,
    .channel = {
        [0] = {
            .gpionum = 18,  // Default NeoPixel pin
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

// Button configuration array
ButtonConfig buttons[] = {
    {24, 1, PI_PUD_DOWN, "Mode Button"},  // Active HIGH
    {6, 1, PI_PUD_DOWN, "New Button"}, // Active HIGH
};

const int button_count = sizeof(buttons) / sizeof(buttons[0]);

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
        vfd_close(vfd);
    }

    // Cleanup pigpio
    gpioTerminate();
    exit(signum);
}

void button_callback(int gpio_pin, int level, uint32_t tick) {
    static uint32_t last_tick = 0;

    // Simple debounce (200ms)
    if (tick - last_tick < 200000) return;
    last_tick = tick;

    for (int i = 0; i < button_count; i++) {
        if (gpio_pin == buttons[i].gpio_pin) {
            if (level == buttons[i].active_state) {
                printf("%s PRESSED at tick %u\n", buttons[i].name, tick);
                if (vfd) {
                    vfd_clear(vfd);
                    vfd_write(vfd, "Button PRESSED!\n");
                }
                for (int j = 0; j < LED_COUNT; j++) {
                    ledstring.channel[0].leds[j] = 0x0000FF;  // Blue
                }
            } else {
                printf("%s RELEASED at tick %u\n", buttons[i].name, tick);
                if (vfd) {
                    vfd_clear(vfd);
                    vfd_write(vfd, "Button RELEASED!\n");
                }
                for (int j = 0; j < LED_COUNT; j++) {
                    ledstring.channel[0].leds[j] = COLOR_OFF;
                }
            }
            ws2811_render(&ledstring);
            break;
        }
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

    // Configure buttons
    for (int i = 0; i < button_count; i++) {
        if (gpioSetMode(buttons[i].gpio_pin, PI_INPUT) != 0 ||
            gpioSetPullUpDown(buttons[i].gpio_pin, buttons[i].pull) != 0) {
            fprintf(stderr, "Failed to configure %s\n", buttons[i].name);
            cleanup(1);
            return 1;
        }
        if (gpioSetAlertFunc(buttons[i].gpio_pin, button_callback) != 0) {
            fprintf(stderr, "Failed to set alert function for %s\n", buttons[i].name);
            cleanup(1);
            return 1;
        }
    }

    // Turn off all NeoPixels initially
    for (int i = 0; i < LED_COUNT; i++) {
        ledstring.channel[0].leds[i] = COLOR_OFF;
    }
    ws2811_render(&ledstring);

    // Clear VFD and show initial message
    vfd_clear(vfd);
    vfd_write(vfd, "Button Test Ready!\nWaiting for input...\n");

    printf("Button test program running. Press Ctrl+C to exit.\n");

    // Main loop
    while (1) {
        sleep(1);
    }

    cleanup(0);
    return 0;
}

