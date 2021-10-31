//
//  gpio_ctrl.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include "gpio_ctrl.h"

struct gpiod_chip *chip = NULL;

void init_gpio_ctrl(void)
{
    // Open GPIO chip
    if (chip == NULL) chip = gpiod_chip_open_by_name("gpiochip0");
}

void close_gpio_ctrl(void)
{
    
}

