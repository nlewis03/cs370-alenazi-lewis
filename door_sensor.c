#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <gpiod.h>

static volatile sig_atomic_t running = 1;

void stop_program(int signal_number)
{
    (void)signal_number;
    running = 0;
}

void print_reading(int value)
{
    time_t now = time(NULL);
    struct tm *current = localtime(&now);

    if (current == NULL) {
        return;
    }

    char timestamp[32];

    strftime(timestamp, sizeof(timestamp),
             "%Y-%m-%d %H:%M:%S", current);

    if (value == GPIOD_LINE_VALUE_ACTIVE) {
        printf("%s Door: CLOSED\n", timestamp);
    } else {
        printf("%s Door: OPEN\n", timestamp);
    }

    fflush(stdout);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr,
                "Usage: %s /dev/gpiochip0 17\n",
                argv[0]);
        return 1;
    }

    const char *chip_path = argv[1];
    unsigned int pin = (unsigned int)atoi(argv[2]);

    signal(SIGINT, stop_program);
    signal(SIGTERM, stop_program);

    struct gpiod_chip *chip =
        gpiod_chip_open(chip_path);

    if (chip == NULL) {
        perror("Cannot open GPIO chip");
        return 1;
    }

    struct gpiod_line_settings *settings =
        gpiod_line_settings_new();

    struct gpiod_line_config *config =
        gpiod_line_config_new();

    struct gpiod_request_config *request_config =
        gpiod_request_config_new();

    if (settings == NULL || config == NULL ||
        request_config == NULL) {
        fprintf(stderr, "GPIO setup failed\n");
        goto error;
    }

    gpiod_line_settings_set_direction(
        settings, GPIOD_LINE_DIRECTION_INPUT);

    gpiod_line_settings_set_bias(
        settings, GPIOD_LINE_BIAS_PULL_UP);

    gpiod_line_settings_set_active_low(settings, true);

    if (gpiod_line_config_add_line_settings(
            config, &pin, 1, settings) < 0) {
        fprintf(stderr, "Line configuration failed\n");
        goto error;
    }

    gpiod_request_config_set_consumer(
        request_config, "fridge-door");

    struct gpiod_line_request *request =
        gpiod_chip_request_lines(
            chip, request_config, config);

    if (request == NULL) {
        perror("Cannot request GPIO line");
        goto error;
    }

    int previous = -1;
    int stable = -1;
    int count = 0;

    printf("Door sensor started\n");

    while (running) {
        enum gpiod_line_value value =
            gpiod_line_request_get_value(request, pin);

        if (value == GPIOD_LINE_VALUE_ERROR) {
            fprintf(stderr, "Sensor read failed\n");
            break;
        }

        if ((int)value == previous) {
            count++;
        } else {
            previous = (int)value;
            count = 1;
        }

        if (count >= 3 && stable != (int)value) {
            stable = (int)value;
            print_reading(stable);
        }

        usleep(50000);
    }

    gpiod_line_request_release(request);
    gpiod_request_config_free(request_config);
    gpiod_line_config_free(config);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);

    printf("Door sensor stopped\n");
    return 0;

error:
    if (request_config != NULL)
        gpiod_request_config_free(request_config);

    if (config != NULL)
        gpiod_line_config_free(config);

    if (settings != NULL)
        gpiod_line_settings_free(settings);

    gpiod_chip_close(chip);
    return 1;
}
