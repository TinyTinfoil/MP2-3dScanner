import serial 
import time 
action = {"none":  0b0000, "send": 0b0010, 'request_ack':  0b0001, 'ack_response': 0b0101, 'resend': 0b0011, 'error': 0b0100}
arduino = serial.Serial(port='COM10', baudrate=9600, timeout=.001)
def write_read(x): 
		arduino.write((action['request_ack'] << 4) | 0b1100)
		arduino.write(0b00000000)
		arduino.write(0b00000000)
		arduino.write(0b00100000)
		time.sleep(0.05) 
		my_bytes = arduino.read(4)
		for my_byte in my_bytes:
			print(f'{my_byte:0>8b}', end=' ')
		data = 0 
		return data
print(write_read(0))
    # inp_panX = input('Pan:')
#     inp_tiltY = input('Tilt:')
    # print(int(inp_panX).to_bytes())
# arduino.close()
