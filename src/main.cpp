/*
 * bare main file
 */

 #include <Arduino.h>
 
 void setup()
 {
    Serial.begin(9600);
 }
 
 void loop()
 {
    Serial.println("running..."); 
    delay (1000);
 }
 