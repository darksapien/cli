#include "neopixel_control.h"
#include <ws2811.h>

static ws2811_t ledstrip;

void init_neopixel(int led_count, int gpio_pin) {
    ledstrip.freq = WS2811_TARGET_FREQ;
    ledstrip.dmanum = 10;
    ledstrip.channel[0].gpionum = gpio_pin;
    ledstrip.channel[0].count = led_count;
    ledstrip.channel[0].strip_type = WS2811_STRIP_GRB;
    ledstrip.channel[0].brightness = 255;

    if (ws2811_init(&ledstrip) != WS2811_SUCCESS) {
        fprintf(stderr, "Failed to initialize NeoPixel\n");
        exit(1);
    }
}

void update_neopixel(int position) {
    // Clear all LEDs
    for (int i = 0; i < ledstrip.channel[0].count; i++) {
        ledstrip.channel[0].leds[i] = 0;
    }

    // Set the LED at the rotary position to green
    int led_index = (position * ledstrip.channel[0].count) / 7;
    ledstrip.channel[0].leds[led_index] = 0x00FF00; // Green

    ws2811_render(&ledstrip);
}

void cleanup_neopixel() {
    ws2811_fini(&ledstrip);
}

