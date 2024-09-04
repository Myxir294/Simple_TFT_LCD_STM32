/*
 * lcd.h
 *
 *  Created on: Aug 24, 2024
 *      Author: Mikolaj
 */

//The file contains prototypes for functions used to drive LCD display ST7735S via SPI
//Disclaimer in case of displaced image - depending on your part type, drawing functions
//may need additional information about offseting data

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>
#include <stdbool.h>

//Definitions of colors in 2 byte format
//You can add new as you will
#define BLACK     0x0000
#define RED       0xf800
#define GREEN     0x07e0
#define BLUE      0x001f
#define YELLOW    0xffe0
#define MAGENTA   0xf81f
#define CYAN      0x07ff
#define WHITE     0xffff
#define PINK      0xf81f

//Resolution of LCD - 128 x 160
#define WIDTH 160
#define HEIGHT 128

//Draws a box starting at (x,y) coordinates, of given width and height and color
//x and y should be smaller then the max defined
void lcd_fill_box(int x, int y, int width, int height, uint16_t color);

//Always run the function below before starting work with the LCD
void lcd_init();

//Draws a pixel at (x,y), of given color
void lcd_put_pixel(int x, int y, uint16_t color);

//Draws a image starting at (x,y) coordinates, of given width and height
//x and y should be smaller then the max defined
//Image pixels data should be stored in 2-byte format array (see the example)
//You can generate such data from image using https://lvgl.io/tools/imageconverter
//with settings: LVGL v8, color format - true colour, output - C array,
//then you need to choose an array that starts with comments matching the one in image.c
void lcd_draw_image(int x, int y, int width, int height, const uint8_t* data);

//Clears display by drawing white all over the LCD
void lcd_clear();

//Changes CS pin state after transferring data, used for SPI callback
void lcd_transfer_done();

//Checks SPI state - currently unused, but may be helpful to check if you
//can proceed with more communicates
bool lcd_is_busy();

#endif /* INC_LCD_H_ */
