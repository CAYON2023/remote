#include "remote.h"
#include "oled.h"
#include "nrf24l01.h"
#include "adc.h"
#include "MENU.h"
#include "menu_task.h"
uint16_t adc_val[8] = {0};
uint8_t tx_buf[8] = {0};
RC_Info_t remote_info;
RC_t remote;
osThreadId scaninfoTaskHandle;
osThreadId menuTaskHandle;
osThreadId sendTaskHandle;

static void RemoteHardware_Init(void)
{
    HAL_Delay(100); // 等待其他外设上电
    __disable_irq();
    OLED_Init();
    NRF24L01_Init();
    HAL_ADCEx_Calibration_Start(&hadc1); // ADC校准
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_val, sizeof(adc_val) / sizeof(uint16_t));
    while (!NRF24L01_Check())
        ;
    TX_Mode(0);
    __enable_irq();
}

void Remote_Init(void)
{
    RemoteHardware_Init();
    memset(&remote_info, 0x00, sizeof(RC_Info_t));
    memset(&remote, 0x00, sizeof(RC_t));
    remote.info = &remote_info;
}

static void RemoteDataProc(RC_t *data)
{
    memcpy(&data->res.key, &data->info->key, sizeof(data->info->key));
    for (int i = 0; i < 2; i++)
    {
        data->res.potVal[i] = (int)(-((float)(data->info->pVal[i] - data->offset_rocker[i]) / 4096) * 255);
    }

    for (int i = 0; i < 4; i++)
    {
        if (i > 1)
            data->res.rockerVal[i] = -(int)(((float)(data->info->rockerVal[i] - data->offset_rocker[2 + i]) / 2048) * 127 * 5);
        else
            data->res.rockerVal[i] = (int)(((float)(data->info->rockerVal[i] - data->offset_rocker[2 + i]) / 2048) * 127 * 5);
    }
}
static void ScanRemoteInfo(void)
{
    remote_info.key.bits.sw1 = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == GPIO_PIN_SET ? 1 : 0);
    remote_info.key.bits.sw2 = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_SET ? 1 : 0);
    remote_info.key.bits.key0 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_RESET ? 1 : 0);
    remote_info.key.bits.key1 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_RESET ? 1 : 0);
    remote_info.key.bits.key2 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET ? 1 : 0);
    remote_info.key.bits.key3 = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET ? 1 : 0);
    memcpy(remote_info.pVal, &adc_val[4], 2 * sizeof(uint16_t));  // 两个电位器值
    memcpy(remote_info.rockerVal, adc_val, 4 * sizeof(uint16_t)); // 两个摇杆值
    memcpy(&remote.Vref, &adc_val[6], 2 * sizeof(uint16_t));      // 基准电压与温度值
}

static void SendBuff(void)
{
    // 0     1   2   3   4   5   6   7
    // 键值 p1   p2  rl0 rl1 rr0 rr1 校验位
    tx_buf[7] = 0xBB; // 校验位
    tx_buf[0] = remote.res.key.kb;
    for (int i = 0; i < 2; i++)
        tx_buf[1 + i] = remote.res.potVal[i];
    for (int i = 0; i < 4; i++)
        tx_buf[3 + i] = remote.res.rockerVal[i] + 128;
    if (NRF24L01_TxPacket(tx_buf) == TX_OK)
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        remote.online = 1;
    }
    else
    {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        remote.online = 0;
    }
}

__attribute__((noreturn)) void ScanInfoTASK(void const *argument)
{
    for (;;)
    {
        ScanRemoteInfo();
        RemoteDataProc(&remote);
        osDelay(5);
    }
}

__attribute__((noreturn)) void SendTASK(void const *argument)
{
    for (;;)
    {
        SendBuff();
        osDelay(10);
    }
}

static MENU_OptionTypeDef MENU_OptionList[] = {
    {"<<<", NULL},                        // 固定格式, 用于退出
    {"RockerInfo", MENU_RockerInfo},      // 工具
    {"Calibration", MENU_Calibration},    // 游戏
    {"Iamge", MENU_ShowImage},            // 设置
    {"Information", MENU_ShowInfomation}, // 信息
    {"..", NULL},                         // 固定格式, 用于计算选项列表长度和退出
};
static MENU_HandleTypeDef MENU = {.OptionList = MENU_OptionList};

__attribute__((noreturn)) void MenuTASK(void const *argument)
{
    MENU_HandleInit(&MENU);
    MENU_Init();
    for (;;)
    {
        // while (MENU.isRun)
        {
            menu_command_callback(BUFFER_CLEAR); // 擦除缓冲区
            MENU_ShowOptionList(&MENU);          /* 显示选项列表 */
            MENU_ShowCursor(&MENU);              /* 显示光标 */
            MENU_ShowBorder(&MENU);              // 显示边框

            menu_command_callback(BUFFER_DISPLAY); // 缓冲区更新至显示器
            MENU_Event_and_Action(&MENU);          // 检查事件及作相应操作
            osDelay(1);
        }
    }
}

void RemoteTask_Init(void)
{
    osThreadDef(scaninfotask, ScanInfoTASK, osPriorityHigh, 0, 128);
    scaninfoTaskHandle = osThreadCreate(osThread(scaninfotask), NULL); //

    osThreadDef(menutask, MenuTASK, osPriorityNormal, 0, 256);
    menuTaskHandle = osThreadCreate(osThread(menutask), NULL);

    osThreadDef(sendtask, SendTASK, osPriorityRealtime, 0, 128);
    sendTaskHandle = osThreadCreate(osThread(sendtask), NULL);
}

RC_t *GetRCpointer(void) { return &remote; }