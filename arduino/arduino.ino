#include <Servo.h>

Servo pan, tilt;
uint16_t panX = 0, tiltY = 0;
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
  uint16_t pan_start = 20;
  uint16_t pan_end = 80;
  pan_start = map(pan_start, 0, 180, 544, 2400);
  pan_end = map(pan_end, 0, 180, 544, 2400);
  uint16_t tilt_start = 50 - 20; // midpoint at 50 degrees
  uint16_t tilt_end = 50 + 20;
  tilt_start = map(tilt_start, 0, 180, 544, 2400);
  tilt_end = map(tilt_end, 0, 180, 544, 2400);
  pan.writeMicroseconds(pan_start);
  tilt.writeMicroseconds(tilt_start);
  Serial.println("Ready");
  while (Serial.available() - 4 < 0) {}     //wait for data available (any)
  
  uint16_t last_sensor_data = 0;
  int pan_res = 10;
  int tilt_res = 10;
  uint16_t num_points = (pan_end - pan_start)/pan_res;
  uint16_t sensor_data[num_points];
  uint16_t panHistory[num_points];
  Serial.println("[");
  for (tiltY = tilt_start; tiltY <= tilt_end; tiltY += tilt_res){
    uint16_t i = 0;
    for (panX = pan_start; panX <= pan_end; panX += pan_res){
      pan.writeMicroseconds(panX);              // tell servo to "scan", left to right
      delay(100); // wait for it to go there
      uint16_t sensor_vals[100];
      for (int i = 0; i < 100; i++){
        sensor_vals[i] = read_sensor();
        delay(1);
      }
      sensor_data[i] = findMedian(sensor_vals,100);
      panHistory[i] = panX;
      i++;
    }
    for (uint16_t i = 0; i < num_points; i++){
      Serial.print(String(panHistory[i])+" "+String(tiltY)+" "+String(sensor_data[i]));
      Serial.print(";");
    }
    pan.writeMicroseconds(pan_start);
    delay(200);
    tilt.writeMicroseconds(tiltY);
    delay(200);
    Serial.println();
  }
  Serial.println("]");
}

uint16_t cmpfunc(const void* a, const void* b)
{
    return (*(uint16_t*)a - *(uint16_t*)b);
}

uint16_t findMedian(uint16_t a[], uint16_t n)
{
    // First we sort the array
    qsort(a, n, sizeof(uint16_t), cmpfunc);
 
    // check for even case
    if (n % 2 != 0)
        return (uint16_t)a[n / 2];
 
    return (uint16_t)(a[(n - 1) / 2] + a[n / 2]) / 2;
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
