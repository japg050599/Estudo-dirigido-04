import serial
import struct

ser = serial.Serial("COM6",115200,timeout=5)

for i in range(10):
    rx = ser.read(2)

    if len(rx) < 2:
        print("Timeout")
        break

    valor = struct.unpack("<H", rx)[0]
    print(i, valor)

ser.close()