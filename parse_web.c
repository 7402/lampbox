//
//  parse_web.c
//  lampbox
//
// Copyright (C) 2021 Michael Budiansky. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <curl/curl.h>

#include "parse_web.h"

// this file contains my airnow API key, in the form:
// #define PRIVATE_API_KEY "00000000-0000-0000-0000-000000000000"
#include "../private_key.h"

// see https://everything.curl.dev/libcurl/examples/getinmem

struct WebBuffer {
  char *data;
  size_t size;
};
typedef struct WebBuffer WebBuffer;

WebBuffer web_buffer = {NULL, 0};

WeatherReport weather_report;

void init_parse_web(void)
{
    curl_global_init(CURL_GLOBAL_ALL);

    weather_report.as_of = 0;
    weather_report.current_condition = NULL;
    weather_report.current_temp = -999.0;
    weather_report.current_humidity = -1.0;
    weather_report.next_period_name = NULL;
    weather_report.next_period_forecast = NULL;
}

void close_parse_web(void)
{
    curl_global_cleanup();
    
    if (web_buffer.data != NULL) {
        free(web_buffer.data);
        web_buffer.data = NULL;
        web_buffer.size = 0;
    }
}

static size_t web_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t add_size = size * nmemb;
    WebBuffer *buffer = (WebBuffer *)userp;

    buffer->data = realloc(buffer->data, buffer->size + add_size);
    if (buffer->data == NULL) {
        fprintf(stderr, "out of memory)\n");
        exit(1);
    }
    
    memcpy(&(buffer->data[buffer->size - 1]), contents, add_size);
    buffer->size += add_size;
    buffer->data[buffer->size - 1] = '\0';

    return add_size;
}

bool read_web(const char *url)
{
    if (web_buffer.data != NULL) {
        free(web_buffer.data);
        web_buffer.data = NULL;
        web_buffer.size = 0;
    }
    
    if (web_buffer.data == NULL) {
        web_buffer.data = malloc(1);
        if (web_buffer.data == NULL) {
            fprintf(stderr, "out of memory)\n");
            exit(1);
        }
        
        web_buffer.data[0] = '\0';
        web_buffer.size = 1;
    }
    
    CURL *curl_handle = curl_easy_init();
    CURLcode result = curl_handle != NULL ? CURLE_OK : CURLE_FAILED_INIT;
    
    if (result == CURLE_OK) curl_easy_setopt(curl_handle, CURLOPT_URL, url);

    if (result == CURLE_OK) curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, web_callback);

    if (result == CURLE_OK) curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&web_buffer);

    if (result == CURLE_OK) curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    
    if (result == CURLE_OK) curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 8L);

    if (result == CURLE_OK) result = curl_easy_perform(curl_handle);

    if (result != CURLE_OK) {
        fprintf(stderr, "web fail %d\n", result);
    }

    if (curl_handle != NULL) curl_easy_cleanup(curl_handle);
    
    return result == CURLE_OK;
}

char *get_weather_page(void)
{
    char *url = "https://forecast.weather.gov/MapClick.php?lat=37.8895&lon=-122.295&unit=0&lg=english&FcstType=dwml";
    
    read_web(url);
    
    char *text = malloc(web_buffer.size);
    if (text != NULL) memcpy(text, web_buffer.data, web_buffer.size);
    
    return text;
}

char *get_air_quality_page(void)
{
    char *url = "https://www.airnowapi.org/aq/observation/zipCode/current/?format=text/csv&zipCode=94706&distance=15&API_KEY="
        PRIVATE_API_KEY;
    
    read_web(url);
    
    char *text = malloc(web_buffer.size);
    if (text != NULL) memcpy(text, web_buffer.data, web_buffer.size);
    
    return text;
}

int get_PM2p5(void)
{
    int value = -1.0;
    char *text = get_air_quality_page();
    
    if (text != NULL) {
        const char *tag = "PM2.5\",\"";
        char *p = strstr(text, tag);
        if (p != NULL) sscanf(p + strlen(tag), "%d", &value);
        
        free(text);
    }

    time_t now = time(NULL);

    struct tm* tm_now;
    tm_now = localtime(&now);

    char str[256];
    strftime(str, 26, "%Y-%m-%d %H:%M:%S", tm_now);

    fprintf(stderr, "\nair quality at %s = %d\n", str, value);

    return value;
}

char *alloc_string(char *p, size_t count);
char *alloc_string(char *p, size_t count)
{
    char *text = malloc(count + 1);
    if (text != NULL) {
        memcpy(text, p, count);
        text[count] = '\0';
    }

    return text;
}

WeatherReport *read_weather(void)
{
    const char *tag0 = "\"";
    const char *tag1 = "<data type=\"current observations\">";
    const char *tag2 = "<temperature type=\"apparent\"";
    const char *tag3 = "<value>";
    const char *tag4 = "<humidity type=\"relative\"";
    const char *tag5 = "<weather-conditions weather-summary=\"";
    const char *tag6 = "<start-valid-time period-name=\"";
    const char *tag7 = "<name>Text Forecast</name>";
    const char *tag8 = "<text>";
    const char *tag9 = "</text>";
    
    if (time(NULL) - weather_report.as_of > 10 * 60) {  // get new if older than 1 hr.
        char *text = get_weather_page();
        
        if (weather_report.current_condition != NULL) free(weather_report.current_condition);
        if (weather_report.next_period_name != NULL) free(weather_report.current_condition);
        if (weather_report.next_period_forecast != NULL) free(weather_report.current_condition);

        weather_report.as_of = time(NULL);
        weather_report.current_condition = NULL;
        weather_report.current_temp = -999.0;
        weather_report.current_humidity = -1.0;
        weather_report.next_period_name = NULL;
        weather_report.next_period_forecast = NULL;
        
        if (text != NULL) {
            char *p1 = NULL;
            char *p2 = NULL;
            char *p3 = NULL;

            // current_condition
            p1 = strstr(text, tag1);
            if (p1 != NULL) p2 = strstr(p1 + strlen(tag1), tag5);
            if (p2 != NULL) p3 = strstr(p2 + strlen(tag5), tag0);
            if (p3 != NULL) weather_report.current_condition =
                alloc_string(p2 + strlen(tag5), p3 - (p2 + strlen(tag5)));
            
            p1 = NULL;
            p2 = NULL;
            p3 = NULL;

            // current_temp
            p1 = strstr(text, tag1);
            if (p1 != NULL) p2 = strstr(p1 + strlen(tag1), tag2);
            if (p2 != NULL) p3 = strstr(p2 + strlen(tag2), tag3);
            if (p3 != NULL) sscanf(p3 + strlen(tag3), "%lf", &weather_report.current_temp);
            
            p1 = NULL;
            p2 = NULL;
            p3 = NULL;

            // current_humidity
            p1 = strstr(text, tag1);
            if (p1 != NULL) p2 = strstr(p1 + strlen(tag1), tag4);
            if (p2 != NULL) p3 = strstr(p2 + strlen(tag4), tag3);
            if (p3 != NULL) sscanf(p3 + strlen(tag3), "%lf", &weather_report.current_humidity);
            
            p1 = NULL;
            p2 = NULL;
            p3 = NULL;

            // next_period_name
            p1 = strstr(text, tag6);
            if (p1 != NULL) p2 = strstr(p1 + strlen(tag6), tag0);
            if (p2 != NULL) weather_report.next_period_name =
                alloc_string(p1 + strlen(tag6), p2 - (p1 + strlen(tag6)));
            
            p1 = NULL;
            p2 = NULL;
            p3 = NULL;

            // next_period_forecast
            p1 = strstr(text, tag7);
            if (p1 != NULL) p2 = strstr(p1 + strlen(tag7), tag8);
            if (p2 != NULL) p3 = strstr(p2 + strlen(tag8), tag9);
            if (p3 != NULL) weather_report.next_period_forecast =
                alloc_string(p2 + strlen(tag8), p3 - (p2 + strlen(tag8)));

            if (weather_report.current_condition != NULL) {
                fprintf(stderr, "weather_report.current_condition: %s\n", weather_report.current_condition);
            }
            fprintf(stderr, "weather_report.current_temp: %f\n", weather_report.current_temp);
            fprintf(stderr, "weather_report.current_humidity: %f\n", weather_report.current_humidity);
            if (weather_report.next_period_name != NULL) {
                fprintf(stderr, "weather_report.current_condition: %s\n", weather_report.next_period_name);
            }
            if (weather_report.next_period_forecast != NULL) {
                fprintf(stderr, "weather_report.current_condition: %s\n", weather_report.next_period_forecast);
            }
            
            free(text);
        }
    }

    return &weather_report;
}
