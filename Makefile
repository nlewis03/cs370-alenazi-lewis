CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Werror -D_DEFAULT_SOURCE

# Door sensor needs libgpiod; temp sensor (I2C/1-Wire) does not.
DOOR_LDLIBS = -lgpiod
TEMP_LDLIBS =

.PHONY: all clean

all: door_sensor temp_sensor

door_sensor: src/capture/door_sensor.c
	$(CC) $(CFLAGS) -o $@ $< $(DOOR_LDLIBS)

temp_sensor: src/capture/temp.c
	$(CC) $(CFLAGS) -o $@ $< $(TEMP_LDLIBS)

clean:
	rm -f door_sensor temp_sensor
