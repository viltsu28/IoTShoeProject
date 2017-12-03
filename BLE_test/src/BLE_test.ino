/*
 * Project BLE_test
 * Description:
 * Author:
 * Date:
 */

// setup() runs once, when the device is first turned on.

const size_t READ_BUF_SIZE = 64;
char readBuf[READ_BUF_SIZE];
size_t readBufOffset = 0;

void setup() {
  // Put initialization like pinMode and begin functions here.
  Serial1.begin(115200, SERIAL_DATA_BITS_8 | SERIAL_STOP_BITS_1 | SERIAL_PARITY_NO);
  Serial.begin();
  Serial.println("moi");

}

void processBuffer() {
	Serial.println(readBuf);
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.
  if ((Serial.available() > 0) && (Serial1.available() > 0)) {
    int incomingByte = Serial.read();
    Serial1.write(incomingByte);
    char input = Serial1.read();

    /*if (input != -1){
      Serial.write(input);
    }*/
    if (readBufOffset < READ_BUF_SIZE) {
			char c = Serial1.read();
			if (c != '\n') {
				// Add character to buffer
				readBuf[readBufOffset++] = c;
			}
			else {
				// End of line character found, process line
				readBuf[readBufOffset] = 0;
				processBuffer();
				readBufOffset = 0;
			}
		}
		else {
			Serial.println("readBuf overflow, emptying buffer");
			readBufOffset = 0;
		}
  }else{
    Serial.println("serial no available.");
  }


}
