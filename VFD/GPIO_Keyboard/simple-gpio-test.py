from gpiozero import Button
from signal import pause
from time import sleep

def button_pressed():
    print("Button was pressed!")

def button_released():
    print("Button was released!")

# Create a button on GPIO6
button = Button(6)

# Attach the callbacks
button.when_pressed = button_pressed
button.when_released = button_released

print("Starting GPIO test...")
print("Press Ctrl+C to exit")

try:
    while True:
        print(f"Current button state: {button.value}")
        sleep(1)
except KeyboardInterrupt:
    print("\nExiting...")