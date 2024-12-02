//===============================
// FILE: vfd_gpio.c
//===============================
#include "vfd_gpio.h"
#include <unistd.h>  // For usleep

// Global pointer for callback access
static GPIO_Control* g_gpio_control = NULL;

void button_callback(int gpio_pin, int level, uint32_t tick) {
    printf("Callback triggered: pin=%d, level=%d\n", gpio_pin, level);
    
    if (!g_gpio_control) {
        printf("Error: g_gpio_control is NULL!\n");
        return;
    }
    
    // Simple debounce
    if (tick - g_gpio_control->button_debounce < 200000) {
        printf("Debounce skip\n");
        return;
    }
    g_gpio_control->button_debounce = tick;
    
    if (level == 1) { // Button pressed (active high)
        printf("Button press detected, changing mode from %d to %d\n", 
               g_gpio_control->current_mode, 
               (g_gpio_control->current_mode + 1) % MODE_COUNT);
               
        g_gpio_control->current_mode = (g_gpio_control->current_mode + 1) % MODE_COUNT;
        g_gpio_control->mode_changed = 1;
        update_neopixel_mode(g_gpio_control);
    }
}

void update_neopixel_mode(GPIO_Control* gpio) {
    if (!gpio) return;

    uint32_t color;
    switch (gpio->current_mode) {
        case MODE_SSH:
            color = COLOR_SSH;
            break;
        case MODE_LOCAL_KB:
            color = COLOR_LOCAL;
            break;
        case MODE_AUTO:
            color = COLOR_AUTO;
            break;
        default:
            color = COLOR_OFF;
            break;
    }

    // Update all LEDs to show current mode
    for (int i = 0; i < LED_COUNT; i++) {
        gpio->ws2811.channel[0].leds[i] = color;
    }
    ws2811_render(&gpio->ws2811);
}

GPIO_Control* gpio_init(void) {
    printf("Starting GPIO initialization...\n");
    
    GPIO_Control* gpio = malloc(sizeof(GPIO_Control));
    if (!gpio) {
        printf("Failed to allocate GPIO structure\n");
        return NULL;
    }

    printf("Setting up global reference...\n");
    // Store global reference for callback
    g_gpio_control = gpio;

    // Initialize pigpio without daemon
    printf("Initializing pigpio...\n");
    gpioCfgSetInternals(1<<10);  // Configure for non-daemon operation
    int init_result = gpioInitialise();
    if (init_result < 0) {
        printf("Failed to initialize pigpio: %d\n", init_result);
        free(gpio);
        return NULL;
    }

    // Initialize button pin (GPIO 22 = Physical pin 16)
    printf("Setting up button pin...\n");
    if (gpioSetMode(MODE_BUTTON_PIN, PI_INPUT) != 0) {
        printf("Failed to set pin mode\n");
        gpioTerminate();
        free(gpio);
        return NULL;
    }
    
    if (gpioSetPullUpDown(MODE_BUTTON_PIN, PI_PUD_DOWN) != 0) {
        printf("Failed to set pull-down resistor\n");
        gpioTerminate();
        free(gpio);
        return NULL;
    }

    // Initialize NeoPixel
    printf("Initializing NeoPixel...\n");
    memset(&gpio->ws2811, 0, sizeof(ws2811_t));
    gpio->ws2811.freq = LED_FREQ;
    gpio->ws2811.dmanum = LED_DMA;
    gpio->ws2811.channel[0].gpionum = NEOPIXEL_PIN;
    gpio->ws2811.channel[0].count = LED_COUNT;
    gpio->ws2811.channel[0].invert = LED_INVERT;
    gpio->ws2811.channel[0].brightness = 128;
    gpio->ws2811.channel[0].strip_type = LED_STRIP;

    if (ws2811_init(&gpio->ws2811) != WS2811_SUCCESS) {
        printf("Failed to initialize WS2811\n");
        gpioTerminate();
        free(gpio);
        return NULL;
    }

    gpio->current_mode = MODE_SSH;
    gpio->button_debounce = 0;
    gpio->mode_changed = 0;

    // Set up GPIO monitoring for button
    printf("Setting up GPIO monitoring...\n");
    int alert_result = gpioSetAlertFunc(MODE_BUTTON_PIN, button_callback);
    if (alert_result != 0) {
        printf("Failed to set alert function: %d\n", alert_result);
        gpioTerminate();
        free(gpio);
        return NULL;
    }

    // Enable alert for pin changes
    printf("Enabling GPIO alerts...\n");
    if (gpioSetPullUpDown(MODE_BUTTON_PIN, PI_PUD_DOWN) != 0 ||
        gpioGlitchFilter(MODE_BUTTON_PIN, 100000) != 0 ||  // 100ms glitch filter
        gpioSetWatchdog(MODE_BUTTON_PIN, 0) != 0) {        // Disable watchdog
        printf("Failed to configure GPIO monitoring\n");
        gpioTerminate();
        free(gpio);
        return NULL;
    }

    // Initial NeoPixel update
    printf("Updating NeoPixel initial state...\n");
    update_neopixel_mode(gpio);
    
    printf("GPIO initialization complete\n");
    return gpio;
}

void gpio_cleanup(GPIO_Control* gpio) {
    if (!gpio) return;
    
    // Clear NeoPixels
    for (int i = 0; i < LED_COUNT; i++) {
        gpio->ws2811.channel[0].leds[i] = COLOR_OFF;
    }
    ws2811_render(&gpio->ws2811);
    ws2811_fini(&gpio->ws2811);
    
    // Cleanup GPIO
    gpioSetMode(MODE_BUTTON_PIN, PI_INPUT);
    gpioSetPullUpDown(MODE_BUTTON_PIN, PI_PUD_DOWN);  // Keep pull-down on cleanup
    gpioTerminate();
    
    g_gpio_control = NULL;
    free(gpio);
}

const char* get_mode_name(VFD_Mode mode) {
    switch (mode) {
        case MODE_SSH:
            return "SSH Keyboard Input";
        case MODE_LOCAL_KB:
            return "Local Keyboard Input";
        case MODE_AUTO:
            return "Automatic Display";
        default:
            return "Unknown Mode";
    }
}

void show_mode_instructions(VFD_Display* vfd, VFD_Mode mode) {
    if (!vfd) return;
    
    vfd_clear(vfd);
    
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Mode: %s\n", get_mode_name(mode));
    vfd_write(vfd, buffer);
    
    switch (mode) {
        case MODE_SSH:
            vfd_write(vfd, "SSH keyboard input active\n");
            vfd_write(vfd, "Use terminal to type\n");
            break;
            
        case MODE_LOCAL_KB:
            vfd_write(vfd, "Local keyboard input active\n");
            vfd_write(vfd, "Connect USB keyboard to use\n");
            break;
            
        case MODE_AUTO:
            vfd_write(vfd, "Auto display mode active\n");
            vfd_write(vfd, "Messages will display automatically\n");
            break;
    }
    
    // Brief delay to show instructions
    usleep(2000000); // 2 second delay
    vfd_clear(vfd);
}