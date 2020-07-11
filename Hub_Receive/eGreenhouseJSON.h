#pragma once

#include <stdint.h>
#include <ArduinoJson.h> //Need to include


struct eGreenhouse_Base_t
{
    char name[16];
    int instance;
    char datestamp[16];
    char timestamp[16];
    int packageNum;
    float sht31dHumidity;
    float sht31dTemp;
    int tsl2591Vis;
    int tsl2591IR;
    int tsl2591Full;
    float k30;
    int X;
    int Y;
    int Z;
    int done;
};

typedef union {
    eGreenhouse_Base_t data;
    uint8_t raw[sizeof(eGreenhouse_Base_t)];
} eGreenhouse_Base;


void hub_struct_to_json(const eGreenhouse_Base& in, const JsonObject& out);
