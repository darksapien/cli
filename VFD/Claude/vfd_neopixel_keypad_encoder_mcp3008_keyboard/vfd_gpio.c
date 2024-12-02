//===============================
// FILE: vfd_gpio.c
//===============================
#include "vfd_gpio.h"
#include <unistd.h>

static GPIO_Control* g_gpio_control = NULL;

void button_callback(int gpio_pin, int level, uint32_t tick) {
    if (!g_gpio_control) return;
    
    if (tick - g_gpio_control->button_debounce < 200000) return;
    g_gpio_control->button_debounce = tick;
    
    if (level == 1) {
        g_gpio_control->current_mode = (g_gpio_control->current_mode + 1) % MODE_COUNT;
        g_gpio_control->mode_changed = 1;
        update_neopixel_mode(g_gpio_control);
    }
}

void encoder_callback_a(int gpio, int level, uint32_t tick) {
    printf("Encoder A: Pin=%d Level=%d\n", gpio, level);
    
    if (!g_gpio_control) {
        printf("Error: g_gpio_control is NULL in encoder_callback_a\n");
        return;
    }
    
    int a = level;
    int b = gpioRead(ENCODER_B_PIN);
    
    printf("Encoder states: A=%d B=%d\n", a, b);
    
    // Only process on rising edge of A
    if (a == 1 && g_gpio_control->last_encoder_a == 0) {
        // If B is high when A rises, counter-clockwise rotation (increase)
        if (b == 1) {
            g_gpio_control->encoder_value += ENCODER_STEP_SIZE;
            printf("Encoder value increased to: %d (CCW)\n", g_gpio_control->encoder_value);
        } else {
            g_gpio_control->encoder_value -= ENCODER_STEP_SIZE;
            printf("Encoder value decreased to: %d (CW)\n", g_gpio_control->encoder_value);
        }
        g_gpio_control->encoder_changed = 1;
    }
    
    g_gpio_control->last_encoder_a = a;
}

void encoder_callback_b(int gpio, int level, uint32_t tick) {
    printf("Encoder B: Pin=%d Level=%d\n", gpio, level);
    
    if (!g_gpio_control) {
        printf("Error: g_gpio_control is NULL in encoder_callback_b\n");
        return;
    }
    
    g_gpio_control->last_encoder_b = level;
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

    for (int i = 0; i < LED_COUNT; i++) {
        gpio->ws2811.channel[0].leds[i] = color;
    }
    ws2811_render(&gpio->ws2811);
}

GPIO_Control* gpio_init(void) {
    printf("Starting GPIO initialization...\n");
    
    GPIO_Control* gpio = malloc(sizeof(GPIO_Control));
    if (!gpio) {
        printf("Failed to allocate GPIO_Control\n");
        return NULL;
    }

    g_gpio_control = gpio;

    if (gpioInitialise() < 0) {
        printf("Failed to initialize pigpio\n");
        free(gpio);
        return NULL;
    }

    // Initialize button pin
    if (gpioSetMode(MODE_BUTTON_PIN, PI_INPUT) != 0 ||
        gpioSetPullUpDown(MODE_BUTTON_PIN, PI_PUD_DOWN) != 0) {
        printf("Failed to initialize button pin\n");
        gpioTerminate();
        free(gpio);
        return NULL;
    }

    // Initialize encoder pins
    if (gpioSetMode(ENCODER_A_PIN, PI_INPUT) != 0 ||
        gpioSetMode(ENCODER_B_PIN, PI_INPUT) != 0 ||
        gpioSetPullUpDown(ENCODER_A_PIN, PI_PUD_UP) != 0 ||
        gpioSetPullUpDown(ENCODER_B_PIN, PI_PUD_UP) != 0) {
        printf("Failed to initialize encoder pins\n");
        gpioTerminate();
        free(gpio);
        return NULL;
    }

    // Initialize NeoPixel
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

// Initialize keypad
    printf("Initializing keypad...\n");
    gpio->keypad = keypad_init();
    if (!gpio->keypad) {
        printf("Failed to initialize keypad\n");
        ws2811_fini(&gpio->ws2811);
        gpioTerminate();
        free(gpio);
        return NULL;
    }
    printf("Keypad initialized successfully\n");

    // Initialize ADC
    printf("Initializing ADC...\n");
    gpio->adc = adc_init();
    if (!gpio->adc) {
        printf("Failed to initialize ADC\n");
        keypad_cleanup(gpio->keypad);
        ws2811_fini(&gpio->ws2811);
        gpioTerminate();
        free(gpio);
        return NULL;
    }
    printf("ADC initialized successfully\n");

    // Initialize ADC
    printf("Initializing ADC...\n");
    gpio->adc = adc_init();
    if (!gpio->adc) {
        printf("Failed to initialize ADC\n");
        keypad_cleanup(gpio->keypad);
        ws2811_fini(&gpio->ws2811);
        gpioTerminate();
        free(gpio);
        return NULL;
    }
    printf("ADC initialized successfully\n");


    // Initialize state variables
    gpio->current_mode = MODE_SSH;
    gpio->button_debounce = 0;
    gpio->mode_changed = 0;
    gpio->encoder_value = 0;
    gpio->last_encoder_a = gpioRead(ENCODER_A_PIN);
    gpio->last_encoder_b = gpioRead(ENCODER_B_PIN);
    gpio->encoder_changed = 0;

    // Set up callbacks
    if (gpioSetAlertFunc(MODE_BUTTON_PIN, button_callback) != 0 ||
        gpioSetAlertFunc(ENCODER_A_PIN, encoder_callback_a) != 0 ||
        gpioSetAlertFunc(ENCODER_B_PIN, encoder_callback_b) != 0) {
        printf("Failed to set up callbacks\n");
        if (gpio->keypad) keypad_cleanup(gpio->keypad);
        ws2811_fini(&gpio->ws2811);
        gpioTerminate();
        free(gpio);
        return NULL;
    }

    // Set up glitch filters
    gpioGlitchFilter(MODE_BUTTON_PIN, 100000);
    gpioGlitchFilter(ENCODER_A_PIN, 100);
    gpioGlitchFilter(ENCODER_B_PIN, 100);

    update_neopixel_mode(gpio);
    return gpio;
}


void gpio_cleanup(GPIO_Control* gpio) {
    if (!gpio) return;
    
    // Clean up keypad first if it exists
    if (gpio->keypad) {
        keypad_cleanup(gpio->keypad);
        gpio->keypad = NULL;
    }

    // Clean up ADC if it exists
    if (gpio->adc) {
        adc_cleanup(gpio->adc);
        gpio->adc = NULL;
    }
    
    // Clean up NeoPixel
    for (int i = 0; i < LED_COUNT; i++) {
        gpio->ws2811.channel[0].leds[i] = COLOR_OFF;
    }
    ws2811_render(&gpio->ws2811);
    ws2811_fini(&gpio->ws2811);
    
    // Clean up GPIO pins
    gpioSetMode(MODE_BUTTON_PIN, PI_INPUT);
    gpioSetMode(ENCODER_A_PIN, PI_INPUT);
    gpioSetMode(ENCODER_B_PIN, PI_INPUT);
    gpioSetPullUpDown(MODE_BUTTON_PIN, PI_PUD_DOWN);
    gpioSetPullUpDown(ENCODER_A_PIN, PI_PUD_DOWN);
    gpioSetPullUpDown(ENCODER_B_PIN, PI_PUD_DOWN);
    
    gpioTerminate();
    g_gpio_control = NULL;
    free(gpio);
}


void show_encoder_value(VFD_Display* vfd, int value) {
    if (!vfd) return;
    
    static int last_value = 0;  // Keep track of last value locally
    printf("Displaying encoder value: %d (last: %d)\n", value, last_value);
    
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Encoder Value: %d\n", value);
    vfd_write(vfd, buffer);
    
    // Add direction indicator based on value change
    if (value < last_value) {
        vfd_write(vfd, "Direction: Clockwise\n");
        printf("Direction display: Clockwise\n");
    } else if (value > last_value) {
        vfd_write(vfd, "Direction: Counter-Clockwise\n");
        printf("Direction display: Counter-Clockwise\n");
    }
    
    last_value = value;  // Update last value for next comparison
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
    
    usleep(2000000);  // 2 second delay
    vfd_clear(vfd);
}
