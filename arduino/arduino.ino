#include <Servo.h>

Servo pan, tilt;
uint16_t panX = 0, tiltY = 0, newpanX = 0, newtiltY = 0;
uint8_t pan_pin = 10;
uint8_t tilt_pin = 11;
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
  uint16_t pan_calibration = 55;
  pan_calibration = map(pan_calibration, 0, 180, 1000, 2000);
  pan.writeMicroseconds(pan_calibration);
  Serial.println("Ready");
  while (Serial.available() - 4 < 0) {}     //wait for data available (any)
  
  uint16_t last_sensor_data = 0;
  uint8_t pos;
  char s[4];
  char t[3];
  char p[3];
  uint16_t num_points = (2000 - pan_calibration)/100;
  uint16_t sensor_data[num_points];
  uint16_t panHistory[num_points];
  Serial.print("[");
  for (tiltY = 1000; tiltY <= 2000; tiltY += 10){
  // delay(5000);
    Serial.print("[");
    uint16_t i = 0;
    for (panX = pan_calibration; panX <= 2000; panX += 100){
      pan.writeMicroseconds(panX);              // tell servo to "scan", left to right
      delay(100); // wait for it to go there
      sensor_data[i] = read_sensor(); // read the data then
      panHistory[i] = panX;
      i++;
    }
    for (uint16_t i = 0; i < num_points; i++){
      Serial.print("("+String(sensor_data[i])+","+String(tiltY)+","+String(panHistory[i])+")");
      Serial.print(",");
    }    
    Serial.println("],");
    pan.writeMicroseconds(pan_calibration);
    delay(200);
    tilt.writeMicroseconds(tiltY);
    delay(200);
  }
  Serial.print("]");
}


void loop() {
}

long int t;
uint16_t x,y,z,res;

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
