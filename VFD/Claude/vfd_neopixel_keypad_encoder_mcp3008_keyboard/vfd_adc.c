//===============================
// FILE: vfd_adc.c
//===============================
#include "vfd_adc.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

ADC_Control* adc_init(void) {
    printf("ADC init: Starting initialization...\n");
    
    ADC_Control* adc = malloc(sizeof(ADC_Control));
    if (!adc) {
        printf("ADC init: Failed to allocate memory\n");
        return NULL;
    }

    // Initialize pin assignments
    adc->cs_pin = ADC_CS_PIN;
    adc->din_pin = ADC_DIN_PIN;
    adc->dout_pin = ADC_DOUT_PIN;
    adc->clk_pin = ADC_CLK_PIN;
    
    // Set up GPIO pins
    if (gpioSetMode(adc->cs_pin, PI_OUTPUT) != 0 ||
        gpioSetMode(adc->din_pin, PI_OUTPUT) != 0 ||
        gpioSetMode(adc->dout_pin, PI_INPUT) != 0 ||
        gpioSetMode(adc->clk_pin, PI_OUTPUT) != 0) {
        printf("ADC init: Failed to set GPIO modes\n");
        free(adc);
        return NULL;
    }

    // Initialize pins to default states
    gpioWrite(adc->cs_pin, 1);   // CS high (inactive)
    gpioWrite(adc->clk_pin, 0);  // CLK low
    gpioWrite(adc->din_pin, 0);  // DIN low
    
    adc->initialized = 1;
    printf("ADC init: Initialization complete\n");
    return adc;
}

void adc_cleanup(ADC_Control* adc) {
    if (!adc) return;
    
    // Set all pins to inputs with pulldown
    gpioSetMode(adc->cs_pin, PI_INPUT);
    gpioSetMode(adc->din_pin, PI_INPUT);
    gpioSetMode(adc->dout_pin, PI_INPUT);
    gpioSetMode(adc->clk_pin, PI_INPUT);
    
    gpioSetPullUpDown(adc->cs_pin, PI_PUD_DOWN);
    gpioSetPullUpDown(adc->din_pin, PI_PUD_DOWN);
    gpioSetPullUpDown(adc->dout_pin, PI_PUD_DOWN);
    gpioSetPullUpDown(adc->clk_pin, PI_PUD_DOWN);
    
    free(adc);
}

uint16_t adc_read_channel(ADC_Control* adc, uint8_t channel) {
    if (!adc || !adc->initialized || channel > 7) return 0;

    uint16_t value = 0;
    static int debug_counter = 0;
    debug_counter++;
    bool debug = (debug_counter % 100 == 0);  // Debug every 100th read
    
    if (debug) printf("ADC: Reading channel %d\n", channel);
    
    // Start bit pattern for MCP3008: Start(1) + Single/Diff(1) + Channel(3)
    uint8_t command = 0b11000000 | (channel << 3);
    
    // Begin conversion
    gpioWrite(adc->cs_pin, 0);  // Select chip
    usleep(1);  // Short delay for setup
    
    // Send start bit and single/diff bit
    for (int i = 7; i >= 0; i--) {
        gpioWrite(adc->din_pin, (command >> i) & 1);
        usleep(1);
        gpioWrite(adc->clk_pin, 1);
        usleep(1);
        gpioWrite(adc->clk_pin, 0);
    }
    
    // Read null bit
    gpioWrite(adc->clk_pin, 1);
    usleep(1);
    gpioWrite(adc->clk_pin, 0);
    
    // Read 10 bits of data
    for (int i = 0; i < 10; i++) {
        gpioWrite(adc->clk_pin, 1);
        usleep(1);
        value = (value << 1) | gpioRead(adc->dout_pin);
        gpioWrite(adc->clk_pin, 0);
        usleep(1);
    }
    
    gpioWrite(adc->cs_pin, 1);  // Deselect chip
    
    if (debug) printf("ADC: Channel %d raw value: %d\n", channel, value);
    return value;
}

float adc_read_voltage(ADC_Control* adc, uint8_t channel) {
    uint16_t raw = adc_read_channel(adc, channel);
    float voltage = (raw * 3.3f) / 1023.0f;
    static int debug_counter = 0;
    debug_counter++;
    if (debug_counter % 100 == 0) {
        printf("ADC: Channel %d voltage: %.3fV (raw: %d)\n", channel, voltage, raw);
    }
    return voltage;
}

float read_rotary_position(ADC_Control* adc) {
    return adc_read_voltage(adc, ADC_CH_ROTARY);
}

float read_slider_position(ADC_Control* adc) {
    return adc_read_voltage(adc, ADC_CH_SLIDER);
}

void read_joystick_position(ADC_Control* adc, float* x, float* y) {
    if (!x || !y) return;
    
    *x = adc_read_voltage(adc, ADC_CH_JOY_X);
    *y = adc_read_voltage(adc, ADC_CH_JOY_Y);
}