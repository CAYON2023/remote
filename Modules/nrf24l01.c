#include "nrf24l01.h"

uint8_t INIT_ADDR[5] = {0x00, 0x1A, 0xB1, 0xB1, 0x01}; // 节点地址

void NRF24L01_Init(void)
{
    DWT_Init(72);
    Clr_NRF24L01_CE;
    Set_NRF24L01_CSN;
    DWT_Delay(100 * 1e-6);
}

// spi单字节读写函数
static uint8_t NRF24L01_SPI_Send_Byte(uint8_t txdata)
{
    uint8_t rxdata;
    HAL_SPI_TransmitReceive(&hspi1, &txdata, &rxdata, 1, 0x10);
    return rxdata;
}

/**
 * @brief 在指定地址写入指定数据SPI封装函数
 *
 * @param regaddr 地址
 * @param pBuf 数据
 * @param datalen 长度
 * @return uint8_t  此次地址读到的状态寄存器值
 */
static uint8_t NRF24L01_Write_Buf(uint8_t regaddr, uint8_t *pBuf, uint8_t datalen)
{
    uint8_t status;
    Clr_NRF24L01_CSN;                              // 片选NRF24L01 SPI
    status = NRF24L01_SPI_Send_Byte(regaddr);      // 指定地址
    HAL_SPI_Transmit(&hspi1, pBuf, datalen, 0x10); // 指定数据
    Set_NRF24L01_CSN;
    return status;
}

/**
 * @brief 在指定地址读出指定长度数据SPI封装函数
 *
 * @param regaddr 地址
 * @param pBuf 数据
 * @param datalen 长度
 * @return uint8_t  此次地址读到的状态寄存器值
 */
static uint8_t NRF24L01_Read_Buf(uint8_t regaddr, uint8_t *pBuf, uint8_t datalen)
{
    uint8_t status;
    Clr_NRF24L01_CSN;                             // 片选NRF24L01 SPI
    status = NRF24L01_SPI_Send_Byte(regaddr);     // 指定地址
    HAL_SPI_Receive(&hspi1, pBuf, datalen, 0x10); // 指定数据
    Set_NRF24L01_CSN;
    return status;
}

/**
 * @brief 检测NRF24L01通讯是否正常
 *
 * @return uint8_t 1：通信正常；0通信异常
 */
uint8_t NRF24L01_Check(void)
{
    uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    uint8_t buf_back[5];
    NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, buf, 5);
    NRF24L01_Read_Buf(TX_ADDR, buf_back, 5);
    for (int i = 0; i < 5; i++)
        if (buf_back[i] != 0xA5)
            return 0; // 检测24L01读写错误
    return 1;
}

/**
 * @brief 对NRF24L01的寄存器写入数据
 *
 * @param regaddr
 * @param data
 * @return uint8_t
 */
static uint8_t NRF24L01_Write_Reg(uint8_t regaddr, uint8_t data)
{
    uint8_t status;
    Clr_NRF24L01_CSN;
    status = NRF24L01_SPI_Send_Byte(regaddr); // 目标地址
    NRF24L01_SPI_Send_Byte(data);             // 写入数据
    Set_NRF24L01_CSN;
    return status;
}

/**
 * @brief 返回NRF24L01对应寄存器的值
 *
 * @param regaddr 需要读取的寄存器地址
 * @return uint8_t
 */
static uint8_t NRF24L01_Read_Reg(uint8_t regaddr)
{
    uint8_t reg_val;
    Clr_NRF24L01_CSN;
    NRF24L01_SPI_Send_Byte(regaddr);
    reg_val = NRF24L01_SPI_Send_Byte(0xFF);
    Set_NRF24L01_CSN;
    return reg_val;
}

/**
 * @brief 配置NRF24L01发送工作模式
 *
 * @param numoslave
 */
void TX_Mode(uint8_t numoslave)
{
    INIT_ADDR[0] = numoslave;
    Clr_NRF24L01_CE;                                                                    // 将NRF24L01工作模式设定为待机模式以写入数据
    NRF24L01_Write_Buf(SPI_WRITE_REG + TX_ADDR, (uint8_t *)INIT_ADDR, TX_ADR_WIDTH);    // 配置发送地址
    NRF24L01_Write_Buf(SPI_WRITE_REG + RX_ADDR_P0, (uint8_t *)INIT_ADDR, RX_ADR_WIDTH); // 配置接收地址
    NRF24L01_Write_Reg(SPI_WRITE_REG + EN_AA, 0x01);                                    // 开启自动应答
    NRF24L01_Write_Reg(SPI_WRITE_REG + EN_RXADDR, 0X01);
    NRF24L01_Write_Reg(SPI_WRITE_REG + SETUP_RETR, 0x1a);
    NRF24L01_Write_Reg(SPI_WRITE_REG + RF_CH, 40); // 设定通讯频率
    NRF24L01_Write_Reg(SPI_WRITE_REG + RF_SETUP, 0x0f);
    NRF24L01_Write_Reg(SPI_WRITE_REG + CONFIG, 0x0e); // 配置基本工作模式为发送
    Set_NRF24L01_CE;                                  // 退出待机模式，关闭配置
}

/**
 * @brief 发送数据
 *
 * @param txbuf 需要发送数据的首地址
 * @return uint8_t
 */
uint8_t NRF24L01_TxPacket(uint8_t *txbuf)
{
    uint8_t state;
    Clr_NRF24L01_CE;                                        // 更改为待机模式，进入配置
    NRF24L01_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); // 将发送数据写入NRF24L01缓存
    Set_NRF24L01_CE;                                        // 退出配置
    while (READ_NRF24L01_IRQ)
        osDelay(1);                                    // 等待发送完成
    state = NRF24L01_Read_Reg(STATUS);                 // 读取发送状态
    NRF24L01_Write_Reg(SPI_WRITE_REG + STATUS, state); // 清除中断标志
    if (state & MAX_TX)
    {
        NRF24L01_Write_Reg(FLUSH_TX, 0xff); // 达到最大发送次数，清空发送邮箱
        return MAX_TX;
    }
    if (state & TX_OK)
        return TX_OK; // 发送成功
    return 0xff;
}

/**
 * @brief 配置NRF24L01工作模式为接收模式
 *
 * @param numoslave
 */
void RX_Mode(uint8_t numoslave)
{
    INIT_ADDR[0] = numoslave;
    Clr_NRF24L01_CE;
    NRF24L01_Write_Buf(SPI_WRITE_REG + RX_ADDR_P0, (uint8_t *)INIT_ADDR, RX_ADR_WIDTH);
    NRF24L01_Write_Reg(SPI_WRITE_REG + EN_AA, 0x01);
    NRF24L01_Write_Reg(SPI_WRITE_REG + EN_RXADDR, 0x01);
    NRF24L01_Write_Reg(SPI_WRITE_REG + RF_CH, 40);
    NRF24L01_Write_Reg(SPI_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);
    NRF24L01_Write_Reg(SPI_WRITE_REG + RF_SETUP, 0x0f);
    NRF24L01_Write_Reg(SPI_WRITE_REG + CONFIG, 0x0f);
    Set_NRF24L01_CE;
}

/**
 * @brief 读取数据
 *
 * @param rxbuf 数据存储地址
 * @return uint8_t 1为读取成功，0为读取失败
 */
uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{
    uint8_t state;
    state = NRF24L01_Read_Reg(STATUS);
    NRF24L01_Write_Reg(SPI_WRITE_REG + STATUS, state);
    if (state & RX_OK)
    {
        NRF24L01_Read_Buf(RD_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH);
        NRF24L01_Write_Reg(FLUSH_RX, 0xff);
        return 1;
    }
    return 0;
}