import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)  # Use BCM GPIO numbers
GPIO.setwarnings(False)

# Set up GPIO 6 and 22
GPIO.setup(6, GPIO.IN, pull_up_down=GPIO.PUD_UP)
GPIO.setup(22, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

print("Starting GPIO test...")
print("Press Ctrl+C to exit")

try:
    while True:
        gpio6_state = GPIO.input(6)
        gpio22_state = GPIO.input(22)
        print(f"GPIO 6: {gpio6_state}, GPIO 22: {gpio22_state}")
        time.sleep(1)

except KeyboardInterrupt:
    print("\nExiting...")
finally:
    GPIO.cleanup()