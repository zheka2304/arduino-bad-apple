import time
import serial


SERIAL_PORT = "com3"

arduino_serial = serial.Serial(SERIAL_PORT, 57600 * 2)
time.sleep(2)
print("initialized serial")


def send_data_span(data):
    index = 0
    while index < len(data):
        # await max block size, received from arduino
        block_size = int(arduino_serial.read()[0])
        if block_size == 0:
            print("arduino requested stream end")
            return

        # send byte count
        cur_block_size = min(block_size, len(data) - index)
        arduino_serial.write(bytes([cur_block_size]))

        # send all bytes
        print(f"sending data {index}/{len(data)} block size {cur_block_size}")
        arduino_serial.write(data[index: index + cur_block_size])
        index += cur_block_size


def start_sending_data():
    # signal to begin sending data
    arduino_serial.write(b'\xab')
    print("initialized data stream, awaiting first block request")


def end_sending_data():
    # at the end send 0 as next block size
    arduino_serial.read()
    arduino_serial.write(bytes([0]))


def send_data(data):
    start_sending_data()
    send_data_span(data)
    end_sending_data()


if __name__ == '__main__':
    print("sending data")
    with open("bad_apple.mp4.bin", "rb") as f:
        send_data(f.read())
