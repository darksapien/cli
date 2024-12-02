//===============================
// FILE: vfd_adc.h
//===============================
#ifndef VFD_ADC_H
#define VFD_ADC_H

#include <stdint.h>
#include <pigpio.h>

// MCP3008 Pin Definitions
#define ADC_CS_PIN   8   // Physical Pin 24
#define ADC_DIN_PIN  10  // Physical Pin 19
#define ADC_DOUT_PIN 9   // Physical Pin 21
#define ADC_CLK_PIN  11  // Physical Pin 23

// Channel Definitions
#define ADC_CH_ROTARY   0
#define ADC_CH_SLIDER   1
#define ADC_CH_JOY_Y    2
#define ADC_CH_JOY_X    3
#define ADC_CH_JOY_NC   4
#define ADC_CH_POT_PW1  5
#define ADC_CH_GND1     6
#define ADC_CH_GND2     7

// ADC Control structure must be defined before it's used
typedef struct {
    int cs_pin;
    int din_pin;
    int dout_pin;
    int clk_pin;
    int initialized;
    
    // Cached analog values
    float rotary_position;
    float slider_position;
    float joystick_x;
    float joystick_y;
    
    // Change flags
    volatile int rotary_changed;
    volatile int slider_changed;
    volatile int joystick_changed;
} ADC_Control;

// Function declarations
ADC_Control* adc_init(void);
void adc_cleanup(ADC_Control* adc);
uint16_t adc_read_channel(ADC_Control* adc, uint8_t channel);
float adc_read_voltage(ADC_Control* adc, uint8_t channel);
float read_rotary_position(ADC_Control* adc);
float read_slider_position(ADC_Control* adc);
void read_joystick_position(ADC_Control* adc, float* x, float* y);

#endif