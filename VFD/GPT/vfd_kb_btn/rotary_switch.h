#ifndef ROTARY_SWITCH_H
#define ROTARY_SWITCH_H

int read_rotary_value(int channel);    // Read raw rotary value
int get_rotary_position(int value);   // Map rotary value to a position

#endif // ROTARY_SWITCH_H

