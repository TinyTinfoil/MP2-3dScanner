#include <Servo.h>

Servo pan, tilt;
uint8_t panX = 0, tiltY = 0, newpanX = 0, newtiltY = 0;
uint8_t pan_pin = 10;
uint8_t tilt_pin = 11;
uint8_t analog = A0;
uint16_t LOOP_INTERVAL = 20;
long loop_time = 0;

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
  int pan_calibration = 55;
  int tilt_calibration = 45;
  pan.write(pan_calibration);
  tilt.write(tilt_calibration);
  Serial.println("Ready");
  while (Serial.available() - 4 < 0) {}     //wait for data available (any)
  uint16_t sensor_vals[100];
  for (int i = 0; i < 100; i++){
    sensor_vals[i] = read_sensor();
    delay(1);
  }
  Serial.println(findMedian(sensor_vals,100));
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
