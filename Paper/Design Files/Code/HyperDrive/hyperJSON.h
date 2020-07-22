#pragma once

#include <stdint.h>
#include <ArduinoJson.h> //Need to include


struct hyper_Base_t
{
    char name[16];
    int instance;
    int Pass;
    int X;                          
    int Y;                                                                  
    int Z;                                                                   
    int MS;                                                                  
    int SRX;                                                                  
    int SRYZ;                                                                 
    int Go;                                                                         
    int L;                                                                        
    int R;                                                                      
    int C;                                                                    
    int PX;                                                                      
    int PY;                                                                      
    int PZ; 
};

typedef union {
    hyper_Base_t data;
    uint8_t raw[sizeof(hyper_Base_t)];
} hyper_Base;

void json_to_struct(const JsonObjectConst& data, hyper_Base& out);


void struct_to_json(const hyper_Base& in, const JsonObject& out);
