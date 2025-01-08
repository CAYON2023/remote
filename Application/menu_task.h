#ifndef __MENU_TASK_H__
#define __MENU_TASK_H__

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

#include "MENU.h"
#include "oled.h"
#include "remote.h"
void MENU_Init(void);
void MENU_RockerInfo(void);
void MENU_Calibration(void);
void MENU_ShowImage(void);
void MENU_ShowInfomation(void);

#endif
