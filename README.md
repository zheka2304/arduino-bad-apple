# Bad Apple for Arduino

Small project, used to play videos on Arduino Nano using 128x64 LED display on ST7920 controller, created mostly for playing Touhou - Bad Apple.

![](https://github.com/zheka2304/arduino-bad-apple/blob/master/img/photo.jpg?raw=true)

# Implementation

Because Arduino has only 2KB RAM, it is required to stream video from pc using serial, which has pretty low bandwidth. With all this in mind, it was decided to split screen into pieces and send only changes, which helps to reach about 20-25 FPS.

Project consists of 3 main parts: video encoder, serial data stream and lightweight video decoder.

- Encoder reads sequence of video frames, resizes and creates binary data, which is split into 8 byte (8x8 pixel) regions. Then it compares frames and saves data only for different regions, if difference frame is less size, than full frame (this is true 99% of the time), it is used instead of full frame. Whole video is encoded in this way in one big binary file, that contains sequence of all encoded frames.  

- Serial stream is a bridge between PC and Arduino. Built-in serial interface is not designed to send alot of data at once, so data is sent in blocks. PC sends one hardcoded byte to start data transfer, than waits for Arduino, to send back next data block maximum size, and after this sends next block. On the Arduino side block is stored in a buffer, and when buffer is empty, it sends request for a new block.

- Decoder is reading data from stream byte by byte, and writes data into screen buffer (in case of full frame it just overrides whole buffer, and in case of difference frame - only regions with changes), then it swaps screen buffers to display it. To interract with ST7920 it uses this library - https://github.com/cbm80amiga/st7920_spi.

This 3 components allow to stream video at relatively high framerate using just Arduino serial.
