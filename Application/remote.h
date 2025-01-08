#ifndef __REMOTE_H__
#define __REMOTE_H__

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

typedef struct
{
    union
    {
        struct
        {
            uint8_t sw1 : 1;
            uint8_t sw2 : 1;
            uint8_t key0 : 1;
            uint8_t key1 : 1;
            uint8_t key2 : 1;
            uint8_t key3 : 1;
            uint8_t reserved1 : 1;
            uint8_t reserved2 : 1;
        } bits;
        uint8_t kb;
    } key;
    uint16_t pVal[2];
    uint16_t rockerVal[4];
} RC_Info_t;

typedef struct
{
    union
    {
        struct
        {
            uint8_t sw1 : 1;
            uint8_t sw2 : 1;
            uint8_t key0 : 1;
            uint8_t key1 : 1;
            uint8_t key2 : 1;
            uint8_t key3 : 1;
            uint8_t reserved1 : 1;
            uint8_t reserved2 : 1;
        } bits;
        uint8_t kb;
    } key;
    uint8_t potVal[2];
    int8_t rockerVal[4];
} RC_Data_t;

typedef struct
{
    RC_Info_t *info;
    uint16_t Vref;
    uint16_t temperature;
    uint8_t online;
    uint16_t offset_rocker[6];
    RC_Data_t res;
} RC_t;

void Remote_Init(void);
void RemoteTask_Init(void);
RC_t *GetRCpointer(void);
#endif