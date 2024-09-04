/*
 * lcd.c
 *
 *  Created on: Aug 24, 2024
 *      Author: Mikolaj
 */

#include "lcd.h"
#include "spi.h"


//Adresses used to talk with the LCD, provided in its' datasheet
#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

//Ideally, these should stay at 0, but it depends on your device
//Change them if the display is displaced
#define LCD_OFFSET_X  0
#define LCD_OFFSET_Y  0

//This macro turns 1 byte command identifier into 2 byte message, marked as command
//This is needed only in commands which take arguments
#define CMD(x)					((x) | 0x100)


//The two functions below send a byte to LCD, and mark it as data or command via DC pin
static void lcd_cmd(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET); //command
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi2, &cmd, sizeof(cmd));
}

static void lcd_data(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); //data
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi2, &data, sizeof(data));
}

//The function below sends data to LCD, with differentiation between data and commands
static void lcd_send(uint16_t value)
{
	if (value & 0x100) //bit 9 is 1 - this is a command
	{
		lcd_cmd(value); //most important byte is skipped, as it's only used to mark
						//a command
	}
	else
	{	//bit 9 is 0 - this is a value for a command that was sent before
		lcd_data(value);
	}
}

//Init table - full of stuff that is not really needed to know in order
//to operate the LCD, only to turn it on properly

static const uint16_t init_table[] = {
  CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
  CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
  CMD(ST7735S_INVCTR), 0x07,
  CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
  CMD(ST7735S_PWCTR2), 0xc5,
  CMD(ST7735S_PWCTR3), 0x0a, 0x00,
  CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
  CMD(ST7735S_PWCTR5), 0x8a, 0xee,
  CMD(ST7735S_VMCTR1), 0x0e,
  CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
  CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
  CMD(0xf0), 0x01,
  CMD(0xf6), 0x00,
  CMD(ST7735S_COLMOD), 0x05,
  CMD(ST7735S_MADCTL), 0xa0,

  //Just commands and data and so on
};

void lcd_init()
{
  int i;

  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  //Delays needed because the producer said so
  HAL_Delay(100); //They can probably be shorter but I haven't checked
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_Delay(100);

  //Sending all that stuff from init table
  for (i = 0; i < sizeof(init_table) / sizeof(uint16_t); i++)
  {
    lcd_send(init_table[i]);
  }

  HAL_Delay(200);

  //Waking up and turning on
  lcd_cmd(ST7735S_SLPOUT);
  HAL_Delay(120);

  lcd_cmd(ST7735S_DISPON);
}

//This function splits 16 bit data into 8 bits data, as the word length is 8 bits
static void lcd_data16(uint16_t value)
{
	lcd_data(value >> 8); //MSB first
	lcd_data(value);
}

static void lcd_set_window(int x, int y, int width, int height)
{
	//Here the defined offsets can be used
	lcd_cmd(ST7735S_CASET); //Sets start and stop columns
	lcd_data16(LCD_OFFSET_X + x);
	lcd_data16(LCD_OFFSET_X + x + width - 1);

	lcd_cmd(ST7735S_RASET); //Sets start and stop rows
	lcd_data16(LCD_OFFSET_Y + y);
	lcd_data16(LCD_OFFSET_Y + y + height- 1);
}

//This function fills a box with a color
void lcd_fill_box(int x, int y, int width, int height, uint16_t color)
{
	lcd_set_window(x, y, width, height); //set working area

	lcd_cmd(ST7735S_RAMWR); //send command and then colors
	for (int i = 0; i < width * height; i++)
	{
		lcd_data16(color);
	}
}

//To write a single pixel you draw a 1x1 box - it's a bit slow but it works
void lcd_put_pixel(int x, int y, uint16_t color)
{
	lcd_fill_box(x, y, 1, 1, color);
}

//This function draws an image, for example from the array in image.c, provided
//in the example -- instead of sending pixel by pixel, you send all the image
//in one transmit command, which is faster
void lcd_draw_image(int x, int y, int width, int height, const uint8_t* data)
{
	lcd_set_window(x, y, width, height); //set working area

	lcd_cmd(ST7735S_RAMWR);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi2, (uint8_t*)data, width * height * 2);

}

//This function fills the display with color white
void lcd_clear()
{
	lcd_fill_box(0, 0, WIDTH, HEIGHT, 0xFFFF);
}

//Deselecting the device after transfer
void lcd_transfer_done()
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

//Checks SPI state - currently unused, but may be helpful to check if you
//can proceed with more communicates
bool lcd_is_busy()
{
	if (HAL_SPI_GetState(&hspi2) == HAL_SPI_STATE_BUSY)
	{
		return true;
	}
	else
	{
		return false;
	}
}
