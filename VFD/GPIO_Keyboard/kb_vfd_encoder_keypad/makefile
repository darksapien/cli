CC = gcc
CFLAGS = -Wall -Wextra
LDFLAGS = -lpigpio -lrt -lws2811

# Source files
SOURCES = main.c \
          vfd_display.c \
          vfd_gpio.c \
          vfd_keypad.c

# Header files (for dependencies)
HEADERS = vfd_display.h \
          vfd_gpio.h \
          vfd_keyboard.h \
          vfd_keypad.h

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executable name
TARGET = vfd_controller

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
