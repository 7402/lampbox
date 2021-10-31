//
//  parse_web.h
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#ifndef parse_web_h
#define parse_web_h

#include <stdbool.h>

void init_parse_web(void);
void close_parse_web(void);

bool read_web(const char *url);

char *get_weather_page(void);
char *get_air_quality_page(void);

int get_PM2p5(void);

struct WeatherReport {
    time_t as_of;
    char *current_condition;
    double current_temp;
    double current_humidity;
    char *next_period_name;
    char *next_period_forecast;
};
typedef struct WeatherReport WeatherReport;

WeatherReport *read_weather(void);

#endif /* parse_web_h */
