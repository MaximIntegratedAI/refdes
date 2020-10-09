/*******************************************************************************
 * Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
 *
 * This software is protected by copyright laws of the United States and
 * of foreign countries. This material may also be protected by patent laws
 * and technology transfer regulations of the United States and of foreign
 * countries. This software is furnished under a license agreement and/or a
 * nondisclosure agreement and may only be used or reproduced in accordance
 * with the terms of those agreements. Dissemination of this information to
 * any party or parties not specified in the license agreement and/or
 * nondisclosure agreement is expressly prohibited.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *******************************************************************************
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdio.h>
#include "spi.h"
#include "dma.h"
#include "gpio.h"
#include "tmr_utils.h"

#include "max20303.h"
#include "lcd.h"
#include "faceid_definitions.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define SPI_NAME          SPI1
#define MXC_SPI           MXC_SPI1
#define DMA_REQSEL_SPITX  DMA_REQSEL_SPI1TX
#define SPI_SPEED         16000000

#define DMA_CHANNEL          0
#define DMA_CHANNEL_IRQ      DMA0_IRQn
#define DMA_CHANNEL_IRQ_HAND DMA0_IRQHandler

#define ST_CMD_DELAY   0x80

#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01

#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID   0x04
#define ST7789_RDDST   0x09

#define ST7789_SLPIN   0x10
#define ST7789_SLPOUT  0x11
#define ST7789_PTLON   0x12
#define ST7789_NORON   0x13

#define ST7789_INVOFF  0x20
#define ST7789_INVON   0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON  0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E

#define ST7789_PTLAR   0x30
#define ST7789_COLMOD  0x3A
#define ST7789_MADCTL  0x36

#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00

#define ST7789_RDID1   0xDA
#define ST7789_RDID2   0xDB
#define ST7789_RDID3   0xDC
#define ST7789_RDID4   0xDD

#define ST7789_ROTATION 2

#if ST7789_ROTATION == 0
    #define X_SHIFT 0
    #define Y_SHIFT 80
#elif ST7789_ROTATION == 1
    #define X_SHIFT 80
    #define Y_SHIFT 0
#elif ST7789_ROTATION == 2
    #define X_SHIFT 0
    #define Y_SHIFT 0
#elif ST7789_ROTATION == 3
    #define X_SHIFT 0
    #define Y_SHIFT 0
#endif


//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
//static volatile int  spi_flag;
static const gpio_cfg_t lcd_dc_pin     = {PORT_1, PIN_14  , GPIO_FUNC_OUT, GPIO_PAD_PULL_UP};
static const gpio_cfg_t lcd_reset_pin  = {PORT_1, PIN_15 , GPIO_FUNC_OUT, GPIO_PAD_PULL_UP};
static const gpio_cfg_t ssel_pin       = {PORT_0, PIN_22 , GPIO_FUNC_OUT, GPIO_PAD_PULL_UP};
static volatile int tx_dma_complete = 1;


//-----------------------------------------------------------------------------
// Local function declarations
//-----------------------------------------------------------------------------
static void lcd_reset();
static void lcd_backlight(unsigned char onoff);
static void lcd_sendCommand(uint8_t command);
static void lcd_sendSmallData(uint8_t data);
static void lcd_sendData(uint8_t *data, uint32_t dataLen);
static void lcd_configure();
static void lcd_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
static void lcd_writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
static void spi_init();
static void spi_sendPacket(uint8_t* out, unsigned int len);


//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

// DMA IRQ Handlers
void DMA_CHANNEL_IRQ_HAND()
{
    int ch = DMA_CHANNEL;
    if (MXC_DMA->intr & (0x1 << ch)) {
        if (MXC_DMA->ch[ch].st & (MXC_F_DMA_ST_TO_ST | MXC_F_DMA_ST_BUS_ERR)) {
            printf("dma error %d\n", MXC_DMA->ch[ch].st);
        }

        if (MXC_DMA->ch[ch].cnt) {
            printf("dma is not empty %d\n", MXC_DMA->ch[ch].cnt);
        }

        // Clear DMA int flags
        MXC_DMA->ch[ch].st =  MXC_DMA->ch[ch].st;

        tx_dma_complete = 1;
    }
}

static void spi_init()
{
    // Initialize DMA peripheral
    DMA_Init();
    NVIC_EnableIRQ(DMA_CHANNEL_IRQ);

    // Initialize SPI peripheral
    sys_cfg_spi_t master_cfg = {0};
    master_cfg.map = MAP_A;
    master_cfg.ss0 = Disable;
    master_cfg.ss1 = Disable;
    master_cfg.ss2 = Disable;

    SPI_Init(SPI_NAME, 0, SPI_SPEED, master_cfg);
    MXC_SPI->ctrl0 |= MXC_F_SPI17Y_CTRL0_MASTER; // Switch SPI to master mode, else the state of the SS pin could cause the SPI periph to appear 'BUSY';

    MXC_SPI->ss_time = (4 << MXC_F_SPI17Y_SS_TIME_PRE_POS) |
            (8 << MXC_F_SPI17Y_SS_TIME_POST_POS) |
            (16 << MXC_F_SPI17Y_SS_TIME_INACT_POS);
}

static void spi_sendPacket(uint8_t* out, unsigned int len)
{
    int ch = DMA_CHANNEL;

    // Clear DMA fifo
    MXC_SPI->dma |= MXC_F_SPI17Y_DMA_TX_FIFO_CLEAR | MXC_F_SPI17Y_DMA_RX_FIFO_CLEAR;

    // Initialize the CTRL0 register
    MXC_SPI->ctrl0 = MXC_F_SPI17Y_CTRL0_MASTER |   // Enable master mode
                     MXC_F_SPI17Y_CTRL0_EN;   // Enable the peripheral

    // Initialize the CTRL1 register
    MXC_SPI->ctrl1 = 0;

    // Set how many to characters to send, or when in quad mode how many characters to receive
    MXC_SPI->ctrl1 |= (len << MXC_F_SPI17Y_CTRL1_TX_NUM_CHAR_POS);

    // Initialize the CTRL2 register
    MXC_SPI->ctrl2 = (MXC_S_SPI17Y_CTRL2_DATA_WIDTH_MONO) |
                     (8 << MXC_F_SPI17Y_CTRL2_NUMBITS_POS);

    // Initialize the DMA register
    MXC_SPI->dma = 0;
    MXC_SPI->dma |= MXC_F_SPI17Y_DMA_TX_DMA_EN |                   // Enable DMA for transmit
                    MXC_F_SPI17Y_DMA_TX_FIFO_EN |                  // Enable the TX FIFO
                    (31 << MXC_F_SPI17Y_DMA_TX_FIFO_LEVEL_POS);    // Set the threshold of when to add more data to TX FIFO

    // Prepare DMA to fill TX FIFO.
    MXC_DMA->ch[ch].cfg = MXC_F_DMA_CFG_SRCINC |
                          MXC_F_DMA_CFG_CHDIEN |
                          MXC_S_DMA_CFG_PRI_LOW |
                          DMA_REQSEL_SPITX |
                          MXC_S_DMA_CFG_TOSEL_TO4 |
                          MXC_S_DMA_CFG_PSSEL_DIS |
                          (MXC_V_DMA_CFG_SRCWD_BYTE << MXC_F_DMA_CFG_SRCWD_POS) |
                          (MXC_V_DMA_CFG_SRCWD_BYTE << MXC_F_DMA_CFG_DSTWD_POS);

    MXC_DMA->cn |= (1 << ch);
    MXC_DMA->ch[ch].src = (unsigned int) out;
    MXC_DMA->ch[ch].dst = 0;
    MXC_DMA->ch[ch].cnt = len;

    tx_dma_complete = 0;

    MXC_DMA->ch[ch].cfg |= MXC_F_DMA_CFG_CHEN;

    // Start the SPI transaction
    MXC_SPI->ctrl0 |= MXC_F_SPI17Y_CTRL0_START;

    // Wait here for all DMA transactions to complete.
    // This would normally be where this function would
    // return so the micro can go do other things.  The
    // callbacks will be triggered when the DMA completes
    // so the application knows its time to do more work.
    // For this simple example, we'll just block here.
    while(!tx_dma_complete);

    // DMA has completed, but that just means DMA has loaded all of its data to/from
    //  the FIFOs.  We now need to wait for the SPI transaction to fully complete.
    if(MXC_SPI->ctrl0 & MXC_F_SPI17Y_CTRL0_EN) {
        while(MXC_SPI->stat & MXC_F_SPI17Y_STAT_BUSY);
    }
}

static void lcd_sendCommand(uint8_t command)
{
    GPIO_OutClr(&lcd_dc_pin);
    spi_sendPacket(&command, 1);
}

static void lcd_sendSmallData(uint8_t data)
{
    GPIO_OutSet(&lcd_dc_pin);
    spi_sendPacket(&data, 1);
}

static void lcd_sendData(uint8_t *data, uint32_t dataLen)
{
    GPIO_OutSet(&lcd_dc_pin);
    spi_sendPacket(data, dataLen);
}

static void lcd_reset()
{
    GPIO_OutClr(&lcd_reset_pin);
    TMR_Delay(MXC_TMR0, MSEC(25), 0);
    GPIO_OutSet(&lcd_reset_pin);
    TMR_Delay(MXC_TMR0, MSEC(15), 0);
}

static void lcd_backlight(unsigned char onoff)
{
    if (onoff) {
        MAX20303_setboost( MAX20303_BOOST_OUTPUT_ON, 0x10 );
    } else {
        MAX20303_setboost( MAX20303_BOOST_OUTPUT_OFF, 0x10 );
    }
}

static void lcd_configure()
{
    GPIO_OutClr(&ssel_pin);

    lcd_sendCommand(ST7789_COLMOD);     //  Set color mode
    lcd_sendSmallData(0x55);            //  RGB565 (16bit)

    lcd_sendCommand(0xB2);              //  Porch control
    {
        uint8_t data[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
        lcd_sendData(data, sizeof(data));
    }

    lcd_sendCommand(ST7789_MADCTL);    //  Display Rotation
#if ST7789_ROTATION == 0
    lcd_sendSmallData(ST7789_MADCTL_MX | ST7789_MADCTL_MY | ST7789_MADCTL_RGB);
#elif ST7789_ROTATION == 1
    lcd_sendSmallData(ST7789_MADCTL_MY | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
#elif ST7789_ROTATION == 2
    lcd_sendSmallData(ST7789_MADCTL_RGB);
#elif ST7789_ROTATION == 3
    lcd_sendSmallData(ST7789_MADCTL_MX | ST7789_MADCTL_MV | ST7789_MADCTL_RGB);
#endif

    /* Internal LCD Voltage generator settings */
    lcd_sendCommand(0XB7);             //  Gate Control
    lcd_sendSmallData(0x35);           //  Default value

    lcd_sendCommand(0xBB);             //  VCOM setting
    lcd_sendSmallData(0x19);           //  0.725v (default 0.75v for 0x20)

    lcd_sendCommand(0xC0);             //  LCMCTRL
    lcd_sendSmallData(0x2C);           //  Default value

    lcd_sendCommand(0xC2);             //  VDV and VRH command Enable
    lcd_sendSmallData(0x01);           //  Default value

    lcd_sendCommand(0xC3);             //  VRH set
    lcd_sendSmallData(0x12);           //  +-4.45v (defalut +-4.1v for 0x0B)

    lcd_sendCommand(0xC4);             //  VDV set
    lcd_sendSmallData(0x20);           //  Default value

    lcd_sendCommand(0xC6);             //  Frame rate control in normal mode
    lcd_sendSmallData(0x0F);           //  Default value (60HZ)

    lcd_sendCommand(0xD0);             //  Power control
    lcd_sendSmallData(0xA4);           //  Default value
    lcd_sendSmallData(0xA1);           //  Default value

    lcd_sendCommand(0xE0);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
        lcd_sendData(data, sizeof(data));
    }

    lcd_sendCommand(0xE1);
    {
        uint8_t data[] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
        lcd_sendData(data, sizeof(data));
    }

    lcd_sendCommand(ST7789_INVON);     //  Inversion ON

    lcd_sendCommand(ST7789_SLPOUT);    //  Out of sleep mode

    lcd_sendCommand(ST7789_NORON);     //  Normal Display on

    lcd_sendCommand(ST7789_DISPON);    //  Main screen turned on

    GPIO_OutSet(&ssel_pin);
}

/**
 * @brief Set address of DisplayWindow
 * @param xi&yi -> coordinates of window
 * @return none
 */
static void lcd_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint16_t x_start = x0 + X_SHIFT, x_end = x1 + X_SHIFT;
    uint16_t y_start = y0 + Y_SHIFT, y_end = y1 + Y_SHIFT;

    GPIO_OutClr(&ssel_pin);
    lcd_sendCommand(ST7789_CASET);
    {
        uint8_t data[] = {x_start >> 8, x_start & 0xFF, x_end >> 8, x_end & 0xFF};
        lcd_sendData(data, sizeof(data));
    }

    lcd_sendCommand(ST7789_RASET);
    {
        uint8_t data[] = {y_start >> 8, y_start & 0xFF, y_end >> 8, y_end & 0xFF};
        lcd_sendData(data, sizeof(data));
    }

    lcd_sendCommand(ST7789_RAMWR);
    GPIO_OutSet(&ssel_pin);
}

/**
 * @brief Draw an Image on the screen
 * @param x&y -> start point of the Image
 * @param w&h -> width & height of the Image to Draw
 * @param data -> pointer of the Image array
 * @return none
 */
void lcd_drawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data)
{
//    uint32_t chunk_size;
//    uint32_t buff_size = 2 * w * h;
    uint8_t *buff = data;

    lcd_setAddrWindow(x, y, x + w - 1, y + h - 1);

    GPIO_OutSet(&lcd_dc_pin);
    GPIO_OutClr(&ssel_pin);

//    while (buff_size > 0) {
//        chunk_size = buff_size > 57600 ? 57600 : buff_size;
//        spi_sendPacket(buff, chunk_size);
//        buff += chunk_size;
//        buff_size -= chunk_size;
//    }

    spi_sendPacket(buff, 57600);
    spi_sendPacket(buff + 57600, 57600);

    GPIO_OutSet(&ssel_pin);
}

/**
 * @brief Write a char
 * @param  x&y -> cursor of the start point.
 * @param ch -> char to write
 * @param font -> fontstyle of the string
 * @param color -> color of the char
 * @param bgcolor -> background color of the char
 * @return  none
 */
static void lcd_writeChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;

    lcd_setAddrWindow(x, y, x + font.width - 1, y + font.height - 1);

    GPIO_OutClr(&ssel_pin);

    for (i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++) {
            if ((b << j) & 0x8000) {
                uint8_t data[] = {color >> 8, color & 0xFF};
                lcd_sendData(data, sizeof(data));
            }
            else {
                uint8_t data[] = {bgcolor >> 8, bgcolor & 0xFF};
                lcd_sendData(data, sizeof(data));
            }
        }
    }

    GPIO_OutSet(&ssel_pin);
}

/**
 * @brief Write a string
 * @param  x&y -> cursor of the start point.
 * @param str -> string to write
 * @param font -> fontstyle of the string
 * @param color -> color of the string
 * @param bgcolor -> background color of the string
 * @return  none
 */
void lcd_writeStringWithBG(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    while (*str) {
        if (x + font.width >= LCD_WIDTH) {
            x = 0;
            y += font.height;
            if (y + font.height >= LCD_HEIGHT) {
                break;
            }

            if (*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }
        lcd_writeChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
}

int lcd_init()
{
    lcd_backlight(1);

    GPIO_OutSet(&ssel_pin);
    GPIO_Config(&ssel_pin);

    GPIO_OutSet(&lcd_dc_pin);
    GPIO_Config(&lcd_dc_pin);

    GPIO_OutSet(&lcd_reset_pin);
    GPIO_Config(&lcd_reset_pin);

    lcd_reset();

    spi_init();

    lcd_configure();

    return E_NO_ERROR;
}
