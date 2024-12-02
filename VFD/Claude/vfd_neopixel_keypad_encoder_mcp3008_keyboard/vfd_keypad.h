//===============================
// FILE: vfd_keypad.h
//===============================
#ifndef VFD_KEYPAD_H
#define VFD_KEYPAD_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define I2C_DEVICE "/dev/i2c-1"
#define I2C_ADDR 0x25

typedef struct {
    int file;
    volatile int key_pressed;
    volatile char last_key;
} Keypad_Control;

// Function declarations
Keypad_Control* keypad_init(void);
void keypad_cleanup(Keypad_Control* keypad);
void read_keypad(Keypad_Control* keypad);
const char* get_key_name(char key);

#endif