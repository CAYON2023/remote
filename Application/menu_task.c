#include "menu_task.h"
#include "stdio.h"
#include "stm32f1xx_hal.h"
#include "flash.h"
RC_t *rc;

static void write_offset_rocker_to_flash(uint16_t *offset_rocker)
{
    uint8_t buffer[12]; // 以字节为单位，每个uint16_t需要2个字节

    // 将uint16_t数组复制到uint8_t缓冲区中
    for (int i = 0; i < 6; i++)
    {
        buffer[i * 2] = (uint8_t)(offset_rocker[i] & 0xFF);            // 低字节
        buffer[i * 2 + 1] = (uint8_t)((offset_rocker[i] >> 8) & 0xFF); // 高字节
    }

    // 将数据写入Flash存储器
    FlashWriteBuff(REMOTE_OFFSET_ADDRESS, buffer, sizeof(buffer));
}

static void read_offset_rocker_from_flash(uint16_t *offset_rocker)
{
    uint8_t buffer[12]; // 以字节为单位，每个uint16_t需要2个字节

    // 从Flash存储器中读取数据到缓冲区
    FlashReadBuff(REMOTE_OFFSET_ADDRESS, buffer, sizeof(buffer));

    // 将uint8_t缓冲区数据转换为uint16_t数组
    for (int i = 0; i < 6; i++)
    {
        offset_rocker[i] = (uint16_t)(buffer[i * 2] | (buffer[i * 2 + 1] << 8));
    }
}
void MENU_Init(void)
{
    rc = GetRCpointer();
    read_offset_rocker_from_flash(rc->offset_rocker);
}

void MENU_RockerInfo(void)
{
    while (1)
    {
        OLED_NewFrame();
        for (int i = 0; i < 2; i++)
        {
            OLED_DrawFilledRectangle(7 + i * 64, 10, (((float)rc->res.potVal[i] / 255) * 50), 5, 0); // 用矩形长度表示电位器大小
        }
        if (rc->info->key.bits.sw1)
            OLED_DrawFilledCircle(16, 25, 5, 0);
        else
            OLED_DrawCircle(16, 25, 5, 0);
        if (rc->info->key.bits.sw2)
            OLED_DrawFilledCircle(128 - 16, 25, 5, 0);
        else
            OLED_DrawCircle(128 - 16, 25, 5, 0);
        if (rc->info->key.bits.key0)
            OLED_DrawFilledCircle(33, 23, 3, 0);
        else
            OLED_DrawCircle(33, 23, 3, 0);
        if (rc->info->key.bits.key1)
            OLED_DrawFilledCircle(33 + 18, 23, 3, 0);
        else
            OLED_DrawCircle(33 + 18, 23, 3, 0);
        if (rc->info->key.bits.key2)
            OLED_DrawFilledCircle(36 + 40, 23, 3, 0);
        else
            OLED_DrawCircle(36 + 40, 23, 3, 0);
        if (rc->info->key.bits.key3)
            OLED_DrawFilledCircle(128 - 36, 23, 3, 0);
        else
            OLED_DrawCircle(128 - 36, 23, 3, 0);
        // 0L-> 1L↑ 2R↑ 3R->
        OLED_DrawRectangle(30, 30, 35 / 1.5, 42 / 1.5, 0);
        OLED_DrawFilledCircle(30 + (35 / 1.5) / 2 + (35 / 1.5) / 2 * ((float)rc->res.rockerVal[0] / 105), 30 + (42 / 1.5) / 2 - (42 / 1.5) / 2 * ((float)rc->res.rockerVal[1] / 105), 2, 0); // 表示摇杆位置
        OLED_DrawRectangle(128 - 30 - 35 / 1.5, 30, 35 / 1.5, 42 / 1.5, 0);
        OLED_DrawFilledCircle(128 - 30 - 35 / 1.5 + (35 / 1.5) / 2 + (35 / 1.5) / 2 * ((float)rc->res.rockerVal[3] / 105), 30 + (42 / 1.5) / 2 - (42 / 1.5) / 2 * ((float)rc->res.rockerVal[2] / 105), 2, 0); // 表示摇杆位置
        OLED_ShowFrame();

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
        static uint32_t adc_sum[6];
        static uint16_t adc_cnt = 0;
        if (state == 0)
        {
            // 显示校准提示：plz Center all joysticks.
            // 按下确认按键后 ，进行校准
            OLED_NewFrame();
            OLED_PrintString(35, 10, "摇杆校准", &font16x16, 0);
            OLED_PrintString(17, 30, "将所有摇杆归中", &font14x14, 0);
            OLED_PrintString(17, 46, "按下确认键校准", &font14x14, 0);
            OLED_ShowFrame();
            if (menu_command_callback(GET_EVENT_ENTER))
                state = 1;
        }
        if (state == 1)
        {
            OLED_NewFrame();
            OLED_PrintString(42, 15, "校准中", &font16x16, 0);
            OLED_DrawFilledRectangle(16, 45, ((float)adc_cnt / 800) * 100, 12, 0);
            OLED_ShowFrame();
            for (int i = 0; i < 4; i++)
            {
                adc_sum[2 + i] += rc->info->rockerVal[i];
                adc_cnt++;
            }
            for (int i = 0; i < 2; i++)
            {
                adc_sum[i] += rc->info->pVal[i];
            }
            if ((adc_cnt / 4) > 200) // 校准完成
            {
                adc_cnt /= 4;
                for (int i = 0; i < 2; i++)
                {
                    rc->offset_rocker[i] = adc_sum[i] / adc_cnt;
                    adc_sum[i] = 0;
                }
                for (int i = 0; i < 4; i++)
                {
                    rc->offset_rocker[2 + i] = adc_sum[2 + i] / adc_cnt;
                    adc_sum[2 + i] = 0;
                }
                write_offset_rocker_to_flash(rc->offset_rocker);
                adc_cnt = 0;
                state = 2;
            }
        }
        if (state == 2)
        {
            OLED_NewFrame();
            OLED_PrintString(35, 10, "校准完成", &font16x16, 0);
            OLED_PrintString(3, 30, "按下确认键重新校准", &font14x14, 0);
            OLED_PrintString(3, 46, "按下返回键返回菜单", &font14x14, 0);
            OLED_ShowFrame();
            if (menu_command_callback(GET_EVENT_ENTER))
                state = 1;
        }
        if (menu_command_callback(GET_EVENT_BACK))
        {
            state = 0;
            adc_cnt = 0;
            for (int i = 0; i < 6; i++)
            {
                adc_sum[i] = 0;
            }
            break;
        }

        osDelay(5);
    }
}

void MENU_ShowImage(void)
{
    while (1)
    {
        static int8_t rank_img = 5;
        // 按上/下键切换图片
        if (Key_GetEvent_Up())
            if (rank_img > 7)
                rank_img = 0;
            else
                rank_img++;
        if (Key_GetEvent_Down())
            if (rank_img < 2)
                rank_img = 7;
            else
                rank_img--;

        OLED_NewFrame();
        if (rank_img > 7)
            OLED_DrawImage(24, 0, &infantry34Img, 0);
        else if (rank_img > 6)
            OLED_DrawImage(16, 0, &inf4Img, 0);
        else if (rank_img > 5)
            OLED_DrawImage(16, 0, &heroainfImg, 0);
        else if (rank_img > 4) // 5
            OLED_DrawImage(33, 0, &APEXImg, 0);
        else if (rank_img > 3) // 4
            OLED_DrawImage(30, 0, &ACEImg, 0);
        else if (rank_img > 2) // 3
            OLED_DrawImage(17, 0, &ROBOMASTERImg, 0);
        else if (rank_img > 1) // 2
            OLED_DrawImage(17, 0, &CSGOImg, 0);
        else // 1
            OLED_DrawImage(24, 0, &dafuImg, 0);
        OLED_ShowFrame();

        if (menu_command_callback(GET_EVENT_BACK))
            break;
        osDelay(5);
    }
}

void MENU_ShowInfomation(void)
{
    while (1)
    {
        char Vbuf[50];
        char Tbuf[50];
        float Vref = (1.2 / ((float)rc->Vref / 4095));
        float temperature = ((1.34 - ((float)rc->temperature) * Vref / 4096) / 4.3) + 25.0;
        int intValue = (int)Vref;
        float decimalValue = Vref - (float)intValue;
        int decimalIntValue = (int)(decimalValue * 1000); // 保留两位小数
        sprintf(Vbuf, "Vref=%d.%dV", intValue, decimalIntValue);
        intValue = (int)temperature;
        decimalValue = temperature - (float)intValue;
        decimalIntValue = (int)(decimalValue * 1000); // 保留两位小数
        sprintf(Tbuf, "Temp=%d.%dC", intValue, decimalIntValue);
        OLED_NewFrame();
        OLED_PrintASCIIString(5, 4, Vbuf, &afont16x8, 0);
        OLED_PrintASCIIString(5, 24, Tbuf, &afont16x8, 0);
        if (rc->online)
            OLED_PrintASCIIString(5, 44, "ONLINE", &afont16x8, 0);
        else
            OLED_PrintASCIIString(5, 44, "OFFLINE", &afont16x8, 0);
        OLED_PrintASCIIString(72, 48, "By CAYON", &afont12x6, 0);
        OLED_ShowFrame();
        if (menu_command_callback(GET_EVENT_BACK))
            break;
        osDelay(5);
    }
}