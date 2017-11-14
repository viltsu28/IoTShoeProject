#include "HttpClient.h"
int led2 = D7;
int photoCellPin = A0;
int lastValue = 0;

HttpClient http;
http_header_t headers[] = {
   { "Content-Type", "application/json" },
   { NULL, NULL }
};
http_request_t request;
http_response_t response;

void getRequest() {

  request.path = "/photoresistor.json";
  request.body = "";

  http.get(request, response, headers);
  printResponse(response);
}

void printResponse(http_response_t &response) {
  Serial.println("HTTP Response: ");
  Serial.println(response.status);
  Serial.println(response.body);
}

void setup() {

  Serial.begin(9600);
  pinMode(led2, OUTPUT);
  request.port = 80;
  request.hostname = "iotproject-c.firebaseio.com";


}

void loop() {

  digitalWrite(led2, HIGH);

  // We'll leave it on for 1 second...
  delay(1000);
  int value = analogRead(photoCellPin);
  Serial.println(analogRead(photoCellPin));
  digitalWrite(led2, LOW);



  getRequest();


  delay(1000);
//  lastValue = value;
}
