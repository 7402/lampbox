//
//  gpio_ctrl.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef gpio_ctrl_h
#define gpio_ctrl_h

#include <gpiod.h>

extern struct gpiod_chip *chip;

void init_gpio_ctrl(void);
void close_gpio_ctrl(void);

#endif /* gpio_ctrl_h */
