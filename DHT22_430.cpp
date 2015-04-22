//
// DHT22_430.cpp 
// Library C++ code
// ----------------------------------
// Developed with embedXcode 
// http://embedXcode.weebly.com
//
// Project DHT22_430_main
//
// Created by Rei VILO, 17/06/12 11:55
// http://embeddedcomputing.weebly.com
//	
//
// Copyright © Rei VILO, 2012
// Licence CC = BY NC SA
//
// See LocalLibrary.h and ReadMe.txt for references
//


#include "DHT22_430.h"

#if defined(__MSP430G2452__) || defined(__MSP430G2553__) || defined(__MSP430G2231__) \
	|| defined(__MSP430F5529__) || defined (__MSP430FR5739) || defined (__MSP430FR5969__) // MSP430 family
	#define DHT22_DELAY_US 3
	#define DHT22_CYCLES_CNT 8
#else // Other boards, tested only with Stellaris Launchpad (EK-LM4F120XL)
	#define DHT22_DELAY_US 1
	#define DHT22_CYCLES_CNT 13
#endif

DHT22::DHT22(uint8_t pin) {
  _pin = pin;
  _lastMillis = 0;
  _lastResult = false;
}

void DHT22::begin() {
  pinMode(_pin, INPUT);
  digitalWrite(_pin, HIGH);
}

int32_t DHT22::temperatureX10() {
  return _temperature;
}

int32_t DHT22::humidityX10(void) {
  return _humidity;
}


boolean DHT22::get() {
  uint8_t lastState = HIGH;
  uint16_t counter = 0;
  uint8_t j = 0, i;
  
  // sensor frequency = 0.5 Hz
  // check last time + 2000 ms
  if (millis() < _lastMillis) {
    return _lastResult; // return last correct measurement
  }
  _lastMillis = millis() + 2000;
  
  _temperature = 0;
  _humidity    = 0;
  _lastResult  = false;
  
  // pull the pin high and wait 250 milliseconds
  digitalWrite(_pin, HIGH);
  delay(250);
  
  data[0] = data[1] = data[2] = data[3] = data[4] = 0;
  
  // now pull it low for ~20 milliseconds
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  delay(20);
 
  noInterrupts();
  
  digitalWrite(_pin, HIGH);
  delayMicroseconds(40);
  pinMode(_pin, INPUT);
  
  // read in timings
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (digitalRead(_pin) == lastState) {
      counter++;

      // LaunchPad implementation
      // LaunchPad faster than Arduino
      // 1. replace delayMicroseconds(1) with delayMicroseconds(3)
      // or
      // 2. compare counter to a higher number
      // by energia » Tue Jun 26, 2012 9:24 pm
      // see http://www.43oh.com/forum/viewtopic.php?p=20821#p20821
      delayMicroseconds(DHT22_DELAY_US);

      if (counter == 255) {
        break;
      }
    }

    lastState = digitalRead(_pin);
    
    if (counter == 255) break;
    
    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      data[j/8] <<= 1;
      
      // 1. replace delayMicroseconds(1) with delayMicroseconds(3)
      // or
      // 2. compare counter to a higher number
      // by energia » Tue Jun 26, 2012 9:24 pm
      // see http://www.43oh.com/forum/viewtopic.php?p=20821#p20821
      //      if (counter > 8)
      if (counter > DHT22_CYCLES_CNT)
           data[j/8] |= 1;
      j++;
    }
  }
  
  interrupts();
   // check we read 40 bits and that the checksum matches
  if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xff)) ) {
    
    _temperature = data[2] & 0x7f;
    _temperature *= 256;
    _temperature += data[3];
    if (data[2] & 0x80) _temperature *= -1;
    
    _humidity = data[0];
    _humidity *= 256;
    _humidity += data[1];
    
    _lastResult = true;
  }

  return _lastResult;
}




