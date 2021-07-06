 //#include "stm32f0xx.h"
 #include "stm32f030x8.h"
 #include "lcd_i2c.h"
 #include "i2c.h"

/*void	delay_us(unsigned us)
{
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; 
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;   
	unsigned  us_count_tic =  us * 48;
	DWT->CYCCNT = 0U;
	while(DWT->CYCCNT < us_count_tic);
}*/

 void	Delay_us(unsigned us);
 void	Delay		(unsigned ms);
 extern unsigned sysTimer;

LiquidCrystal_I2C_Def lcdi2c;


 
	

void 	SetLcdI2cAdr(char Adr)
 {
   lcdi2c.Addr = Adr;
 }
void LCDI2C_init(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows)
{
  lcdi2c.Addr = lcd_Addr;
  lcdi2c.cols = lcd_cols;
  lcdi2c.rows = lcd_rows;
  lcdi2c.backlightval = LCD_NOBACKLIGHT;

  I2C_Initialization();		//init_I2C1(); // Wire.begin();
  lcdi2c.displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
  LCDI2C_begin(lcd_cols, lcd_rows);
}





/************ low level data pushing commands **********/

void LCDI2C_expanderWrite(uint8_t _data)
{
	I2C_Write_Sym(lcdi2c.Addr,_data | lcdi2c.backlightval);
}

void LCDI2C_pulseEnable(uint8_t _data)
	{
	LCDI2C_expanderWrite(_data | EnPin);	// En high
	Delay_us(1);		// enable pulse must be >450ns
	LCDI2C_expanderWrite(_data & ~EnPin);	// En low
	Delay_us(50);		// commands need > 37us to settle
	}


void LCDI2C_write4bits(uint8_t value) {
	LCDI2C_expanderWrite(value);
	LCDI2C_pulseEnable(value);
}


// write either command or data
void LCDI2C_send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
	LCDI2C_write4bits ((highnib)  | mode);
	LCDI2C_write4bits ((lownib)   | mode);
}
void LCDI2C_write(uint8_t value){
	LCDI2C_send(value, RgPin);
}
void	TrcChar(char ch)
 {
	 LCDI2C_write((byte)ch);
 }

//Функция передачи строки 
void LCDI2C_write_String(char* str) {
  uint8_t i=0;
  while(str[i])
  {
    LCDI2C_write(str[i]);
    i++;
  }
}

/*********** mid level commands, for sending data/cmds */

void LCDI2C_command(uint8_t value) {
	LCDI2C_send(value, 0);
}

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).




void LCDI2C_begin(uint8_t cols, uint8_t lines) {//, uint8_t dotsize) {
	if (lines > 1) {
		lcdi2c.displayfunction |= LCD_2LINE;
	}
	lcdi2c.numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
/*	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}*/

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	Delay(50);

	// Now we pull both RS and R/W low to begin commands
	LCDI2C_expanderWrite(lcdi2c.backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	Delay(1000);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	  // we start in 8bit mode, try to set 4 bit mode
   LCDI2C_write4bits(0x03 << 4);
   Delay(5); // wait min 4.1ms

   // second try
   LCDI2C_write4bits(0x03 << 4);
   Delay(5); // wait min 4.1ms

   // third go!
   LCDI2C_write4bits(0x03 << 4);
   Delay(1);

   // finally, set to 4-bit interface
   LCDI2C_write4bits(0x02 << 4);


	// set # lines, font size, etc.
	LCDI2C_command(LCD_FUNCTIONSET | lcdi2c.displayfunction | 2);  // 2 - second page character set

	// turn the display on with no cursor or blinking default
	lcdi2c.displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	LCDI2C_display();

	// clear it off
	LCDI2C_clear();

	// Initialize to default text direction (for roman languages)
	lcdi2c.displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

	// set the entry mode
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);

	LCDI2C_home();

}

/********** high level commands, for the user! */
void LCDI2C_clear(){
	LCDI2C_command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	Delay_us(3000);  // this command takes a long time!
}

void LCDI2C_home(){
	LCDI2C_command(LCD_RETURNHOME);  // set cursor position to zero
	Delay_us(3000);  // this command takes a long time!
}

void LCDI2C_setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > lcdi2c.numlines ) {
		row = lcdi2c.numlines-1;    // we count rows starting w/0
	}
	LCDI2C_command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCDI2C_noDisplay() {
	lcdi2c.displaycontrol &= ~LCD_DISPLAYON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

void LCDI2C_display() {
	lcdi2c.displaycontrol |= LCD_DISPLAYON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turns the underline cursor on/off
void LCDI2C_noCursor() {
	lcdi2c.displaycontrol &= ~LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}
void LCDI2C_cursor() {
	lcdi2c.displaycontrol |= LCD_CURSORON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// Turn on and off the blinking cursor
void LCDI2C_noBlink() {
	lcdi2c.displaycontrol &= ~LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

void LCDI2C_blink() {
	lcdi2c.displaycontrol |= LCD_BLINKON;
	LCDI2C_command(LCD_DISPLAYCONTROL | lcdi2c.displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCDI2C_scrollDisplayLeft(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCDI2C_scrollDisplayRight(void) {
	LCDI2C_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCDI2C_leftToRight(void) {
	lcdi2c.displaymode |= LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This is for text that flows Right to Left
void LCDI2C_rightToLeft(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYLEFT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'right justify' text from the cursor
void LCDI2C_autoscroll(void) {
	lcdi2c.displaymode |= LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// This will 'left justify' text from the cursor
void LCDI2C_noAutoscroll(void) {
	lcdi2c.displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	LCDI2C_command(LCD_ENTRYMODESET | lcdi2c.displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCDI2C_createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	LCDI2C_command(LCD_SETCGRAMADDR | (location << 3));
	int i;
	for (i=0; i<8; i++) {
		LCDI2C_write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void LCDI2C_noBacklight(void) {
	lcdi2c.backlightval=LCD_NOBACKLIGHT;
	LCDI2C_expanderWrite(0);
}

void LCDI2C_backlight(void) {
	lcdi2c.backlightval=LCD_BACKLIGHT;
	LCDI2C_expanderWrite(0);
}





// Alias functions

void LCDI2C_cursor_on(){
	LCDI2C_cursor();
}

void LCDI2C_cursor_off(){
	LCDI2C_noCursor();
}

void LCDI2C_blink_on(){
	LCDI2C_blink();
}

void LCDI2C_blink_off(){
	LCDI2C_noBlink();
}

void LCDI2C_load_custom_character(uint8_t char_num, uint8_t *rows){
		LCDI2C_createChar(char_num, rows);
}

void LCDI2C_setBacklight(uint8_t new_val){
	if(new_val)
			lcdi2c.backlightval = LCD_BACKLIGHT;		// turn backlight on
	else
			lcdi2c.backlightval = LCD_NOBACKLIGHT;
		
	}

// Define our numbers 0 thru 9
// 254 is blank and 255 is the "Full Block"
uint8_t custChar[8][8] = {
  {31, 31, 31, 0, 0, 0, 0, 0},      // Small top line - 0
  {0, 0, 0, 0, 0, 31, 31, 31},      // Small bottom line - 1
  { 0x1f,
    0x0,
    0x0,
    0x0,                         // This shows an alternative
    0,                         // way of defining a custome character,
    0,                         // a bit more 'visually' perhaps?
    0,
    0x1f,
  },
  //{31, 0, 0, 0, 0, 0, 0, 31},     // Small lines top and bottom -2
  {0, 0, 0, 0, 0, 0,  0, 31},       // Thin bottom line - 3
  {31, 31, 31, 31, 31, 31, 15, 7},  // Left bottom chamfer full - 4
  {28, 30, 31, 31, 31, 31, 31, 31}, // Right top chamfer full -5
  {31, 31, 31, 31, 31, 31, 30, 28}, // Right bottom chamfer full -6
  {7, 15, 31, 31, 31, 31, 31, 31},  // Left top chamfer full -7
};

uint8_t bigNums[10][6] = {
  {7, 0, 5, 4, 1, 6},         //0
  {0, 5, 254, 1, 255, 1},     //1
  {0, 2, 5, 7, 3, 1},         //2
  {0, 2, 5, 1, 3, 6},         //3
  {7, 3, 255, 254, 254, 255}, //4
  {7, 2, 0, 1, 3, 6},         //5
  {7, 2, 0, 4, 3, 6},         //6
  {0, 0, 5, 254, 7, 254},   //7
  {7, 2, 5, 4, 3, 6},         //8
  {7, 2, 5, 1, 3, 6},         //9
};
void	CreateBigNum()
{
 for (int cnt = 0; cnt < sizeof(custChar) / 8; cnt++) {
    LCDI2C_createChar(cnt, custChar[cnt]);
  }
}
// -----------------------------------------------------------------
// Print big number over 2 lines, 3 colums per half digit
// -----------------------------------------------------------------
void printBigNum(int number, int startCol, int startRow) {

  // Position cursor to requested position (each char takes 3 cols plus a space col)
  LCDI2C_setCursor(startCol, startRow);

  // Each number split over two lines, 3 chars per line. Retrieve character
  // from the main array to make working with it here a bit easier.
  uint8_t thisNumber[6];
  for (int cnt = 0; cnt < 6; cnt++) {
    thisNumber[cnt] = bigNums[number][cnt];
  }

  // First line (top half) of digit
  for (int cnt = 0; cnt < 3; cnt++) {
    LCDI2C_write((char)thisNumber[cnt]);
  }

  // Now position cursor to next line at same start column for digit
  LCDI2C_setCursor(startCol, startRow + 1);

  // 2nd line (bottom half)
  for (int cnt = 3; cnt < 6; cnt++) {
    LCDI2C_write((char)thisNumber[cnt]);
  }
}

