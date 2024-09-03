# Simple_TFT_LCD_STM32

The programs allow to operate ST7735S LCD TFT Display via Nucleo L476RGT6 board, using SPI and DMA.  
They can be easily migrated to another STM32-type board and used in simple projects.  

The code is a part of my STM32 training with an online course from polish website Forbot.pl, but I find it nice so I decided to share it. It's mainly focused on being easy to undertand and as simple as possible.  

There are two versions, v1 takes less RAM but it's slower, v2 uses buffering to draw images faster but takes up way more memory space.  

The project uses 5 pins - MOSI, SCK and CS for SPI, DC for data/command marking, and RST for resetting.  

Depending on the device, you may also need to connect display's LED pin to 3.3V.

The library is in files lcd.h and lcd.c, the program also uses image.c to store image data.  

