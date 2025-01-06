#include "menu_task.h"

void MENU_RockerInfo(void)
{
    while (1)
    {
        OLED_NewFrame();
        OLED_DrawRectangle(30, 30, 35 / 1.5, 42 / 1.5, 0);
        OLED_DrawFilledCircle(30 + (35 / 1.5) / 2, 30 + (42 / 1.5) / 2, 2, 0); // 表示摇杆位置
        OLED_DrawRectangle(128 - 30 - 35 / 1.5, 30, 35 / 1.5, 42 / 1.5, 0);
        OLED_DrawFilledCircle(128 - 30 - 35 / 1.5 + (35 / 1.5) / 2, 30 + (42 / 1.5) / 2, 2, 0); // 表示摇杆位置
        // TODO
        OLED_PrintASCIIString(0, 0, "100", &afont24x12, 0);
        OLED_PrintASCIIString(0, 0, "-100", &afont24x12, 0);
        OLED_ShowFrame();
        if (menu_command_callback(GET_EVENT_ENTER))
            break;

        if (menu_command_callback(GET_EVENT_BACK))
            break;
        osDelay(5);
    }
}

void MENU_Calibration(void)
{
    while (1)
    {
        // 状态机思想
        static uint8_t state = 0;
        if (state == 0)
        {
            // 显示校准提示：plz Center all joysticks.
            // 按下确认按键后 ，进行校准
            if (menu_command_callback(GET_EVENT_ENTER))
                state = 1;
        }
        if (state == 1)
        {
            // 花费1s进行校准
            // 最好显示个进度条
            // 1s到，记录均值并作为0度
            // 状态机跳到state=2
        }
        if (state == 2)
        {
            // 显示校准完成：按下确认键重新校准/按下返回退出校准
        }
        if (menu_command_callback(GET_EVENT_BACK))
            break;
        osDelay(5);
    }
}

void MENU_ShowImage(void)
{
    while (1)
    {
        // 按上/下键切换图片
        if (menu_command_callback(GET_EVENT_ENTER))
            break;

        if (menu_command_callback(GET_EVENT_BACK))
            break;
        osDelay(5);
    }
}

void MENU_ShowInfomation(void)
{
    while (1)
    {
        // 显示温度，版本，autor,链接状态
        if (menu_command_callback(GET_EVENT_ENTER))
            break;

        if (menu_command_callback(GET_EVENT_BACK))
            break;
        osDelay(5);
    }
}