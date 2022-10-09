#include "LCD_GFX.h"
#include "ST7735.h"

uint8_t x_max = 160;
uint8_t y_max = 128;

/******************************************************************************
* Local Functions
******************************************************************************/



/******************************************************************************
* Global Functions
******************************************************************************/

/**************************************************************************//**
* @fn			uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
* @brief		Convert RGB888 value to RGB565 16-bit color data
* @note
*****************************************************************************/
uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue)
{
return ((((31*(red+4))/255)<<11) | (((63*(green+2))/255)<<5) | ((31*(blue+4))/255));
}

/**************************************************************************//**
* @fn			void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color)
* @brief		Draw a single pixel of 16-bit rgb565 color to the x & y coordinate
* @note
*****************************************************************************/
void LCD_drawPixel(uint8_t x, uint8_t y, uint16_t color) {
LCD_setAddr(x,y,x,y);
SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor)
* @brief		Draw a character starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawChar(uint8_t x, uint8_t y, uint16_t character, uint16_t fColor, uint16_t bColor)
{
uint16_t row = character - 0x20;		//Determine row of ASCII table starting at space
int i, j;
if ((LCD_WIDTH-x>7)&&(LCD_HEIGHT-y>7))
{
for(i=0;i<5;i++)
{
uint8_t pixels = ASCII[row][i]; //Go through the list of pixels
for(j=0;j<8;j++){
if ((pixels>>j)&1==1){
LCD_drawPixel(x+i,y+j,fColor);
}
else 
{
LCD_drawPixel(x+i,y+j,bColor);
}
}
}
}
}


/******************************************************************************
* LAB 4 TO DO. COMPLETE THE FUNCTIONS BELOW.
* You are free to create and add any additional files, libraries, and/or
*  helper function. All code must be authentically yours.
******************************************************************************/

/**************************************************************************//**
* @fn			void LCD_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color)
* @brief		Draw a colored circle of set radius at coordinates
* @note
*****************************************************************************/

void drawCircle(int xc, int yc, int r, int color)
{
for(int i = r; i >= 0; i--)
{
int x = 0, y = i;
int d = 3 - 2 * i;
drawSevenPixels(xc, yc, x, y, color);

while (y >= x)
{

x++;
if (d > 0)
{
y--;
d = d + 4 * (x - y) + 10;
}
else
d = d + 4 * x + 6;
drawSevenPixels(xc, yc, x, y, color);
Delay_ms(50);
}
}
}

void drawSemiCircle(int xc, int yc, int r)
{
for(int i = r; i >= 0; i--)
{
int x = 0, y = i;
int d = 3 - 2 * i;
while (y >= x)
{
x++;
if (d > 0)

{
y--;
d = d + 4 * (x - y) + 10;
}
else
d = d + 4 * x + 6;
drawLeftFourPixels(xc, yc, x, y, BLUE);
Delay_ms(50);
}
}

for(int i = r; i >= 0; i--)
{
int x = 0, y = i;
int d = 3 - 2 * i;
while (y >= x)
{
x++;
if (d > 0)
{
y--;
d = d + 4 * (x - y) + 10;
}
else
d = d + 4 * x + 6;
drawRightFourPixels(xc, yc, x, y, CYAN);
Delay_ms(50);
}
}

LCD_drawLine(80, yc - r - 1, 80, yc + r + 1, RED);
LCD_drawLine(79, yc - r - 1, 79, yc + r + 1, BLUE);
}

void drawSevenPixels(int xc, int yc, int x, int y, int color)
{
LCD_drawPixel(xc+x, yc+y, color);
LCD_drawPixel(xc-x, yc+y, color);
LCD_drawPixel(xc+x, yc-y, color);
LCD_drawPixel(xc-x, yc-y, color);
LCD_drawPixel(xc+y, yc+x, color);
LCD_drawPixel(xc-y, yc+x, color);
LCD_drawPixel(xc+y, yc-x, color);
LCD_drawPixel(xc-y, yc-x, color);
}

void drawLeftFourPixels(int xc, int yc, int x, int y, int color)
{
LCD_drawPixel(xc-x, yc+y, color);
LCD_drawPixel(xc-x, yc-y, color);
LCD_drawPixel(xc-y, yc+x, color);
LCD_drawPixel(xc-y, yc-x, color);
LCD_drawPixel(xc-2, yc, color);
LCD_drawPixel(xc-1, yc, color);
}

void drawRightFourPixels(int xc, int yc, int x, int y, int color)
{
LCD_drawPixel(xc+x, yc+y, color);
LCD_drawPixel(xc+x, yc-y, color);
LCD_drawPixel(xc+y, yc+x, color);
LCD_drawPixel(xc+y, yc-x, color);
LCD_drawPixel(xc+2, yc, color);
}

void LCD_drawSemiCircle(uint8_t x0, uint8_t y0, uint8_t radius,uint16_t color1, uint16_t color2)
{
int r = radius;
int start, finish;

start = 0;
finish = x_max / 2;

for (int i = start; i <= finish; i++)
for (int j= 0; j <= y_max; j++)
if ((x0 - i) * (x0 - i) + (y0 - j) * (y0 - j) <= (r * r))
LCD_drawPixel(i, j, color1);

start = x_max / 2;
finish = x_max;

for (int i = start; i <= finish; i++)
for (int j= 0; j <= y_max; j++)
if ((x0 - i) * (x0 - i) + (y0 - j) * (y0 - j) <= (r * r))
LCD_drawPixel(i, j, color2);

LCD_drawLine(80, y0 - radius - 1, 80, y0 + radius + 1, RED); //middle line
}

/**************************************************************************//**
* @fn			void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t c)
* @brief		Draw a line from and to a point with a color
* @note
*****************************************************************************/
void LCD_drawLine(short x0,short y0,short x1,short y1,uint16_t color)
{
for (int i = x0; i <= x1; i++)
for (int j = y0; j <= y1; j++)
LCD_drawPixel(i, j, color);
}

/**************************************************************************//**
* @fn			void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
* @brief		Draw a colored block at coordinates
* @note
*****************************************************************************/
void LCD_drawBlock(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1,uint16_t color)
{
LCD_setAddr(x0, y0, x1, y1);
for (int j = x0; j <= x1; j++)
for (int k = y0; k <= y1; k++)
SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_setScreen(uint16_t color)
* @brief		Draw the entire screen to a color
* @note
*****************************************************************************/
void LCD_setScreen(uint16_t color)
{
LCD_setAddr(0, 0, x_max, y_max);

for (int j = 0; j < x_max; j++)
for (int k = 0; k < y_max; k++)
SPI_ControllerTx_16bit(color);
}

/**************************************************************************//**
* @fn			void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
* @brief		Draw a string starting at the point with foreground and background colors
* @note
*****************************************************************************/
void LCD_drawString(uint8_t x, uint8_t y, char* str, uint16_t fg, uint16_t bg)
{

char letter;

for(int i=0; str[i]!= '\0'; i++){
letter = str[i];
LCD_drawChar(x, y, (int)letter, fg, bg);
x += 5;
}
}
