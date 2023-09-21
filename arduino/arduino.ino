#include <Servo.h>

Servo pan, tilt;
uint8_t panX = 0, tiltY = 0, newpanX = 0, newtiltY = 0;
uint8_t pan_pin, tilt_pin;
uint8_t analog = A0;
uint16_t LOOP_INTERVAL = 20;



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
  move = false;
  Serial.println("Ready");
  while (Serial.available() - 4 < 0) {}     //wait for data available (any)
}

uint16_t last_sensor_data = 0;

void loop() {

  // if (move){
  //   newpanX = 90;
  //   newtiltY = 90;
  //   for (uint8_t pos = panX; (panX>newpanX? pos >= newpanX : pos <= newpanX ); (panX>newpanX? pos -=1 :pos +=1 )) { // goes from 0 degrees to 180 degrees
  //     // in steps of 1 degree
  //     pan.write(pos);              // tell servo to go to position in variable 'pos'
  //     delay(15);                       // waits 15ms for the servo to reach the position
  //     panX = pos;
  //   }
  //   for (uint8_t pos = tiltY; (tiltY> newtiltY? pos >= newtiltY : pos <= newtiltY ); (tiltY>newtiltY? pos -=1 :pos +=1 )) { // goes from 180 degrees to 0 degrees
  //     tilt.write(pos);              // tell servo to go to position in variable 'pos'
  //     delay(15);                       // waits 15ms for the servo to reach the position
  //     tiltY = pos;
  //   }

  // }
  Serial.println(read_sensor());
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
