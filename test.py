import serial 
import time 
arduino = serial.Serial(port='COM10', baudrate=9600, timeout=.1) 
def write_read(x): 
	#    arduino.write(x) 
	   time.sleep(0.05) 
	   data = arduino.readline() 
	   return data

#write read data to file
file = open("data.txt", "a")
for i in range(0, 120):
    file.write(str(write_read(0)))
file.close()
arduino.close()