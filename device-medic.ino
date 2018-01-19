//*** NeoPixel ***//
// https://github.com/adafruit/Adafruit_NeoPixel/blob/master/examples/simple/simple.ino
// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

//*** EC-11 rotary encoder ***//
// https://little-scale.blogspot.com.au/2016/07/simple-midi-rotary-encoder-with-teensy.html
// http://milkcrate.com.au/_other/downloads/arduino/Teensy_Rotary_Encoder.ino
// Thanks Seb!

#include <Adafruit_NeoPixel.h>
#include <Encoder.h>

#define PIXPIN	PIN_D7
#define LED	11
#define RINGS	2
#define PIXELS	16

#define MIDI_CHANNEL 1

int frame = 0;

// int values[2] = {0, 0};
int values[2] = {0, 0};
int values_previous[2] = {0, 0};
int value_frame[2] = {0, 0};

int knob_buttons_source[2] = {PIN_F5, PIN_F4};
int knob_buttons[2] = {0, 0};
int knob_buttons_previous[2] = {0, 0};

int buttons_source[2] = {PIN_F6, PIN_B6};
int buttons_leds[2] = {PIN_F7, PIN_B5};
int buttons[2] = {0, 0};
int buttons_previous[2] = {0, 0};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXELS * RINGS, PIXPIN, NEO_GRB + NEO_KHZ800);
Encoder encoders[2] = {
  Encoder(PIN_D0, PIN_D1),
  Encoder(PIN_D2, PIN_D3)
};

int colors[6] = {
  pixels.Color(0, 1, 0),
  pixels.Color(0, 0, 1),
  pixels.Color(0, 1, 1),
  pixels.Color(1, 1, 0),
  pixels.Color(1, 0, 0),
  pixels.Color(1, 0, 1),
};

void setup() {
  pinMode(PIXPIN, OUTPUT);
  pinMode(PIN_F4, INPUT_PULLUP);
  pinMode(PIN_F5, INPUT_PULLUP);
  
  for (int i=0; i<2; i++) {
    pinMode(knob_buttons_source[i], INPUT_PULLUP);
    pinMode(buttons_source[i], INPUT_PULLUP);
    pinMode(buttons_leds[i], OUTPUT);
  }
  
  pixels.begin();
  
  // flash LED to indicate startup
  pinMode(LED, OUTPUT);
  for (int i=0; i<6; i++) {  
    digitalWrite(LED, i % 2 ? HIGH : LOW);
    delay(250);
  }
}

void loop() {
  for (int i=0; i<2; i++) {
    values[i] = encoders[i].read();
    // check knob values
    if (values_previous[i] / 4 != values[i] / 4) {
      // check for fast Without pushbutton code
      int frames = frame - value_frame[i];
      int diff = values[i] - values_previous[i];
      int mult = 1;
      if (frames < 100) {
      	mult = 2;
      }
      if (frames < 50) {
        mult = 4;
      }
      if (frames < 10) {
        mult = 8;
      }
      values[i] = values_previous[i] + (diff * mult);
      values[i] = values[i] < 0 ? 0 : values[i];
      values[i] = values[i] > 511 ? 511: values[i];
      value_frame[i] = frame;
      // send new value
      values_previous[i] = values[i];
      usbMIDI.sendControlChange(i + 1, values_previous[i] / 4, MIDI_CHANNEL);
      int offset = (!i) * PIXELS;
      for (int l=0; l < PIXELS; l++) {
        int remainder = min(max((values_previous[i]) / 4 - l * 8, 0), 8);
        pixels.setPixelColor((PIXELS - 1 - l) + offset, remainder * colors[i]);
      }
      pixels.show();
    }
    
    encoders[i].write(values[i]);
    
    // check button values
    knob_buttons[i] = digitalRead(knob_buttons_source[i]);
    if (knob_buttons_previous[i] != knob_buttons[i]) {
      knob_buttons_previous[i] = knob_buttons[i];
      //usbMIDI.sendControlChange(2 + i, (1 - knob_buttons_previous[i]) * 127, MIDI_CHANNEL);
    }
  }
  
  for (int i=0; i<2; i++) {
    int button_state = !digitalRead(buttons_source[i]);
    if (button_state != buttons_previous[i]) {
      // digitalWrite(buttons_leds[i], button_state);
      if (button_state) {
        usbMIDI.sendNoteOn(i + 1, 127, MIDI_CHANNEL);
      } else {
        usbMIDI.sendNoteOff(i + 1, 0, MIDI_CHANNEL);
      }
      buttons_previous[i] = button_state;
    }
  }
  
  // check if there is any incoming MIDI data
  if (usbMIDI.read()) {
    int type = usbMIDI.getType();
    int channel = usbMIDI.getChannel();
    if (channel == MIDI_CHANNEL) {
      if (type == 1) {
          int note = usbMIDI.getData1();
          int velocity = usbMIDI.getData2();
          if (note == 1 || note == 2) {
            digitalWrite(buttons_leds[note - 1], 1);
	  }
        } else if (type == 0) {
          int note = usbMIDI.getData1();
          int velocity = usbMIDI.getData2();
          if (note == 1 || note == 2) {
            digitalWrite(buttons_leds[note - 1], 0);
	  }
        } else if (type == 3) {
          int controller = usbMIDI.getData1();
          int value = usbMIDI.getData2();
          if (controller == 0 || controller == 1) {
            // TODO: set value
	  }
        } else {
          int d1 = usbMIDI.getData1();
	  int d2 = usbMIDI.getData2();
        }
      }
  }
  
  delay(1);
  frame += 1;
}
