#include <Servo.h>

Servo pan, tilt;
uint8_t panX, tiltY, newpanX, newtiltY;
uint8_t pan_pin, tilt_pin;
uint8_t analog = A0;
uint16_t LOOP_INTERVAL = 20;

enum mode {
  BUSY = 0b000,
  WRITE1 = 0b001,
  WRITE2 = 0b010,
  WRITE3 = 0b011,
  WRITE4 = 0b100,
  CHECKSUM = 0b101,
  READ = 0b110,
  RESEND = 0b111
};
enum mode action = READ;
uint32_t loop_time;

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


void loop() {
  uint8_t data;
  while(Serial.available())
  {
    data = Serial.read();
    receive_data(data);
  }
  
  if (completeTransfer){
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
  uint8_t data_out = send_data();
  if (data_out){
    Serial.printLn(data_out);
  }
}
uint16_t input_buffer_tilt = 0;
uint16_t input_buffer_pan = 0;
bool completeTransfer = true;
uint8_t action = 0;
uint8_t receive_data(uint8_t data){
//   //Checks ending bits to determine operation
//  // 000 - Busy
//  // 001 - Write 1 (start write transfer) - 6 bits of tilt
//  // 010 - Write 2 
//  // 011 - Write 3- 6 bits of pan
//  // 100 - write 4 - 6 bits of pan
//  // 101 - Checksum (xor of data)
//  // 110 - Read
//  // 111 - Resend (resend read)
  action = data & 0b00000111;
  if (action == WRITE1){
    completeTransfer = false;
    input_buffer_tilt = 0;
    input_buffer_tilt = data >> 3;
  }
  if (action == WRITE2){
    input_buffer_tilt = (data >> 3) << 5 | input_buffer_tilt;
  }
  if (action == WRITE3){
    input_buffer_pan = 0;
    input_buffer_pan = data >> 3;
  }
  if (action == WRITE4){
    input_buffer_pan = (data >> 3) << 5 | input_buffer_pan;
  }
  if (action == CHECKSUM){
    uint8_t checksum = data >> 3;
    uint16_t xor = input_buffer_tilt ^ input_buffer_pan;
    xor = (xor >> 8) ^ xor;
    if ((xor << 3) == checksum){
      completeTransfer = true;
      input_buffer_tilt = newtiltY;
      input_buffer_pan = newpanX;
    }
    else{
      completeTransfer = false;
      send_data(RESEND);
    }
  }
  if (action == RESEND){
    send_data(WRITE1);
  }
}

uint8_t output_buffer1 = 0;
uint8_t output_buffer2 = 0;
uint8_t checksum = 0;
enum mode next_mode = READ; // Read is a placeholder state
uint8_t send_data(uint8_t data){
  if (action == READ || data == WRITE1){
    uint16_t res = read_sensor();
    // Set WRITE1, then WRITE2, then CHECKSUM
    output_buffer1 = (res << 3) | WRITE1;
    output_buffer2 = ((res >> 5) << 3) | WRITE2;
    checksum = (((res >> 8) ^ res) << 3) | CHECKSUM;
    next_mode = WRITE2;
    return output_buffer1;
  }
  if (next_mode == WRITE2){
    next_mode = CHECKSUM;
    return output_buffer2;
  }
  if (next_mode == CHECKSUM){
    next_mode = READ;
    return checksum;
  }
  if (data == RESEND){
    return RESEND;
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