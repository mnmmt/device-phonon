//*** EC-11 rotary encoder ***//
// https://little-scale.blogspot.com.au/2016/07/simple-midi-rotary-encoder-with-teensy.html
// http://milkcrate.com.au/_other/downloads/arduino/Teensy_Rotary_Encoder.ino
// Thanks Seb!

#include <Encoder.h>

#define LED	13

#define MIDI_CHANNEL 1

#define NUM_VIRTUAL 127

int frame = 0;

int value_virtual_knobs[1] = {0};
int selected = 0;
int last = 0;
int lastTime = 0;

Encoder knob = Encoder(2, 4);

void setup() {
  pinMode(3, OUTPUT_OPENDRAIN);

  // flash LED to indicate startup
  pinMode(LED, OUTPUT);
  for (int i=0; i<6; i++) {
    digitalWrite(LED, i % 2 ? HIGH : LOW);
    delay(250);
  }
  
  //knob.write(64);
  
  //Serial.begin(9600);
  //Serial.println("Hello World...");
}

void loop() {
  int raw = knob.read();
  //knob.write(64);
  
  if (raw != last) {
    if (lastTime - millis() > 20) {
      int previous_value = value_virtual_knobs[selected];
      value_virtual_knobs[selected] -= raw - last;
      if (value_virtual_knobs[selected] > 127) {
        value_virtual_knobs[selected] = 127;
      }
      if (value_virtual_knobs[selected] < 0) {
        value_virtual_knobs[selected] = 0;
      }
      if (previous_value !=  value_virtual_knobs[selected]) {
        usbMIDI.sendControlChange(selected + 1, value_virtual_knobs[selected], MIDI_CHANNEL);
      }
      lastTime = millis();
    }
    last = raw;
  }
  
  //knob.write(64);
  
  // check if there is any incoming MIDI data
  /*if (usbMIDI.read()) {
    int type = usbMIDI.getType();
    int channel = usbMIDI.getChannel();
    if (channel == MIDI_CHANNEL) {
      if (type == 1) {
          int note = usbMIDI.getData1();
          int velocity = usbMIDI.getData2();
        } else if (type == 0) {
          int note = usbMIDI.getData1();
          int velocity = usbMIDI.getData2();
        } else if (type == 3) {
          int controller = usbMIDI.getData1();
          int value = usbMIDI.getData2();
          if (controller >= 0 && controller < NUM_VIRTUAL) {
            value_virtual_knobs[controller] = value;
	  }
        } else {
          int d1 = usbMIDI.getData1();
	  int d2 = usbMIDI.getData2();
        }
      }
  }*/
  
  //delay(20);
  //frame += 1;
}
