#include <Servo.h>

Servo pan, tilt;
uint8_t panX, tiltY, newpanX, newtiltY;
uint8_t pan_pin, tilt_pin;
uint8_t analog = A0;
uint16_t LOOP_INTERVAL = 20;

// Send - transmit data, Request Ack - request acknowledgement while also sending, Error CMD - error, extra bits define appropriate response from messager, cmd - command
enum control_bits {send = 0b00, request_ack = 0b01, error_cmd = 0b10, cmd = 0b11};
enum command {ack = 1, request_data = 2};


struct bytes4 {
  byte byte1;
  byte byte2;
  byte byte3;
  byte byte4;
};

struct packet
{
  uint16_t data;
  byte c_bits;
};

long int loop_time;
enum mode {none, ack_response, gather_data};
enum mode action = 0;

bool move = false;

/*
** Returns a boolean value that indicates whether the current time, t, is later than some prior 
** time, t0, plus a given interval, dt.  The condition accounts for timer overflow / wraparound.
*/
bool it_is_time(uint32_t t, uint32_t t0, uint16_t dt) {
  return ((t >= t0) && (t - t0 >= dt)) ||         // The first disjunct handles the normal case
            ((t < t0) && (t + (~t0) + 1 >= dt));  //   while the second handles the overflow case
}

void setup() {
  // put your setup code here, to run once:
  // start the serial port
  //
  long baudRate = 9600;       // NOTE1: The baudRate for sending & receiving programs must match
  Serial.begin(baudRate);     // NOTE2: Set the baudRate to 115200 for faster communication
  Serial.setTimeout(1);
  pan.attach(pan_pin);
  tilt.attach(tilt_pin);
  loop_time = millis();
}

uint16_t last_sensor_data = 0;

void loop() {
  byte sentData[4] = {0,0,0,0};
  if (Serial.available() >= 4)
  {
    Serial.readBytes(sentData, 4);
    receive_data(sentData);
    while (Serial.available() > 0){
      Serial.read();
    }
  }

  if (move){
    for (uint8_t pos = panX; (panX>newpanX? pos >= newpanX : pos <= newpanX ); (panX>newpanX? pos -=1 :pos +=1 )) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      pan.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (uint8_t pos = tiltY; (tiltY> newtiltY? pos >= newtiltY : pos <= newtiltY ); (tiltY>newtiltY? pos -=1 :pos +=1 )) { // goes from 180 degrees to 0 degrees
    tilt.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
    }
  }
  last_sensor_data = read_sensor();
  action = ack_response;
  send_data();
}


void receive_data(byte data[10]){
  struct packet packet = depacketify(data);
  if (packet.c_bits == cmd){
    if (packet.data == request_data){
      action = gather_data;
      move = false;
      return;
    }
    return;
  }
  if (packet.c_bits == request_ack){ // the python script will always request acknowledgment for servo movement, or keep sending data
    action = ack_response;
    send_data();
    move = true; // Thus, the servo shall move
    newpanX = packet.data >> 8;
    newtiltY = packet.data & 0b0000000011111111;
    return;
  }
  action = none;
  return;
}

void send_data(){
  Serial.flush();
  if (action == ack_response){
    Serial.write(packetify(ack,cmd));
  }
  if (action == gather_data){
    Serial.write(packetify(last_sensor_data,send)); // no need for request_ack, since the python script will keep requesting data until it has enough
  }
}

long int t;
uint8_t x,y,z,res;

uint16_t read_sensor(){
  t = millis();
    if (it_is_time(t, loop_time, LOOP_INTERVAL)) {
    x = analogRead(analog);
    y = analogRead(analog);
    z = analogRead(analog);
    res = min(min(x, y), z);
    loop_time = t;
    return res;
  }
}



enum control_bits c_bits = error_cmd;
// commands sent with command bit


uint32_t packetify(uint16_t data, control_bits c_bits){
  byte data1 = (data >> 8);
  byte data2 = (data & 0b0000000011111111);
  byte dpacket1 = (data1 >> 2) << 1 | 0b00000001;
  byte dpacket2 = (data2 >> 2) << 1 | 0b10000000;
  byte endpacket = (data1 & 0b0000000000000011) << 5  | (data2 & 0b0000000000000011) << 3 | c_bits << 1;
  uint32_t packet = (0b11111111 << 24) | (dpacket1 << 16) | (dpacket2 << 8) | endpacket;
  return packet;
}


packet depacketify(byte data[4]){
  byte endbit_data_1 = data[4] >> 5;
  byte endbit_data_2 = (data[4] & 0b00011000) >> 3;
  byte data1 = (data[1] >> 1) << 2 | endbit_data_1;
  byte data2 = ((data[2] & 0b01111110) >> 1) << 2 | endbit_data_2;
  byte c_bits = (data[4] & 0b00000110) >> 1;
  uint16_t data_a = (data1 << 8) | data2;
  return {data_a, c_bits};
}

bool is_packet_valid(byte data[4]){
  return (data[0] == 0b11111111 && data[1] & 0b10000001 == 0b00000001 && data[2] & 0b10000001 == 0b10000000 && data[3] & 0b10000001 == 0b00000000);
}