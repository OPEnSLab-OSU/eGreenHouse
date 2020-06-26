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
};

typedef union {
    eGreenhouse_Base_t data;
    uint8_t raw[sizeof(eGreenhouse_Base_t)];
} eGreenhouse_Base;

void json_to_struct(const JsonObjectConst& data, eGreenhouse_Base& out);

/*
[Device] Json:
{
  "type": "data",
  "id": {
    "name": "Device",
    "instance": 1
  },
  "contents": [
    {
      "module": "Packet",
      "data": {
        "Number": 12
      }
    },
    {
      "module": "SHT31D",
      "data": {
        "temp": 23.12,
        "humid": 54.53
      }
    },
    {
      "module": "TSL2591",
      "data": {
        "Vis": 934,
        "IR": 581,
        "Full": 1517
      }
    },
    {
      "module": "K30",
      "data": {
        "C02": 882
      }
    }
  ],
  "timestamp": {
    "date": "2020/4/30",
    "time": "4:1:27"
  }
}
*/

void struct_to_json(const eGreenhouse_Base& in, const JsonObject& out);
