// --------------------------------------
// i2c_scanner
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, Juni 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
// Version 6, November 27, 2015.
//    Added waiting for the Leonardo serial communication.
//
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//
/*
#include <Wire.h>


void setup()
{
  Wire.begin();

  Serial.begin(9600);
  while (!Serial);             // Leonardo: wait for serial monitor
  Serial.println("\nI2C Scanner");
}


void loop()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(500);           // wait 5 seconds for next scan
}



*/






// Include the library:
#include "SparkFunMMA8452Q.h"
#include "HttpClient.h"
#include <cstdio>
#include <cstring>
#include "ArduinoJson.h"
#include "MQTT.h"

const int sampleSize = 100;
// Create an MMA8452Q object, used throughout the rest of the sketch.
MMA8452Q accel; // Default constructor, SA0 pin is HIGH

char MQTT_domain[] = "backend.abelonditi.com";
byte MQTT_server[] = {34,241,123,175};

// The above works if the MMA8452Q's address select pin (SA0) is high.
// If SA0 is low (if the jumper on the back of the SparkFun MMA8452Q breakout
// board is closed), initialize it like this:
// MMA8452Q accel(MMA8452Q_ADD_SA0_);

HttpClient http;
http_header_t headers[] = {
   { "Content-Type", "application/json" },
   { NULL, NULL }
};
http_request_t request;
http_response_t response;

void printResponse(http_response_t &response) {
  Serial.println("HTTP Response: ");
  Serial.println(response.status);
  Serial.println(response.body);
}





//StaticJsonBuffer<20000> jsonBuffer;
void postAccelerometer(float *x_array, float *y_array, float *z_array, int *time) {
  DynamicJsonBuffer jsonBuffer;
  char buffer[5000];
  //sprintf(buffer, "{\"data\":[{\"timestamp\":%d,\"accel_x\":%f,\"accel_y\":%f,\"accel_z\":%f},{\"timestamp\":%d,\"accel_x\":%f,\"accel_y\":%f,\"accel_z\":%f}]}\r\n",12345, 0.22,0.33,0.44,123,0.11,0.41,0.14);

  JsonObject& root = jsonBuffer.createObject();
  JsonArray& data = root.createNestedArray("data");

  for(int i = 0; i < sampleSize; i++){
    JsonObject& object = data.createNestedObject();
    object["timestamp"] = time[i];
    object["accel_x"] = x_array[i];
    object["accel_y"] = y_array[i];
    object["accel_z"] = z_array[i];
  }

  root.printTo((char*)buffer, root.measureLength() + 1);

  //Serial.println(buffer);
  request.path = "/sensors";
  request.body = buffer;
  http.post(request, response, headers);
  //printResponse(response);
}








void setup()
{
    Serial.begin(9600);
    request.port = 80;
    request.hostname = "iot-shoe.herokuapp.com";

	// Initialize the accelerometer with begin():
	// begin can take two parameters: full-scale range, and output data rate (ODR).
	// Full-scale range can be: SCALE_2G, SCALE_4G, or SCALE_8G (2, 4, or 8g)
	// ODR can be: ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12, ODR_6 or ODR_1
    accel.begin(SCALE_2G, ODR_100); // Set up accel with +/-2g range, and slowest (1Hz) ODR
}

float x_values[sampleSize];
float y_values[sampleSize];
float z_values[sampleSize];
int timestamp[sampleSize];


void loop()
{
	// accel.available() will return 1 if new data is available, 0 otherwise
    static int sampleCount = 0;
    if (accel.available())
    {

		// To update acceleration values from the accelerometer, call accel.read();
        accel.read();
            //timestamp = Time.millis();

        x_values[sampleCount] = accel.cx;
        y_values[sampleCount] = accel.cy;
        z_values[sampleCount] = accel.cz;
        timestamp[sampleCount] = millis();

        /*printAccelGraph(accel.cx, "X", 20, 2.0);  //Print stuff
        printAccelGraph(accel.cy, "Y", 20, 2.0);
        printAccelGraph(accel.cz, "Z", 20, 2.0);*/
        //Serial.println("deem\r\n");

        ++sampleCount;
        if (sampleCount >= sampleSize){
            postAccelerometer(x_values, y_values, z_values, timestamp);
            sampleCount = 0;
            memset(x_values, 0, sizeof(x_values));
            memset(y_values, 0, sizeof(y_values));
            memset(z_values, 0, sizeof(z_values));
            memset(timestamp, 0, sizeof(timestamp));
        }

    }

	// No need to delay, since our ODR is set to 1Hz, accel.available() will only return 1
	// about once per second.
}

// printAccelGraph prints a simple ASCII bar graph for a single accelerometer axis value.
// Examples:
//	printAccelGraph(-0.1, "X", 20, 2.0) will print:
// 		X:                    =|                     (0.1 g)
//	printAccelGraph(1.0, "Z", 20, 2.0) will print:
//		Z:                     |==========           (1.0 g)
// Input:
//	- [value]: calculated value of an accelerometer axis (e.g accel.cx, accel.cy)
//	- [name]: name of the axis (e.g. "X", "Y", "Z")
//	- [numBarsFull]: Maximum number of bars either right or left of 0 point.
//	- [rangeAbs]: Absolute value of the maximum acceleration range
void printAccelGraph(float value, String name, int numBarsFull, float rangeAbs)
{
	// Calculate the number of bars to fill, ignoring the sign of numBars for now.
	int numBars = abs(value / (rangeAbs / numBarsFull));

    Serial.print(name + ": "); // Print the axis name and a colon:

	// Do the negative half of the graph first:
    for (int i=0; i<numBarsFull; i++)
    {
        if (value < 0) // If the value is negative
        {
			// If our position in the graph is in the range we want to graph
            if (i >= (numBarsFull - numBars))
                Serial.print('='); // Print an '='
            else
                Serial.print(' '); // print spaces otherwise
        }
        else // If our value is positive, just print spaces
            Serial.print(' ');
    }

    Serial.print('|'); // Print a pipe (|) to represent the 0-point

	// Do the positive half of the graph last:
    for (int i=0; i<numBarsFull; i++)
    {
        if (value > 0)
        {	// If our position in the graph is in the range we want to graph
            if (i <= numBars)
                Serial.print('='); // Print an '='
            else
                Serial.print(' '); // otherwise print spaces
        }
        else // If value is negative, just print spaces
            Serial.print(' ');
    }

	// To end the line, print the actual value:
    Serial.println(" (" + String(value, 2) + " g)");
}
