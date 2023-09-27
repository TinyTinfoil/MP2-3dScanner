#include <Servo.h>

Servo pan, tilt;
uint16_t panX = 0, tiltY = 0;
uint8_t pan_pin = 10;
uint8_t tilt_pin = 11;
uint8_t analog = A0;
uint16_t LOOP_INTERVAL = 20;

void setup() {
  // put your setup code here, to run once:
  // start the serial port
  long baudRate = 9600;       // The baudRate for sending & receiving programs must match
  Serial.begin(baudRate);     
  Serial.setTimeout(1); // Set the serial timeout to 1 ms
  pan.attach(pan_pin); // Attach servo pins
  tilt.attach(tilt_pin);
  loop_time = millis();
  uint16_t pan_start = 0; // Set the pan endpoints in degrees
  uint16_t pan_end = 80;
  pan_start = map(pan_start, 0, 180, 544, 2400); // Map degrees to microseconds
  pan_end = map(pan_end, 0, 180, 544, 2400);
  uint16_t tilt_start = 10; // Same for tilt endpoints
  uint16_t tilt_end = 80;
  tilt_start = map(tilt_start, 0, 180, 544, 2400);
  tilt_end = map(tilt_end, 0, 180, 544, 2400);
  pan.writeMicroseconds(pan_start); // Set pan and tilt to start positions
  tilt.writeMicroseconds(tilt_start);
  Serial.println("Ready"); // Send ready message to serial monitor
  while (Serial.available() - 4 < 0) {}     //wait for data available (any)
  int pan_res = 10; // Set the pan and tilt resolutions in microseconds
  int tilt_res = 10;
  uint16_t num_points = (pan_end - pan_start)/pan_res; // Calculate array lengths
  uint16_t sensor_data[num_points]; // Stores sensor data for a horizontal scan
  uint16_t panHistory[num_points]; // Stores servo microsecond data for a horizontal scan
  Serial.println("["); // The data is printed in a format that can be copy-pasted into matlab as a matrix 
  for (tiltY = tilt_start; tiltY <= tilt_end; tiltY += tilt_res){ 
    uint16_t i = 0;
    for (panX = pan_start; panX <= pan_end; panX += pan_res){
      pan.writeMicroseconds(panX);              // tell servo to "scan", left to right
      delay(100); // wait for it to go there
      uint16_t sensor_vals[100]; // Set up an array of 100 samples
      for (int i = 0; i < 100; i++){ // Take 100 samples
        sensor_vals[i] = read_sensor();
        delay(1); // Wait 1 ms between samples to make sure we get 'fresh' data
      }
      sensor_data[i] = findMedian(sensor_vals,100); // Take the median of the 100 samples
      panHistory[i] = panX; // Save the pan position
      i++; // Increment the array indicies
    }
    for (uint16_t i = 0; i < num_points; i++){ // Print all the data for one left to right scan
      Serial.print(String(panHistory[i])+" "+String(tiltY)+" "+String(sensor_data[i]));
      Serial.print(";");
    }
    pan.writeMicroseconds(pan_start); // Return to the start position
    delay(200);
    tilt.writeMicroseconds(tiltY); // Change tilt position
    delay(200);
    Serial.println();
  }
  Serial.println("]");
}

// findMedian function copied from https://www.geeksforgeeks.org/program-for-mean-and-median-of-an-unsorted-array/
// With some modifications to types
uint16_t cmpfunc(const void* a, const void* b)
{
    return (*(uint16_t*)a - *(uint16_t*)b);
}

uint16_t findMedian(uint16_t a[], uint16_t n)
{
    qsort(a, n, sizeof(uint16_t), cmpfunc);
    if (n % 2 != 0)
        return (uint16_t)a[n / 2];
    return (uint16_t)(a[(n - 1) / 2] + a[n / 2]) / 2;
}

void loop() {
  // Nothing needs to be done in the loop, as we only need to "loop" once for data collection in the setup function.
}




// Support variables for reading the IR sensor
long int t;
uint16_t x,y,z,res;

// Copied from https://github.com/bminch/PIE/blob/main/sharp_distance_test.ino

/*
** Copyright (c) 2021, Bradley A. Minch
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met: 
** 
**     1. Redistributions of source code must retain the above copyright 
**        notice, this list of conditions and the following disclaimer. 
**     2. Redistributions in binary form must reproduce the above copyright 
**        notice, this list of conditions and the following disclaimer in the 
**        documentation and/or other materials provided with the distribution. 
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
** POSSIBILITY OF SUCH DAMAGE.
*/

/*
** Returns a boolean value that indicates whether the current time, t, is later than some prior 
** time, t0, plus a given interval, dt.  The condition accounts for timer overflow / wraparound.
*/
bool it_is_time(uint32_t t, uint32_t t0, uint16_t dt) {
  return ((t >= t0) && (t - t0 >= dt)) ||         // The first disjunct handles the normal case
            ((t < t0) && (t + (~t0) + 1 >= dt));  //   while the second handles the overflow case
}

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
