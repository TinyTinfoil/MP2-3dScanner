#include <Servo.h>
// Copied from main program
Servo pan, tilt;
uint8_t panX = 0, tiltY = 0;
uint8_t pan_pin = 10;
uint8_t tilt_pin = 11;
uint8_t analog = A0;
uint16_t LOOP_INTERVAL = 20;
long loop_time = 0;

void setup() {
  // put your setup code here, to run once:
  // start the serial port
  long baudRate = 9600;       //The baudRate for sending & receiving programs must match
  Serial.begin(baudRate);     
  Serial.setTimeout(1); // Set the serial timeout to 1 ms
  pan.attach(pan_pin); // Attach servo pins
  tilt.attach(tilt_pin);
  loop_time = millis(); // Used for measuring sensor
  int pan_calibration = 55; // Set pan and tilt to calibrated values to be roughly straight onto the target
  int tilt_calibration = 45;
  pan.write(pan_calibration); 
  tilt.write(tilt_calibration);
  Serial.println("Ready"); // Send ready message to serial monitor
  while (Serial.available() - 4 < 0) {}     //Wait for data available (any)
  uint16_t sensor_vals[100]; // Create array to store sensor values
  for (int i = 0; i < 100; i++){
    sensor_vals[i] = read_sensor(); // Read sensor 100 times and store values in array
    delay(1);
  }
  Serial.println(findMedian(sensor_vals,100)); // Send median of sensor values to serial monitor
  //Exit
}

// Comparing function for qsort - copied from main program
uint16_t cmpfunc(const void* a, const void* b)
{
    return (*(uint16_t*)a - *(uint16_t*)b);
}

// findMedian function - copied from main program
uint16_t findMedian(uint16_t a[], uint16_t n)
{
    qsort(a, n, sizeof(uint16_t), cmpfunc);
    if (n % 2 != 0)
        return (uint16_t)a[n / 2];
    return (uint16_t)(a[(n - 1) / 2] + a[n / 2]) / 2;
}

void loop() {
  // Nothing needs to be done here continuously
}

// Same as in the main program

bool it_is_time(uint32_t t, uint32_t t0, uint16_t dt) {
  return ((t >= t0) && (t - t0 >= dt)) ||         
            ((t < t0) && (t + (~t0) + 1 >= dt));  
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
