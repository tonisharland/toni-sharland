/***************************************************************************
  This is a library for the APDS9960 digital proximity, ambient light, RGB, and gesture sensor

  This sketch puts the sensor in proximity mode and enables the interrupt
  to fire when proximity goes over a set value

  Designed specifically to work with the Adafruit APDS9960 breakout
  ----> http://www.adafruit.com/products/3595

  These sensors use I2C to communicate. The device's I2C address is 0x39

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Dean Miller for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/
#include <SPI.h>
#include <Ethernet.h>
#include <PubNub.h>
#include "Adafruit_APDS9960.h"

byte mac[] = {0x00, 0x0E, 0xEF, 0x00, 0x00, 0x64};

char pubkey[] = "pub-c-db8ffe75-7878-4859-b6ef-d111c6719b3b";
char subkey[] = "sub-c-c8d2909e-3f07-11e8-91e7-8ad1b2d46395";
char channel[] = "iot";

//the pin that the interrupt is attached to
const int INT_PIN = 5;

// value to determine the size of the distance array.
const int numDistance = 100;

//create the APDS9960 object
Adafruit_APDS9960 apds;

//bringing in vibration sensor
int active_buzzer = 12;

//distance
int distanceone;

int lastDistance = 0;

int distance[numDistance];      // the distance from the analog input
int distIndex = 0;              // the index of the current distance
int total = 0;                  // the running total
int average = 0;                // the average

int inputPin = A4;

int lastCount = 0;

void setup() {

  pinMode(active_buzzer, OUTPUT);//vibration
  
  pinMode(5, OUTPUT);//proximity

  // initialize all the distances to 0:
  for (int thisDistance = 0; thisDistance < numDistance; thisDistance++) {
    distance[thisDistance] = 0;
  }

  if (!apds.begin()) {
    Serial.println("failed to initialize device! Please check your wiring.");
  }//proximity
  else Serial.println("Device initialized!");//proximity

  //enable proximity mode
  apds.enableProximity(true);

  //set the interrupt threshold to fire when proximity reading goes above 175
  apds.setProximityInterruptThreshold(0, 175);

  //enable the proximity interrupt
  apds.enableProximityInterrupt();
  
  while(!Ethernet.begin(mac)) {
    Serial.println("Ethernet setup error!");
    delay(1000);
  }
  
  Serial.println("Ethernet setup");
  PubNub.begin(pubkey,subkey);
  Serial.println("Pubnub set up");
  
  Serial.begin(9600);//proximity
  
}

void loop() {  

  
  distanceone = apds.readProximity();
  char msg[32] = "";

  //print the proximity reading when the interrupt pin goes low
  if (!digitalRead(INT_PIN)) {
    //Serial.println(apds.readProximity());

    //clear the interrupt
    apds.clearInterrupt();
  }

  // subtract the last distance:
  total = total - distance[distIndex];
  // read from the sensor:
  distance[distIndex] = distanceone;
  // add the distance to the total:
  total = total + distance[distIndex];
  // advance to the next position in the array:
  distIndex = distIndex + 1;

  // if we're at the end of the array...
  if (distIndex >= numDistance) {
    // ...wrap around to the beginning:
    distIndex = 0;
  }

  // calculate the average:
  average = total / numDistance;

  //Serial.println(average);

  // send it to the computer as ASCII digits
  if (average != lastDistance) {
    // Serial.println(average);

    if (distanceone < 50) {

     strcat(msg,"{\"feedback\":[\"FAR\"]}");
     
     // msg = "{\"feedback\":[\"FAR\"]}";
      
      
      Serial.println("far");
      //on and off for vibration sensor
      //for (int i = 0; i < 255; i++){
      analogWrite(active_buzzer, 0);
      //delay(20);
      //  }
    } else {
     strcat(msg,"{\"feedback\":[\"CLOSE\"]}");
      //msg = "{\"feedback\":[\"CLOSE\"]}";
      Serial.println("close");
      //on and off for vibration sensor
      // digitalWrite(active_buzzer,LOW);
      analogWrite(active_buzzer, 255);

    }
  }
  //Serial.println(average);
  lastDistance = average;

  

    Ethernet.maintain();
    EthernetClient *client;

    //Serial.println(lastCount);
    
    if((millis() / 1000) > lastCount){
      
      Serial.println("send data");
     
      client = PubNub.publish(channel, msg);
    
      if(!client) {
        Serial.println("publishing error");
      } else {
        Serial.println("published");
        client->stop();
      }
      
      lastCount++;
      
      
    }


  
   delay(1000);        // delay in between reads for stability

  
}


