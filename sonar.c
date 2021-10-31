//
//  sonar.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

#include "sonar.h"
#include "gpio_ctrl.h"

struct gpiod_line *lineTrigger = NULL;   // 17
struct gpiod_line *lineEcho = NULL;      // 27

const unsigned long long MSEC = 1000LL; // 1 msec in usec

void init_sonar(void)
{
    // Open GPIO chip
    init_gpio_ctrl();

    // Open GPIO lines
    lineTrigger = gpiod_chip_get_line(chip, 17);
    lineEcho = gpiod_chip_get_line(chip, 27);

    // Open line for output
    gpiod_line_request_output(lineTrigger, "lampbox", 0);
    gpiod_line_set_value(lineTrigger, 0);
    usleep(100 * MSEC);

    // Open line for input
    gpiod_line_request_input(lineEcho, "lampbox");
}

void close_sonar(void)
{
    close_gpio_ctrl();
    lineTrigger = NULL;
    lineEcho = NULL;
}

double get_distance(void)
{
    const unsigned long long TIMEOUT = 1000LL * MSEC;
    
    gpiod_line_set_value(lineTrigger, 1);
    unsigned long long trigger_start = get_usec();
    loop_sleep(10);
    gpiod_line_set_value(lineTrigger, 0);
    
    unsigned long long delay = get_usec() - trigger_start;

    int echo;
    unsigned long long echo_start;
    
    do {
        echo = gpiod_line_get_value(lineEcho);
        echo_start = get_usec();
        delay = echo_start - trigger_start;

    } while (delay < TIMEOUT && echo == 0);
    
    if (delay < TIMEOUT) {
        do {
            echo = gpiod_line_get_value(lineEcho);
            delay = get_usec() - echo_start;

        } while (delay < TIMEOUT && echo == 1);
        
    } else {
        fprintf(stderr, "wait timeout\n");
        return -1.0;
    }

    if (delay >= 1000 * MSEC) {
        fprintf(stderr, "timeout\n");
        return -1.0;

    } else {
        double distance = delay * 0.017;
        // fprintf(stderr, "%ld echo length %6.1f cm\n", delay, distance);
        return distance;
    }

}

unsigned long long get_usec(void)
{
    struct timespec now;

    clock_gettime(CLOCK_BOOTTIME, &now);
    return now.tv_nsec / 1000LL + now.tv_sec * 1000000LL;
}

void loop_sleep(unsigned long long usec)
{
    unsigned long long start = get_usec();
    while (get_usec() - start < usec) ;
}
