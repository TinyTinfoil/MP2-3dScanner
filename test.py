import serial 
import time 
arduino = serial.Serial(port='COM10', baudrate=9600, timeout=.001) 
def write_read(x): 
	#    arduino.write(x) 
	   time.sleep(0.05) 
	   data = arduino.readline() 
	   return data

#write read data to file
file = open("data.txt", "w")
for i in range(0, 120):
    byte = write_read(0)[:3]
    if byte == b'':
        byte = "-1"
    file.write(str(int(byte)) + ",")
file.close()
arduino.close()