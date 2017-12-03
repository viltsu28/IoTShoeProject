#include "application.h"
#include "SparkFunMMA8452Q.h"
#include "HttpClient.h"
#include "MQTT.h"
#include <cstdio>
#include <cstring>




MMA8452Q accel; // Default constructor, SA0 pin is HIGH
Thread* getAccelerometer;
Thread* sendAccelerometer;

void callback(char* topic, byte* payload, unsigned int length);

MQTT client("backend.abelonditi.com", 1883, callback);

// recieve message
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    delay(1);
}

os_queue_t accelDataQueue;

typedef struct {
  float x_accel;
  float y_accel;
  float z_accel;
  int timestamp;
} accelData;

HttpClient http;
http_header_t headers[] = {
   { "Content-Type", "application/json" },
   { NULL, NULL }
};
http_request_t request;
http_response_t response;

os_thread_return_t getAccelerometerData(void* param){


    for(;;) {
      if (accel.available()){

  // To update acceleration values from the accelerometer, call accel.read();
      accel.read();

      accelData data;

      data.timestamp = millis();
      data.x_accel = accel.cx;
      data.y_accel = accel.cy;
      data.z_accel = accel.cz;

      os_queue_put(accelDataQueue, &data, CONCURRENT_WAIT_FOREVER, NULL);

      }
    }
}


os_thread_return_t sendAccelerometerData(void* param){


    for(;;) {
      accelData data;

      if (os_queue_take(accelDataQueue, &data, CONCURRENT_WAIT_FOREVER, NULL) == 0){
        char buffer[100];
        sprintf(buffer, "{\"timestamp\":%d,\"accel_x\":%f,\"accel_y\":%f,\"accel_z\":%f}\r\n", data.timestamp, data.x_accel, data.y_accel, data.z_accel);
        if (client.isConnected()) {
            client.publish("team16/test/accelerometer",buffer);
        }else{
            Serial.println("No Mqtt connection");
        }
      }
    }
}

void setup() {
    Serial.begin(9600);

    request.port = 80;
    request.hostname = "iot-shoe.herokuapp.com";
    client.connect("team16", "team16", "nqnqb");

    os_queue_create(&accelDataQueue, sizeof(accelData), 500, NULL);

	// Initialize the accelerometer with begin():
	// begin can take two parameters: full-scale range, and output data rate (ODR).
	// Full-scale range can be: SCALE_2G, SCALE_4G, or SCALE_8G (2, 4, or 8g)
	// ODR can be: ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12, ODR_6 or ODR_1
    accel.begin(SCALE_2G, ODR_100); // Set up accel with +/-2g range, and slowest (1Hz) ODR

    getAccelerometer = new Thread("get data", getAccelerometerData);
    sendAccelerometer = new Thread("send data", sendAccelerometerData);
}

void loop() {
    // Nothing here.
}
