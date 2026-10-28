# Door Sensor - CS370 Term Project

## My Part

For our fridge monitor project, I am working on the magnetic door sensor. My program is written in C and uses the Raspberry Pi GPIO pins to check if the fridge door is open or closed.

Nailah is working on the temperature sensor and will test both sensors on the Raspberry Pi.

## How It Works

I use a magnetic reed switch to detect the door position. My program checks the sensor every 50 milliseconds.

I also check for three matching readings before reporting a change. This helps avoid reporting a door change because of a short signal bounce.

The program prints the date, time, and door status.

## Requirements

- Raspberry Pi
- Magnetic reed switch
- GCC compiler
- libgpiod version 2.x

## Compile

First install the required packages:

sudo apt update
sudo apt install -y gcc make pkg-config libgpiod-dev gpiod

Check the library version:

pkg-config --modversion libgpiod

The code requires libgpiod 2.x.

Compile the program:

make -f Makefile.door

## Run

After confirming the correct GPIO chip and line, run:

./door_sensor /dev/gpiochip0 17

The chip and line above are only examples. They must match the actual Raspberry Pi wiring.

## Testing

We will test the sensor by moving the magnet close to and away from the switch.

The expected results are:

- Magnet close: CLOSED
- Magnet away: OPEN

We will save the real terminal output after testing.

## Next Step

We will connect the door sensor and temperature sensor to the same monitoring pipeline and save their readings to storage.
