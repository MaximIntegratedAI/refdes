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

#ifndef _MAXCAM_DEFINTIIONS_H_
#define _MAXCAM_DEFINTIIONS_H_

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "stdint.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
/*** Common ***/
// Common SPI
#define QSPI_SPEED                         10000000UL  // hz
#define QSPI_START_SYMBOL                  0xAABBCCDD

#define SPI_TIMEOUT_CNT                    10000000

// Common LCD
#define LCD_WIDTH                          240
#define LCD_HEIGHT                         240
#define LCD_BYTE_PER_PIXEL                 2  // RGB565
#define LCD_DATA_SIZE                      (LCD_WIDTH * LCD_HEIGHT * LCD_BYTE_PER_PIXEL)
#define LCD_SUBTITLE_SIZE                  32
#define LCD_TOPTITLE_SIZE                  32
#define LCD_ROTATION                       2  // 0-3

// Common Camera
#define CAMERA_WIDTH                       LCD_WIDTH
#define CAMERA_HEIGHT                      LCD_HEIGHT
#define CAMERA_FORMAT                      PIXFORMAT_RGB565

// Common FaceID
#define FACEID_WIDTH                       120
#define FACEID_HEIGHT                      160

#define FACEID_RECTANGLE_X1                ((LCD_WIDTH - FACEID_WIDTH) / 2)
#define FACEID_RECTANGLE_Y1                ((LCD_HEIGHT - FACEID_HEIGHT) / 2)
#define FACEID_RECTANGLE_X2                FACEID_RECTANGLE_X1 + FACEID_WIDTH
#define FACEID_RECTANGLE_Y2                FACEID_RECTANGLE_Y1 + FACEID_HEIGHT

/*** MAX32666 ***/
// MAX32666 PINS
#define MAX32666_VIDEO_INT_PIN             {PORT_0, PIN_30, GPIO_FUNC_IN, GPIO_PAD_PULL_UP}
#define MAX32666_VIDEO_INT_MODE            GPIO_INT_EDGE, GPIO_INT_FALLING
#define MAX32666_VIDEO_CS_PIN              {PORT_0, PIN_8, GPIO_FUNC_OUT, GPIO_PAD_NONE}

#define MAX32666_AUDIO_INT_PIN             {PORT_1, PIN_13, GPIO_FUNC_IN, GPIO_PAD_PULL_UP}
#define MAX32666_AUDIO_INT_MODE            GPIO_INT_EDGE, GPIO_INT_FALLING
#define MAX32666_AUDIO_CS_PIN              {PORT_0, PIN_14, GPIO_FUNC_OUT, GPIO_PAD_NONE}

#define MAX32666_LCD_DC_PIN                {PORT_1, PIN_14  , GPIO_FUNC_OUT, GPIO_PAD_PULL_UP}
#define MAX32666_LCD_RESET_PIN             {PORT_1, PIN_15 , GPIO_FUNC_OUT, GPIO_PAD_PULL_UP}
#define MAX32666_LCD_CS_PIN                {PORT_0, PIN_22 , GPIO_FUNC_OUT, GPIO_PAD_PULL_UP}
#define MAX32666_LCD_TE_PIN                {PORT_0, PIN_31 , GPIO_FUNC_IN, GPIO_PAD_NONE}  // TODO
#define MAX32666_LCD_TOUCH_INT_PIN         {PORT_1, PIN_10, GPIO_FUNC_IN, GPIO_PAD_NONE}   // TODO
#define MAX32666_LCD_TOUCH_INT_MODE        GPIO_INT_EDGE, GPIO_INT_FALLING                 // TODO

#define MAX32666_SD_EN_PIN                 {PORT_1, PIN_12, GPIO_FUNC_OUT, GPIO_PAD_NONE}

#define MAX32666_VIDEO_IO_PIN              {PORT_0, PIN_20, GPIO_FUNC_OUT, GPIO_PAD_NONE}  // TODO
#define MAX32666_AUDIO_IO_PIN              {PORT_0, PIN_21, GPIO_FUNC_OUT, GPIO_PAD_NONE}  // TODO

#define MAX32666_EXPANDER_INT_PIN          {PORT_0, PIN_15, GPIO_FUNC_IN, GPIO_PAD_NONE}   // TODO
#define MAX32666_EXPANDER_INT_MODE          GPIO_INT_EDGE, GPIO_INT_FALLING                // TODO

#define MAX32666_CODEC_INT_PIN             {PORT_0, PIN_23, GPIO_FUNC_IN, GPIO_PAD_NONE}   // TODO
#define MAX32666_CODEC_INT_MODE            GPIO_INT_EDGE, GPIO_INT_FALLING                 // TODO

#define MAX32666_BUTTON1_INT_PIN           {PORT_1, PIN_6, GPIO_FUNC_IN, GPIO_PAD_NONE}    // TODO
#define MAX32666_BUTTON1_INT_MODE          GPIO_INT_EDGE, GPIO_INT_FALLING                 // TODO

#define MAX32666_BUTTON2_INT_PIN           {PORT_1, PIN_8, GPIO_FUNC_IN, GPIO_PAD_NONE}    // TODO
#define MAX32666_BUTTON2_INT_MODE          GPIO_INT_EDGE, GPIO_INT_FALLING                 // TODO

#define MAX32666_ACCEL_INT_PIN             {PORT_1, PIN_11, GPIO_FUNC_IN, GPIO_PAD_NONE}   // TODO
#define MAX32666_ACCEL_INT_MODE            GPIO_INT_EDGE, GPIO_INT_FALLING                 // TODO

#define MAX32666_PMIC_PFN2_PIN             {PORT_1, PIN_9, GPIO_FUNC_OUT, GPIO_PAD_NONE}   // TODO

// MAX32666 LCD
#define MAX32666_LCD_SPI_ID                SPI1
#define MAX32666_LCD_SPI                   MXC_SPI1
#define MAX32666_LCD_DMA_REQSEL_SPITX      DMA_REQSEL_SPI1TX
#define MAX32666_LCD_SPI_SPEED             16000000

#define MAX32666_LCD_DMA_CHANNEL           0
#define MAX32666_LCD_DMA_CHANNEL_IRQ       DMA0_IRQn
#define MAX32666_LCD_DMA_CHANNEL_IRQ_HAND  DMA0_IRQHandler

#define MAX32666_LCD_MAP                   MAP_A

// MAX32666 QSPI
#define MAX32666_QSPI_ID                   SPI0
#define MAX32666_QSPI                      MXC_SPI0
#define MAX32666_QSPI_DMA_REQSEL_SPITX     DMA_REQSEL_SPI0TX
#define MAX32666_QSPI_DMA_REQSEL_SPIRX     DMA_REQSEL_SPI0RX

#define MAX32666_QSPI_DMA_CHANNEL          1
#define MAX32666_QSPI_DMA_CHANNEL_IRQ      DMA1_IRQn
#define MAX32666_QSPI_DMA_CHANNEL_IRQ_HAND DMA1_IRQHandler

#define MAX32666_QSPI_MAP                  MAP_B

// MAX32666 SD CARD
#define MAX32666_SD_BUS_VOLTAGE            SDHC_Bus_Voltage_3_3
#define MAX32666_SD_CLK_DIV                0x0b0  // Maximum divide ratio
#define MAX32666_SD_INIT_RETRY             10

// MAX32666 Hardware semaphores
#define MAX32666_SEMAPHORE_PRINT           1

/*** MAX78000 AUDIO ***/
// MAX78000 AUDIO PINS
#define MAX78000_AUDIO_CNN_BOOST_PIN       {MXC_GPIO2, MXC_GPIO_PIN_5, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_AUDIO_AUDIO_OSC_PIN       {MXC_GPIO0, MXC_GPIO_PIN_31, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_AUDIO_HOST_INT_PIN        {MXC_GPIO0, MXC_GPIO_PIN_12, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_AUDIO_LED_RED_PIN         {MXC_GPIO2, MXC_GPIO_PIN_0, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}
#define MAX78000_AUDIO_LED_GREEN_PIN       {MXC_GPIO2, MXC_GPIO_PIN_1, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}
#define MAX78000_AUDIO_LED_BLUE_PIN        {MXC_GPIO2, MXC_GPIO_PIN_2, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_AUDIO_HOST_IO_PIN         {MXC_GPIO1, MXC_GPIO_PIN_6, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

#define MAX78000_AUDIO_EXPANDER_IN_PIN     {MXC_GPIO1, MXC_GPIO_PIN_7, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO} // TODO

#define MAX78000_AUDIO_PMIC_MPC0_PIN       {MXC_GPIO3, MXC_GPIO_PIN_0, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

#define MAX78000_AUDIO_BUTTON_INT_PIN      {MXC_GPIO0, MXC_GPIO_PIN_2, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

#define MAX78000_AUDIO_DEBUG_SEL_PIN       {MXC_GPIO2, MXC_GPIO_PIN_2, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

// MAX78000 AUDIO QSPI
#define MAX78000_AUDIO_QSPI_ID             MXC_SPI0
#define MAX78000_AUDIO_QSPI_DMA_CHANNEL    1
#define MAX78000_AUDIO_QSPI_DMA_IRQ        DMA1_IRQn
#define MAX78000_AUDIO_QSPI_DMA_IRQ_HAND   DMA1_IRQHandler

/*** MAX78000 VIDEO ***/
// MAX78000 VIDEO PINS
#define MAX78000_VIDEO_FLASH_PIN           {MXC_GPIO0, MXC_GPIO_PIN_19, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_PULL_DOWN, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_VIDEO_CAMERA_PIN          {MXC_GPIO0, MXC_GPIO_PIN_3, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_VIDEO_HOST_INT_PIN        {MXC_GPIO0, MXC_GPIO_PIN_12, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_VIDEO_LED_RED_PIN         {MXC_GPIO2, MXC_GPIO_PIN_0, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}
#define MAX78000_VIDEO_LED_GREEN_PIN       {MXC_GPIO2, MXC_GPIO_PIN_1, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}
#define MAX78000_VIDEO_LED_BLUE_PIN        {MXC_GPIO2, MXC_GPIO_PIN_2, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}

#define MAX78000_VIDEO_HOST_IO_PIN         {MXC_GPIO1, MXC_GPIO_PIN_6, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

#define MAX78000_VIDEO_EXPANDER_IN_PIN     {MXC_GPIO1, MXC_GPIO_PIN_7, MXC_GPIO_FUNC_OUT, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO} // TODO

#define MAX78000_VIDEO_PMIC_MPC2_PIN       {MXC_GPIO3, MXC_GPIO_PIN_0, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO
#define MAX78000_VIDEO_PMIC_MPC1_PIN       {MXC_GPIO3, MXC_GPIO_PIN_1, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

#define MAX78000_VIDEO_BUTTON_INT_PIN      {MXC_GPIO0, MXC_GPIO_PIN_2, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

#define MAX78000_VIDEO_DEBUG_SEL_PIN       {MXC_GPIO2, MXC_GPIO_PIN_2, MXC_GPIO_FUNC_IN, MXC_GPIO_PAD_NONE, MXC_GPIO_VSSEL_VDDIO}  // TODO

// MAX78000 VIDEO QSPI
#define MAX78000_VIDEO_QSPI_ID             MXC_SPI0
#define MAX78000_VIDEO_QSPI_DMA_CHANNEL    1
#define MAX78000_VIDEO_QSPI_DMA_IRQ        DMA1_IRQn
#define MAX78000_VIDEO_QSPI_DMA_IRQ_HAND   DMA1_IRQHandler

// MAX78000 VIDEO CAMERA
#define MAX78000_VIDEO_CAMERA_DMA_CHANNEL  0
#define MAX78000_VIDEO_CAMERA_DMA_IRQ      DMA0_IRQn
#define MAX78000_VIDEO_CAMERA_DMA_IRQ_HAND DMA0_IRQHandler


//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef enum {
    QSPI_TYPE_NO_DATA = 0,
    QSPI_TYPE_RESPONSE_VIDEO_DATA ,
    QSPI_TYPE_RESPONSE_VIDEO_RESULT,
    QSPI_TYPE_RESPONSE_AUDIO_RESULT
} teQspiDataType;

typedef struct __attribute__((packed)) {
    uint32_t start_symbol;
    uint32_t data_len;
    uint8_t data_type;
} qspi_header_t;


#endif /* _MAXCAM_DEFINTIIONS_H_ */