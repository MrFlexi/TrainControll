
// https://www.online-utility.org/image/convert/to/XBM

#define WiFi_Logo_width 60
#define WiFi_Logo_height 36
const char WiFi_Logo_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
  0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
  0xFF, 0x03, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
  0x00, 0xFF, 0xFF, 0xFF, 0x07, 0xC0, 0x83, 0x01, 0x80, 0xFF, 0xFF, 0xFF,
  0x01, 0x00, 0x07, 0x00, 0xC0, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x0C, 0x00,
  0xC0, 0xFF, 0xFF, 0x7C, 0x00, 0x60, 0x0C, 0x00, 0xC0, 0x31, 0x46, 0x7C,
  0xFC, 0x77, 0x08, 0x00, 0xE0, 0x23, 0xC6, 0x3C, 0xFC, 0x67, 0x18, 0x00,
  0xE0, 0x23, 0xE4, 0x3F, 0x1C, 0x00, 0x18, 0x00, 0xE0, 0x23, 0x60, 0x3C,
  0x1C, 0x70, 0x18, 0x00, 0xE0, 0x03, 0x60, 0x3C, 0x1C, 0x70, 0x18, 0x00,
  0xE0, 0x07, 0x60, 0x3C, 0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C,
  0xFC, 0x73, 0x18, 0x00, 0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00,
  0xE0, 0x87, 0x70, 0x3C, 0x1C, 0x70, 0x18, 0x00, 0xE0, 0x8F, 0x71, 0x3C,
  0x1C, 0x70, 0x18, 0x00, 0xC0, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x08, 0x00,
  0xC0, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x0C, 0x00, 0x80, 0xFF, 0xFF, 0x1F,
  0x00, 0x00, 0x06, 0x00, 0x80, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x07, 0x00,
  0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0xF8, 0xFF, 0xFF,
  0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xFF, 0x01, 0x00, 0x00,
  0x00, 0x00, 0xFC, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0xFF,
  0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

const char activeSymbole[] PROGMEM = {
    B00000000,
    B00000000,
    B00011000,
    B00100100,
    B01000010,
    B01000010,
    B00100100,
    B00011000
};

const char inactiveSymbole[] PROGMEM = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011000,
    B00011000,
    B00000000,
    B00000000
};


#define ice_width 100
#define ice_height 48
const char ice_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xFF, 0x01, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x3C, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x80, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x7F, 0x00, 0x00, 
  0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0xFF, 0x01, 
  0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 
  0x03, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 
  0xFF, 0x01, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 
  0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xF0, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0xFC, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, 
  0x00, 0x00, 0xFE, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 
  0x00, 0x00, 0x00, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 
  0x01, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0x03, 0x00, 0xD8, 0x00, 0x00, 0x00, 
  0x00, 0x0E, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0x82, 
  0x00, 0x00, 0x70, 0x00, 0x00, 0xE0, 0xFF, 0xFF, 0x00, 0xE0, 0xFF, 0xFF, 
  0xBF, 0x06, 0x00, 0x80, 0x07, 0x00, 0xF0, 0xFF, 0x7F, 0x00, 0xF8, 0xFF, 
  0xFF, 0xDF, 0xFE, 0x1F, 0x00, 0x08, 0x00, 0xF8, 0xFF, 0x3F, 0x00, 0xFC, 
  0xFF, 0xFF, 0xDF, 0xFE, 0xFF, 0x1F, 0x08, 0x00, 0xF8, 0xFF, 0x0F, 0x00, 
  0xFF, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0xFC, 0xFF, 0x07, 
  0x80, 0xFF, 0xFF, 0xFF, 0xDF, 0xFE, 0xFF, 0xFF, 0x0F, 0x00, 0xFE, 0xFF, 
  0x00, 0xE0, 0xFF, 0xFF, 0xFF, 0xDF, 0xFE, 0xFF, 0x07, 0x08, 0x00, 0xFF, 
  0x1F, 0x00, 0xF0, 0xFF, 0xFF, 0xFF, 0xBF, 0x1E, 0x00, 0x50, 0x08, 0x00, 
  0x01, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xDF, 0x01, 0x00, 0x68, 0x03, 0x0E, 
  0x80, 0x00, 0x00, 0x00, 0xFE, 0x7F, 0x05, 0x00, 0x00, 0xE0, 0x09, 0xE0, 
  0x01, 0x80, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x00, 0x00, 0xEC, 0x1F, 0x00, 
  0x3C, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD, 0x3A, 0x00, 
  0x00, 0x0C, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8, 0x46, 0x03, 
  0x00, 0x80, 0x01, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x96, 0xD5, 0x16, 
  0x00, 0x00, 0x70, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xD4, 0xEA, 0x1F, 
  0x00, 0x00, 0x10, 0x0E, 0x00, 0x00, 0x10, 0x00, 0x00, 0xB8, 0x9B, 0x79, 
  0x00, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x64, 0x6D, 
  0x01, 0x00, 0x00, 0x40, 0x38, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x5A, 
  0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x04, 0x02, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x80, 0xC6, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x02, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xA5, 0x07, 0x00, 0x00, 0x00, 0x00, 
  0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x98, 0xF9, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x80, 0x6F, 0x0E, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x84, 0xD6, 0x03, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x40, 0x79, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x10, 0x00, 0xA0, 0x0E, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x00, 0xD0, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x38, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
  0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x10, 0x00, 
  0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
  0xC0, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x9F, 
  0xEE, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  };


#define dampflok_width 100
#define dampflok_height 79
const char dampflok_bits[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6B, 0xA6, 0xA9, 0x6A, 0xA6, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0xF7, 0xBE, 0xB7, 0x37, 0x00, 0x00, 0xF8, 0xFF, 
  0x3F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xF8, 
  0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 
  0xF8, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0xBF, 0x59, 0x55, 0x7F, 0x00, 
  0x00, 0xF8, 0x7E, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x7F, 
  0x00, 0x00, 0x00, 0x99, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 
  0x7F, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 
  0x00, 0x7E, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3F, 
  0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 
  0x3F, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 
  0x00, 0x3F, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 
  0x00, 0x00, 0x3F, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 
  0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0xFF, 0x01, 
  0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0xFF, 
  0x01, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 
  0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x7F, 0x00, 0x00, 
  0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x7E, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0xFF, 
  0x7D, 0xDF, 0xB7, 0xBF, 0xDF, 0x05, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x00, 0x00, 0x00, 0x3F, 0x00, 
  0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 0x3F, 
  0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0x00, 0x00, 0x00, 
  0x3F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 
  0x00, 0x3F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 
  0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x80, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x80, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 
  0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 
  0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x3F, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0x1F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0x0F, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0xFF, 0xFF, 0x87, 0xFF, 
  0xFF, 0x87, 0xFF, 0xFF, 0x87, 0xFF, 0x03, 0x00, 0x80, 0xFF, 0xFF, 0x00, 
  0xFC, 0xFF, 0x00, 0xFC, 0xFF, 0x00, 0xFC, 0x07, 0x00, 0x00, 0xFF, 0x3F, 
  0x78, 0xF0, 0x3F, 0xD8, 0xF0, 0x3F, 0xD8, 0xF0, 0x03, 0x00, 0x80, 0xFF, 
  0x1F, 0xFF, 0xE3, 0x1F, 0xFF, 0xE3, 0x1F, 0xFF, 0xE3, 0x03, 0x00, 0x00, 
  0xFF, 0x8F, 0xFF, 0xC7, 0x8F, 0xFF, 0xC7, 0x8F, 0xFF, 0xC7, 0x07, 0x00, 
  0x80, 0xFF, 0xC7, 0xFF, 0x8F, 0xC7, 0xFF, 0x8F, 0xC7, 0xFF, 0xCF, 0x07, 
  0x00, 0x00, 0xFF, 0xE7, 0xFF, 0x9F, 0xE7, 0xFF, 0x9F, 0xE7, 0xFF, 0x9F, 
  0x03, 0x00, 0x00, 0xFF, 0xF3, 0xFF, 0x3F, 0xF3, 0xFF, 0x3F, 0xF3, 0xFF, 
  0x3F, 0x03, 0x00, 0x80, 0xFF, 0xF3, 0x87, 0x7F, 0xF3, 0x87, 0x3F, 0xF3, 
  0x87, 0x3F, 0x07, 0x00, 0x00, 0xFF, 0xF9, 0x03, 0x7F, 0xF3, 0x03, 0x7E, 
  0xFA, 0x03, 0x7F, 0x03, 0x00, 0x80, 0xFF, 0xFB, 0x01, 0x7E, 0xF8, 0x01, 
  0x7E, 0xF8, 0x01, 0x7E, 0x06, 0x00, 0x00, 0xFF, 0xF9, 0x01, 0x7C, 0xF8, 
  0x01, 0x7E, 0xF8, 0x01, 0x7E, 0x06, 0x00, 0x80, 0xFF, 0xF9, 0x01, 0x7E, 
  0xF8, 0x01, 0x7C, 0xFA, 0x00, 0x7C, 0x02, 0x00, 0x00, 0x00, 0xF8, 0x01, 
  0x7C, 0xF8, 0x01, 0x7E, 0xF8, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0xF8, 
  0x01, 0x7E, 0xF8, 0x01, 0x7E, 0xF8, 0x01, 0x7E, 0x00, 0x00, 0x00, 0x00, 
  0xF8, 0x03, 0x7F, 0xF8, 0x03, 0x7E, 0xF8, 0x03, 0x7F, 0x00, 0x00, 0x00, 
  0x00, 0xF0, 0x07, 0x7F, 0xF0, 0x87, 0x3F, 0xF0, 0x87, 0x3F, 0x00, 0x00, 
  0x00, 0x00, 0xF0, 0xFF, 0x3F, 0xF0, 0xFF, 0x3F, 0xF0, 0xFF, 0x3F, 0x00, 
  0x00, 0x00, 0x00, 0xE0, 0xFF, 0x1F, 0xE0, 0xFF, 0x1F, 0xE0, 0xFF, 0x1F, 
  0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0x1F, 0xC0, 0xFF, 0x0F, 0xC0, 0xFF, 
  0x0F, 0x00, 0x00, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x80, 0xFF, 0x0F, 0x80, 
  0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x03, 0x00, 0xFF, 0x03, 
  0x00, 0xFF, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78, 
  0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };



