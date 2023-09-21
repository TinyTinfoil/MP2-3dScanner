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

# Send - transmit data, Request Ack - request acknowledgement while also sending, Error CMD - error, extra bits define appropriate response from messager, cmd - command
control_bits  = {'send': 0b00, 'request_ack': 0b01, 'error_cmd': 0b10, 'cmd': 0b11}
c_bits = 0b00
# commands sent with command bit
command = {'ack':1, 'request_data':2}

def packetify(data, c_bits):
	data1 = (data >> 8)
	data2 = (data & 0b0000000011111111)
	dpacket1 = (data1 >> 2) << 1 | 0b00000001
	dpacket2 = (data2 >> 2) << 1 | 0b10000000
	endpacket = (data1 & 0b0000000000000011) << 5  | (data2 & 0b0000000000000011) << 3 | c_bits << 1
	packet = {0b11111111,dpacket1,dpacket2,endpacket}
	return packet

# struct packet
# {
#   uint16_t data;
#   bytes c_bits;
# };

def depacketify(data):
	endbit_data_1 = data[4] >> 5
	endbit_data_2 = (data[4] & 0b00011000) >> 3
	data1 = (data[1] >> 1) << 2 | endbit_data_1
	data2 = ((data[2] & 0b01111110) >> 1) << 2 | endbit_data_2
	c_bits = (data[4] & 0b00000110) >> 1
	data = (data1 << 8) | data2
	packet = {data, c_bits}

def is_packet_valid(data):
	return (data[0] == 0b11111111 and data[1] & 0b10000001 == 0b00000001 and data[2] & 0b10000001 == 0b10000000 and data[3] & 0b10000001 == 0b00000000)