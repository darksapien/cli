#ifndef NEOPIXEL_CONTROL_H
#define NEOPIXEL_CONTROL_H

void init_neopixel(int led_count, int gpio_pin);   // Initialize NeoPixel
void update_neopixel(int position);               // Update NeoPixel to reflect rotary position
void cleanup_neopixel();                          // Cleanup NeoPixel resources

#endif // NEOPIXEL_CONTROL_H

