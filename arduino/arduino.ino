#include <Servo.h>

Servo pan, tilt;
uint8_t panX, tiltY, newpanX, newtiltY;
uint8_t pan_pin, tilt_pin;
uint8_t analog = A0;
uint16_t LOOP_INTERVAL = 20;

long int loop_time;

enum mode {none = 0b0000, send = 0b0010, request_ack = 0b0001, ack_response = 0b0101, resend = 0b0011, error = 0b0100};
enum mode action = none;

bool completeTransfer = true;

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
uint8_t byte1,byte2,byte3,byte4;

uint16_t last_sensor_data = 0;

void loop() {
  
  while(Serial.available())
  {
    byte1 = Serial.read();
    byte2 = Serial.read();
    byte3 = Serial.read();
    byte4 = Serial.read();
  }
  uint16_t data = receive_data(byte1, byte2, byte3, byte4);
  // if (completeTransfer){
  //   newpanX = data >> 8;
  //   newtiltY = data & 0b0000000011111111;
  //   for (uint8_t pos = panX; (panX>newpanX? pos >= newpanX : pos <= newpanX ); (panX>newpanX? pos -=1 :pos +=1 )) { // goes from 0 degrees to 180 degrees
  //     // in steps of 1 degree
  //     pan.write(pos);              // tell servo to go to position in variable 'pos'
  //     delay(15);                       // waits 15ms for the servo to reach the position
  //   }
  //   for (uint8_t pos = tiltY; (tiltY> newtiltY? pos >= newtiltY : pos <= newtiltY ); (tiltY>newtiltY? pos -=1 :pos +=1 )) { // goes from 180 degrees to 0 degrees
  //   tilt.write(pos);              // tell servo to go to position in variable 'pos'
  //   delay(15);                       // waits 15ms for the servo to reach the position
  //   }
  // }
  last_sensor_data = read_sensor();
  action = resend;
  send_data();
}



uint8_t receive_data(byte header, byte data1, byte data2, byte terminator){
  if (header >> 4 == 0b0010 && terminator & 0b00001111 == 0b1100){
    uint8_t crc = header & 0b00001111;
    uint16_t data = (data1 << 8) | data2;
    uint8_t data_crc = ((data1 >> 4) ^ data1) ^ ((data2 >> 4) ^ data2);
    action = terminator >> 4;
    if (data_crc != crc){
      action = resend;
      completeTransfer = false;
    }
    if (action == request_ack){
      action = ack_response;
      completeTransfer = true;
      return data;
    }
    if (action == send){
      completeTransfer = false;
      action = send;
    }
    if (action == resend){
      completeTransfer = false;
      action = send;
    }
    if (action == ack_response){
      // noop
    }
  } 
}

uint8_t send_data(){
  if (action == ack_response){
    Serial.write(0b00100000);
    Serial.write(0b00000000);
    Serial.write(0b00000000);
    Serial.write((ack_response << 4) | 0b1100);
  }
  if (action == resend){
    Serial.write(0b00100000);
    Serial.write(0b00000000);
    Serial.write(0b00000000);
    Serial.write((resend << 4) | 0b1100);
  }
  if (action == send){
    uint16_t data = last_sensor_data;
    uint8_t data1 = (data << 8);
    uint8_t data2 = 0b0000000011111111 & data;
    uint8_t data_crc = ((data1 >> 4) ^ data1) ^ ((data2 >> 4) ^ data2);
    Serial.write((0b0010 << 4) | data_crc);
    Serial.write(data1);
    Serial.write(data2);
    Serial.write((request_ack << 4) | 0b1100);
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