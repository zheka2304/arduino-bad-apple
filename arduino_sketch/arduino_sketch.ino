// (C)2019 Pawel A. Hernik

/*
 128x64 ST7920 connections in SPI mode (only 6 wires between LCD and MCU):
 #01 GND  -> GND
 #02 VCC  -> VCC (5V)
 #04 RS   -> D10/CS or any pin
 #05 R/W  -> D11/MOSI
 #06 E    -> D13/SCK
 #15 PSB  -> GND (for SPI mode)
 #19 BLA  -> D9, VCC or any pin via 300ohm resistor
 #20 BLK  -> GND
*/

#define LCD_BACKLIGHT  9
#define LCD_CS         10

#include <ST7920_SPI.h>
#include <SPI.h>
ST7920_SPI lcd(LCD_CS);


#define FPS 12

uint8_t read_next_byte_raw() {
  while(Serial.available() == 0) {
    delay(1);
  }
  return Serial.read();
}


#define STREAM_BUFF_SIZE 255
uint8_t stream_buffer[STREAM_BUFF_SIZE];
int stream_buffer_pos = STREAM_BUFF_SIZE;
int stream_current_buffer_size = 0;

uint8_t read_next_byte() {
  if (stream_buffer_pos >= STREAM_BUFF_SIZE) {
    stream_buffer_pos = 0;
    Serial.write(STREAM_BUFF_SIZE);
    stream_current_buffer_size = read_next_byte_raw();
    for (int i = 0; i < stream_current_buffer_size; i++) {
      stream_buffer[i] = read_next_byte_raw();
    }
  }
  return stream_buffer[stream_buffer_pos++];
}

void read_next_bytes(uint8_t* bytes, int count) {
  for (int i = 0; i < count; i++) {
    bytes[i] = read_next_byte();
  }
}

void setup() 
{
  Serial.begin(57600 * 2);

  SPI.begin();
  lcd.init();
  lcd.cls();
  for (int i = 0; i < 100; i++) {
    lcd.scr[i] = 169;
  }
  lcd.display(0);
  lcd.cls();
  for (int i = 500; i < 600; i++) {
    lcd.scr[i] = 169;
  }
  lcd.display(1);

  while(read_next_byte_raw() != 0xabu) {}
}


uint8_t double_buffer = 0;

void loop() 
{
  static long last_frame_end = 0;
  if (last_frame_end != 0) {
    long t = millis();
    long d = 1000 / FPS - (t - last_frame_end);
    last_frame_end = t;
    if (d > 0) delay(d);
  } else {
    last_frame_end = millis();
  }
  
  // lcd.cls();
  uint8_t frame_size = read_next_byte();
  if (frame_size == 255) {
    // full frame
    for (int x = 0; x < 16; x++) {
      for (int y = 0; y < 8; y++) {
        for (int yi = 0; yi < 8; yi++) {
          lcd.scr[(y * 8 + yi) * 16 + x] = read_next_byte();
        }
      }
    }
  } else {
    for (int i = 0; i < frame_size; i++) {
      int offset = read_next_byte();
      int x = offset / 8;
      int y = offset % 8;
      for (int yi = 0; yi < 8; yi++) {
        lcd.scr[(y * 8 + yi) * 16 + x] = read_next_byte();
      }
    }
  }

  
  lcd.display(double_buffer);
  lcd.switchBuf(double_buffer);
  double_buffer = !double_buffer;
  
}
