#include <ST7920_SPI.h>
#include <SPI.h>

#define LCD_PIN_CS 10
ST7920_SPI lcd(LCD_PIN_CS);


#define FPS 25

uint8_t read_next_byte_raw() {
  while(Serial.available() == 0) {
    delay(0);
  }
  return Serial.read();
}


#define STREAM_BUFF_SIZE 252
uint8_t stream_buffer[STREAM_BUFF_SIZE];
int stream_buffer_pos = STREAM_BUFF_SIZE;
int stream_current_buffer_size = 0;

void request_new_buff() {
  stream_buffer_pos = 0;
  Serial.write(STREAM_BUFF_SIZE);
  stream_current_buffer_size = read_next_byte_raw();

  const int minor_block_size = 12;
  int i = 0;
  int e = stream_current_buffer_size - (minor_block_size - 1);
  while (i < e) {
    while (Serial.available() < minor_block_size) {
      // delay(0);
    }
    for (int j = 0; j < minor_block_size; j++) {
      stream_buffer[i++] = Serial.read();
    }
  } 
  while (i < stream_current_buffer_size) {
    stream_buffer[i++] = read_next_byte_raw();
  }
}

uint8_t read_next_byte() {
  if (stream_buffer_pos >= STREAM_BUFF_SIZE) {
    request_new_buff();
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
  Serial.begin(150000);

  SPI.begin();
  lcd.init();
  lcd.cls();
  lcd.display(0);
  lcd.display(1);

  while(read_next_byte_raw() != 0xabu) {}
}


uint8_t double_buffer = 0;

// more optimized way to send data to LCD

void lcdSendCmd(byte b) {
  SPI.transfer(0xF8);
  SPI.transfer(b & 0xF0);
  SPI.transfer(b << 4);
}

void lcdDisplay(int buff) {
  byte i, j, b;
  int index_i, index_j;
  SPI.beginTransaction(SPISettings(1750000l, MSBFIRST, SPI_MODE3));

  index_j = -buff * 32;
  for(j=0;j<64/2;j++) {
    digitalWrite(LCD_PIN_CS, HIGH);
    lcdSendCmd(LCD_ADDR | index_j++);
    lcdSendCmd(LCD_ADDR);
    digitalWrite(LCD_PIN_CS, LOW);
    digitalWrite(LCD_PIN_CS, HIGH);
    SPI.transfer(0xFA);  // data

    index_i = j*16;    
    for(i=0;i<16;i++) {  // 16 bytes from line #0+
      b=lcd.scr[index_i++]; 
      SPI.transfer(b & 0xF0);  SPI.transfer(b << 4);
    }

    index_i = (j+32)*16;
    for(i=0;i<16;i++) {  // 16 bytes from line #32+
      b=lcd.scr[index_i++]; 
      SPI.transfer(b & 0xF0);  SPI.transfer(b << 4);
    }
    digitalWrite(LCD_PIN_CS, LOW);
  }
  SPI.endTransaction();
}


void loop() 
{
  static long frame_counter = 0;
  
  static bool skip_frame = false;
  static int frame_skip_cd = 0;
  
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
    // difference frame
    for (int i = 0; i < frame_size; i++) {
      int offset = read_next_byte();
      int x = offset / 8;
      int y = offset % 8;
      for (int yi = 0; yi < 8; yi++) {
        lcd.scr[(y * 8 + yi) * 16 + x] = read_next_byte();
      }
    }
  }
  if (!skip_frame) {
    lcdDisplay(double_buffer);
  }
  
  static long time_start = 0;
  if (time_start == 0) {
    time_start = millis(); 
  }
  long current_time = millis() - time_start;
  long desired_time = (1000l * frame_counter) / FPS;
  long time_delta = desired_time - current_time;

  bool skip_next_frame = false;
  if (time_delta > 0) {
    delay(time_delta); 
  } else if (time_delta < -500 && frame_skip_cd <= 0) {
    skip_next_frame = true;
    frame_skip_cd = 5;
  }

  if (!skip_frame) {
    lcd.switchBuf(double_buffer);
    double_buffer = !double_buffer;
  }

  skip_frame = skip_next_frame;
  frame_counter++;
  frame_skip_cd--;
}
