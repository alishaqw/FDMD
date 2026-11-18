//FDMD.cpp
#include "FDMD.h"


const unsigned char FAmap[50][6] = 
{
    // Isolated, Initial(next only), Final(prev only), Medial(both), ConnectRight, ConnectLeft
    {0xB0, 0x8C, 0xB0, 0x8C, 1, 1},
    {0x81, 0x03, 0x81, 0x03, 1, 1},
    {0x8D, 0x93, 0x94, 0x93, 1, 1},
    {0x8E, 0x8E, 0x8E, 0x8E, 1, 0},
    {0x90, 0x01, 0x90, 0x01, 1, 1},
    {0x98, 0x96, 0x98, 0x96, 0, 1},
    {0x99, 0x99, 0x02, 0x02, 1, 0},
    {0xC3, 0xC3, 0x01, 0x01, 1, 0},
    {0xC4, 0xC4, 0xC4, 0xC4, 0, 0},
    {0xC5, 0xC5, 0x03, 0x03, 1, 0},
    {0xC6, 0xC6, 0xC6, 0xC6, 0, 0},
    {0x9A, 0x9A, 0x00, 0x00, 1, 0},
    {0x9B, 0x04, 0x9B, 0x04, 1, 1},
    {0xC9, 0xFE, 0xE9, 0x95, 1, 1},
    {0x9C, 0x05, 0x9C, 0x05, 1, 1},
    {0x9D, 0x06, 0x9D, 0x06, 1, 1},
    {0x9E, 0x07, 0x08, 0x07, 1, 1},
    {0x9F, 0x09, 0x0A, 0x09, 1, 1},
    {0xA0, 0x0B, 0x0C, 0x0B, 1, 1},
    {0xA1, 0xA1, 0xA1, 0xA1, 1, 0},
    {0xA2, 0xA2, 0xA2, 0xA2, 1, 0},
    {0xA3, 0xA3, 0xA3, 0xA3, 1, 0},
    {0xA4, 0xA4, 0xA4, 0xA4, 1, 0},
    {0xA5, 0x8A, 0xA5, 0x8A, 1, 1},
    {0xA6, 0x0E, 0xA6, 0x0E, 1, 1},
    {0xA7, 0x0F, 0xA7, 0x0F, 1, 1},
    {0xA8, 0x1A, 0xA8, 0x1A, 1, 1},
    {0xD7, 0xD7, 0xD7, 0xD7, 0, 0},
    {0xA9, 0xA9, 0xA9, 0xA9, 1, 1},
    {0xAA, 0xAA, 0xAA, 0xAA, 1, 1},
    {0xAB, 0x1B, 0x89, 0x88, 1, 1},
    {0xAC, 0x85, 0x7F, 0x84, 1, 1},
    {0xAD, 0xAD, 0xAD, 0xAD, 1, 1},
    {0xAE, 0x80, 0xAE, 0x80, 1, 1},
    {0xAF, 0x83, 0xAF, 0x83, 1, 1},
    {0xB0, 0x8C, 0xB0, 0x8C, 1, 1},
    {0xE0, 0xE0, 0xE0, 0xE0, 0, 0},
    {0xB1, 0x86, 0xB1, 0x86, 1, 1},
    {0xE2, 0xE2, 0xE2, 0xE2, 0, 0},
    {0xB2, 0x82, 0xB2, 0x82, 1, 1},
    {0xB3, 0x87, 0xB3, 0x87, 1, 1},
    {0xB4, 0x0D, 0x8B, 0x91, 1, 1},
    {0xB5, 0xB5, 0xB5, 0xB5, 1, 0},
    {0xE7, 0xE7, 0xE7, 0xE7, 0, 0},
    {0xE8, 0xE8, 0xE8, 0xE8, 0, 0},
    {0xE9, 0xE9, 0xE9, 0xE9, 0, 0},
    {0xFE, 0xFE, 0xFE, 0xFE, 0, 0},
    {0xFF, 0xFF, 0xFF, 0xFF, 0, 0},
    {0xEC, 0xC0, 0xA0, 0xC0, 1, 1},
    {0xB6, 0x8F, 0x92, 0x8F, 1, 1}
};



static const byte bPixelLookupTable[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

FDMD::FDMD(byte panelsWide, byte panelsHigh, 
           uint8_t nOEPin, uint8_t aPin, uint8_t bPin, 
           uint8_t clkPin, uint8_t latPin, uint8_t rDataPin)
    : DisplaysWide(panelsWide), DisplaysHigh(panelsHigh),
      _nOEPin(nOEPin), _aPin(aPin), _bPin(bPin),
      _clkPin(clkPin), _latPin(latPin), _rDataPin(rDataPin)
{
    DisplaysTotal = DisplaysWide * DisplaysHigh;
    row1 = DisplaysTotal << 4;
    row2 = DisplaysTotal << 5;
    row3 = ((DisplaysTotal << 2) * 3) << 2;
}

FDMD::~FDMD() {
    if (bDMDScreenRAM) free(bDMDScreenRAM);
    if (vspi) delete vspi;
}

void FDMD::begin() {
    bDMDScreenRAM = (byte *)malloc(DisplaysTotal * DMD_RAM_SIZE_BYTES);

    vspi = new SPIClass(VSPI);
    vspi->begin(_clkPin, -1, _rDataPin, -1);

    pinMode(_aPin, OUTPUT);
    pinMode(_bPin, OUTPUT);
    pinMode(_latPin, OUTPUT);
    //pinMode(_nOEPin, OUTPUT);
    ledcAttach(_nOEPin, 15000, 8);
	
    digitalWrite(_aPin, LOW);
    digitalWrite(_bPin, LOW);
    digitalWrite(_latPin, LOW);
    //digitalWrite(_nOEPin, LOW);
	oeRowsOff();

    clearScreen(true);
    bDMDByte = 0;
}

void FDMD::setBrightness(uint8_t brightness) {
    _brightness = brightness;
}

void FDMD::clearScreen(bool normal) {
    if (bDMDScreenRAM) {
        memset(bDMDScreenRAM, normal ? 0xFF : 0x00, DMD_RAM_SIZE_BYTES * DisplaysTotal);
    }
}

void FDMD::writePixel(unsigned int bX, unsigned int bY, DMDGraphicsMode mode, bool pixel) {
    if (bX >= (unsigned int)(DMD_PIXELS_ACROSS * DisplaysWide) || 
        bY >= (unsigned int)(DMD_PIXELS_DOWN * DisplaysHigh)) {
        return;
    }

    byte panel = (bX / DMD_PIXELS_ACROSS) + (DisplaysWide * (bY / DMD_PIXELS_DOWN));
    bX = (bX % DMD_PIXELS_ACROSS) + (panel << 5);
    bY = bY % DMD_PIXELS_DOWN;

    unsigned int ptr = (bX >> 3) + bY * (DisplaysTotal << 2);
    byte lookup = bPixelLookupTable[bX & 0x07];

    switch (mode) {
        case GRAPHICS_NORMAL:
            if (pixel) bDMDScreenRAM[ptr] &= ~lookup;
            else       bDMDScreenRAM[ptr] |= lookup;
            break;
        case GRAPHICS_INVERSE:
            if (!pixel) bDMDScreenRAM[ptr] &= ~lookup;
            else        bDMDScreenRAM[ptr] |= lookup;
            break;
        case GRAPHICS_TOGGLE:
            if (pixel)  bDMDScreenRAM[ptr] ^= lookup;
            break;
        case GRAPHICS_OR:
            if (pixel)  bDMDScreenRAM[ptr] &= ~lookup;
            break;
        case GRAPHICS_NOR:
            if (pixel && ((bDMDScreenRAM[ptr] & lookup) == 0))
                bDMDScreenRAM[ptr] |= lookup;
            break;
    }
}

void FDMD::drawLine(int x1, int y1, int x2, int y2, DMDGraphicsMode mode) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        writePixel(x1, y1, mode, true);
        if (x1 == x2 && y1 == y2) break;
        int e2 = err << 1;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void FDMD::lightRow(byte row) {
    digitalWrite(_aPin, (row & 0x01) ? HIGH : LOW);
    digitalWrite(_bPin, (row & 0x02) ? HIGH : LOW);
}

void FDMD::scanDisplayBySPI() {
    int rowsize = DisplaysTotal << 2;
    int offset = rowsize * bDMDByte;

    vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    for (int i = 0; i < rowsize; i++) {
        vspi->transfer(bDMDScreenRAM[offset + i + row3]);
        vspi->transfer(bDMDScreenRAM[offset + i + row2]);
        vspi->transfer(bDMDScreenRAM[offset + i + row1]);
        vspi->transfer(bDMDScreenRAM[offset + i]);
    }
    vspi->endTransaction();

    oeRowsOff();
    lightRow(bDMDByte);
    latchShiftRegToOutput();
    oeRowsOn();

    bDMDByte = (bDMDByte + 1) & 0x03;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int FDMD::drawChar12x16(int bX, int bY, uint8_t charIndex, DMDGraphicsMode mode) {
    if (charIndex >= 183) return 0;

    // خواندن عرض کاراکتر از 4 بیت اول سطر صفر
    uint16_t firstRow = pgm_read_word(&(font12x16[charIndex][0]));
    uint8_t charWidth = (firstRow >> 12) & 0x0F;
    if (charWidth == 0) charWidth = 6;

    for (uint8_t row = 0; row < 16; row++) {
        // خواندن 12 بیت دیتای سطر
        uint16_t rowData = pgm_read_word(&(font12x16[charIndex][row])) & 0x0FFF;

        for (uint8_t col = 0; col < charWidth; col++) {
            // اصلاح میرور افقی: خواندن بیت‌ها از پرارزش‌ترین بیت کاراکتر
            bool pixelOn = (rowData & (1 << (charWidth - 1 - col))) != 0;
            writePixel(bX + col, bY + row, mode, pixelOn);
        }
    }

    return charWidth;
}


int FDMD::getFarsiRow(uint16_t Uch) {
    uint8_t winCode = 0;

    if (Uch >= 1575 && Uch <= 1590)        winCode = (uint8_t)(Uch - 1376); // ا تا ض
    else if (Uch >= 1591 && Uch <= 1594)   winCode = (uint8_t)(Uch - 1375); // ط تا غ
    else if (Uch >= 1601 && Uch <= 1602)   winCode = (uint8_t)(Uch - 1380); // ف و ق
    else if (Uch == 1705 || Uch == 1603)   winCode = 223;                   // ک
    else if (Uch == 1604)                  winCode = 225;                   // ل
    else if (Uch >= 1605 && Uch <= 1608)   winCode = (uint8_t)(Uch - 1378); // م تا و
    else if (Uch == 1740 || Uch == 1610)   winCode = 237;                   // ی
    else if (Uch == 1570)                  winCode = 193;                   // آ
    else if (Uch == 1574)                  winCode = 197;                   // ئ
    else if (Uch == 1662)                  winCode = 129;                   // پ
    else if (Uch == 1670)                  winCode = 141;                   // چ
    else if (Uch == 1688)                  winCode = 142;                   // ژ
    else if (Uch == 1711)                  winCode = 144;                   // گ
    else return -1;

    switch (winCode) {
        case 129: return 1;  // پ
        case 141: return 2;  // چ
        case 142: return 3;  // ژ
        case 144: return 4;  // گ

        default:
            if (winCode >= 193 && winCode <= 237) {
                return (winCode - 193 + 5);
            }
            return -1;
    }
}


int FDMD::parseFarsiString(const char* str, FarsiChar* out, int maxLen) {
    int i = 0, count = 0;
    int len = (int)strlen(str);

    while (i < len && count < maxLen) {
        uint8_t b1 = (uint8_t)str[i];

        // ASCII
        if (b1 < 0x80) {
            if (b1 == 0x20) {
                out[count++] = { 0x0020, -2 };   // space
            } else {
                out[count++] = { b1, -3 };       // ascii
            }
            i += 1;
        }
        // 2-byte UTF-8
        else if ((b1 & 0xE0) == 0xC0 && i + 1 < len) {
            uint8_t b2 = (uint8_t)str[i + 1];
            uint16_t uch = ((uint16_t)(b1 & 0x1F) << 6) | (uint16_t)(b2 & 0x3F);
            out[count++] = { uch, getFarsiRow(uch) };
            i += 2;
        }
        // 3-byte UTF-8
        else if ((b1 & 0xF0) == 0xE0 && i + 2 < len) {
            uint8_t b2 = (uint8_t)str[i + 1];
            uint8_t b3 = (uint8_t)str[i + 2];
            uint16_t uch = ((uint16_t)(b1 & 0x0F) << 12) |
                           ((uint16_t)(b2 & 0x3F) << 6) |
                           (uint16_t)(b3 & 0x3F);

            if (uch == 0x200C) {
                out[count++] = { 0x200C, -4 };   // ZWNJ
            } else {
                out[count++] = { uch, getFarsiRow(uch) };
            }
            i += 3;
        }
        // 4-byte UTF-8 -> skip
        else if ((b1 & 0xF8) == 0xF0 && i + 3 < len) {
            i += 4;
        }
        else {
            i++;
        }
    }

    return count;
}


void FDMD::drawFarsi(int x, int y, const char* str) {
    const int MAX_CHARS = 128;
    FarsiChar chars[MAX_CHARS];
    uint8_t glyphIdx[MAX_CHARS];
    int widths[MAX_CHARS];

    int count = parseFarsiString(str, chars, MAX_CHARS);
    if (count <= 0) return;

    bool hasFarsi = false;

    // گام اول: شناسایی حروف و محاسبه عرض‌ها
    for (int i = 0; i < count; i++) {
        int row = chars[i].row;

        if (row == -2) { // فاصله
            glyphIdx[i] = 0;
            widths[i] = 4;
            continue;
        }
        if (row == -3) { // کاراکتر اسکی / انگلیسی / عدد
            uint8_t ac = (uint8_t)chars[i].uch;
            glyphIdx[i] = ac;
            uint16_t fw = pgm_read_word(&(font12x16[ac][0]));
            widths[i] = (fw >> 12) & 0x0F;
            if (widths[i] == 0) widths[i] = 6;
            continue;
        }
        if (row == -4 || row < 0 || row >= 50) {
            glyphIdx[i] = 0;
            widths[i] = 0;
            continue;
        }

        hasFarsi = true; // اگر حداقل یک کاراکتر فارسی باشد

        bool connectRight = false; 
        bool connectLeft  = false; 

        if (i < count - 1) {
            int nr = chars[i + 1].row;
            if (nr >= 0 && nr < 50 && FAmap[nr][4] == 1) connectLeft = true; 
        }

        if (i > 0) {
            int pr = chars[i - 1].row;
            if (pr >= 0 && pr < 50 && FAmap[pr][5] == 1) connectRight = true;
        }

        if (FAmap[row][4] == 0) connectRight = false; 
        if (FAmap[row][5] == 0) connectLeft  = false; 

        int col = 0;
        if (connectRight && connectLeft) col = 3;
        else if (connectRight)          col = 2;
        else if (connectLeft)           col = 1;
        else                            col = 0;

        glyphIdx[i] = FAmap[row][col];

        uint16_t fw = pgm_read_word(&(font12x16[glyphIdx[i]][0]));
        widths[i] = (fw >> 12) & 0x0F;
        if (widths[i] == 0) widths[i] = 6;
    }

    // حالت الف: اگر متن کاملاً انگلیسی/لاتین است (بدون فارسی) -> رندر مستقیم چپ به راست (LTR)
    if (!hasFarsi) {
        int curX = x;
        for (int i = 0; i < count; i++) {
            if (widths[i] <= 0) continue;
            if (chars[i].row == -3) {
                drawChar12x16(curX, y, glyphIdx[i], GRAPHICS_NORMAL);
            }
            curX += widths[i];
        }
        return;
    }

    // حالت ب: متن فارسی یا ترکیبی (RTL Base)
    int totalWidth = 0;
    for (int i = 0; i < count; i++) {
        totalWidth += widths[i];
    }

    int curX = x + totalWidth;
    int i = 0;
    while (i < count) {
        if (widths[i] <= 0) {
            i++;
            continue;
        }

        // اگر به کاراکتر اسکی رسیدیم
        if (chars[i].row == -3) {
            int startAscii = i;
            int blockWidth = 0;

            // بلاک انگلیسی را جلو می‌بریم تا جایی که به حرف فارسی برسیم
            // (فاصله‌های بین کلمات انگلیسی هم داخل همین بلاک حفظ می‌شوند)
            while (i < count) {
                if (chars[i].row == -3) {
                    blockWidth += widths[i];
                    i++;
                } else if (chars[i].row == -2) {
                    // چک کنیم آیا بعد از فاصله باز هم کاراکتر انگلیسی هست؟
                    bool nextIsAscii = false;
                    for (int n = i + 1; n < count; n++) {
                        if (chars[n].row == -2) continue;
                        if (chars[n].row == -3) nextIsAscii = true;
                        break;
                    }
                    if (nextIsAscii) {
                        blockWidth += widths[i]; // فاصله متعلق به متن انگلیسی است
                        i++;
                    } else {
                        break; // فاصله متعلق به متن فارسی بعدی است
                    }
                } else {
                    break; // رسیدیم به حرف فارسی
                }
            }

            curX -= blockWidth;
            int drawX = curX;

            // چاپ LTR کاراکترهای این بلاک انگلیسی
            for (int k = startAscii; k < i; k++) {
                if (chars[k].row == -3) {
                    drawChar12x16(drawX, y, glyphIdx[k], GRAPHICS_NORMAL);
                }
                drawX += widths[k];
            }
        } 
        else {
            // کاراکتر فارسی یا فاصله فارسی
            curX -= widths[i];
            if (chars[i].row != -2 && chars[i].row != -4) {
                drawChar12x16(curX, y, glyphIdx[i], GRAPHICS_NORMAL);
            }
            i++;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int FDMD::getFarsiStringWidth(const char* str) {
    const int MAX_CHARS = 128;
    FarsiChar chars[MAX_CHARS];
    int count = parseFarsiString(str, chars, MAX_CHARS);
    if (count <= 0) return 0;

    int totalWidth = 0;
    for (int i = 0; i < count; i++) {
        int row = chars[i].row;
        if (row == -2) { totalWidth += 4; continue; }
        if (row == -3) {
            uint8_t ac = (uint8_t)chars[i].uch;
            uint16_t fw = pgm_read_word(&(font12x16[ac][0]));
            int w = (fw >> 12) & 0x0F;
            totalWidth += (w == 0) ? 6 : w;
            continue;
        }
        if (row == -4 || row < 0 || row >= 50) continue;

        uint8_t glyph = FAmap[row][0];
        uint16_t fw = pgm_read_word(&(font12x16[glyph][0]));
        int w = (fw >> 12) & 0x0F;
        totalWidth += (w == 0) ? 6 : w;
    }
    return totalWidth;
}


void FDMD::scrollFarsi(int y, const char* str, ScrollDirection dir, int speedMs) {
    int textWidth = getFarsiStringWidth(str);
    if (textWidth <= 0) return;

    // عرض کل نمایشگر شما (مثلاً اگر یک پنل P10 دارید 32، دو پنل 64 و...)
    // اگر متغیری مثل DisplaysWide*32 دارید می‌توانید از آن استفاده کنید
    int displayWidth = DisplaysWide * 32; 

    if (dir == SCROLL_LEFT) {
        // حرکت از بیرون کادر سمت راست به سمت چپ تا وقتی کلاً خارج شود
        for (int x = displayWidth; x >= -textWidth; x--) {
            clearScreen(true);
            drawFarsi(x, y, str);
            delay(speedMs);
        }
    } 
    else if (dir == SCROLL_RIGHT) {
        // حرکت از بیرون کادر سمت چپ به سمت راست تا وقتی کلاً خارج شود
        for (int x = -textWidth; x <= displayWidth; x++) {
            clearScreen(true);
            drawFarsi(x, y, str);
            delay(speedMs);
        }
    }
}
