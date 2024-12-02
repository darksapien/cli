#include "rotary_switch.h"
#include <pigpio.h>

// Read the raw rotary switch value from MCP3008
int read_rotary_value(int channel) {
    char tx_buffer[3] = {1, (8 + channel) << 4, 0};
    char rx_buffer[3];
    spiXfer(0, tx_buffer, rx_buffer, 3);
    return ((rx_buffer[1] & 3) << 8) | rx_buffer[2]; // 10-bit result
}

// Map the raw value to a position
int get_rotary_position(int value) {
    if (value >= 934) return 0;
    if (value >= 768) return 1;
    if (value >= 597) return 2;
    if (value >= 426) return 3;
    if (value >= 256) return 4;
    if (value >= 85) return 5;
    return 6;
}

