#include "mbed.h"
#include "Arduino.h"
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h>
#include <FreeMonoBoldOblique24pt7b.h>    // Core graphics library
//#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeMono12pt7b.h>
#include <FreeSansBoldOblique12pt7b.h>
MCUFRIEND_kbv tft;

uint8_t Orientation = 1;  

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


void print_emergencia ()
{
    
    
    tft.fillScreen(BLACK);  // Fundo do Display
    tft.fillRect(15,55,300,100,RED); 
    tft.setTextColor(YELLOW);
    tft.setFont(&FreeMonoBoldOblique24pt7b);
    tft.setCursor(25, 120); // Orientação X,Y
    tft.println("EMERGENCIA");


}

//****************************************************************************//



void setup(void)
{

    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    
    print_emergencia();
    delay(1000);
}

void loop()
{

}
