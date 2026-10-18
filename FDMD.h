#ifndef FDMD_H
#define FDMD_H

#include <Arduino.h>
#include <SPI.h>
#include "fonts\Font12x16.h" 

// ابعاد استاندارد ماژول P10 تک رنگ
#define DMD_PIXELS_ACROSS  32
#define DMD_PIXELS_DOWN    16
#define DMD_RAM_SIZE_BYTES 64 // 32 * 16 / 8

// حالت‌های گرافیکی
enum DMDGraphicsMode {
    GRAPHICS_NORMAL,
    GRAPHICS_INVERSE,
    GRAPHICS_TOGGLE,
    GRAPHICS_OR,
    GRAPHICS_NOR
};

enum ScrollDirection {
    SCROLL_LEFT,
    SCROLL_RIGHT
};

struct FarsiChar {
    uint16_t uch;    
    int      row;   
};

class FDMD {
public:
    FDMD(byte panelsWide, byte panelsHigh, 
         uint8_t nOEPin, uint8_t aPin, uint8_t bPin, 
         uint8_t clkPin, uint8_t latPin, uint8_t rDataPin);
    ~FDMD();

    void begin();
	void setBrightness(uint8_t brightness);
    void clearScreen(bool normal = true);
    void writePixel(unsigned int bX, unsigned int bY, DMDGraphicsMode mode, bool pixel);
    void drawLine(int x1, int y1, int x2, int y2, DMDGraphicsMode mode = GRAPHICS_NORMAL);
    void scanDisplayBySPI();
    inline int getWidth()  const { return DisplaysWide * DMD_PIXELS_ACROSS; }
    inline int getHeight() const { return DisplaysHigh * DMD_PIXELS_DOWN; }
	
	int drawChar12x16(int bX, int bY, uint8_t charIndex, DMDGraphicsMode mode);
	static int getFarsiRow(uint16_t Uch);
	static int parseFarsiString(const char* str, FarsiChar* out, int maxLen);
	void drawFarsi(int x, int y, const char* str);
	
	int getFarsiStringWidth(const char* str);
	void scrollFarsi(int y, const char* str, ScrollDirection dir, int speedMs);
	
	

private:
    uint8_t _aPin, _bPin, _latPin, _nOEPin, _clkPin, _rDataPin;
    byte DisplaysWide, DisplaysHigh, DisplaysTotal;
    byte *bDMDScreenRAM = nullptr;
    SPIClass *vspi = nullptr;

    uint32_t spiClk = 16000000; 
    byte bDMDByte = 0;
    int row1, row2, row3;
	const uint8_t *Font = nullptr;
	uint8_t _brightness = 100;

    inline void latchShiftRegToOutput() {
        digitalWrite(_latPin, HIGH);
        digitalWrite(_latPin, LOW);
    }
    inline void oeRowsOff() { ledcWrite(_nOEPin, 0);   }  
    inline void oeRowsOn()  { ledcWrite(_nOEPin, _brightness);  }

    void lightRow(byte row);
};

#endif
