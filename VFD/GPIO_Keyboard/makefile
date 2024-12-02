CC=gcc
CFLAGS=-Wall -O2
LDFLAGS=-lpigpio -lws2811 -lm
TARGET=button_test
SOURCES=button_test.c

$(TARGET): $(SOURCES)
	$(CC) $(SOURCES) -o $(TARGET) $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET)