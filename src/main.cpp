#include <Arduino.h>
#include <SimpleTempSensorNode.h>


SimpleTempSensorNode *tempSensorNode;

void setup() {
  Serial.begin(115200);
  tempSensorNode = new SimpleTempSensorNode();
  tempSensorNode->setup();
  
}

void loop() {
  tempSensorNode->loop();
}