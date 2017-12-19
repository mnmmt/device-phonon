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

int knob_buttons[2] = {0, 0};
int knob_buttons_previous[2] = {0, 0};

int buttons[2] = {0, 0};
int buttons_previous[2] = {0, 0};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXELS * RINGS, PIXPIN, NEO_GRB + NEO_KHZ800);
Encoder enc_one(PIN_D0, PIN_D1);
Encoder enc_two(PIN_D2, PIN_D3);

void setup() {
  pinMode(PIXPIN, OUTPUT);
  pinMode(PIN_F4, INPUT_PULLUP);
  pinMode(PIN_F5, INPUT_PULLUP);
  
  pinMode(PIN_F6, INPUT_PULLUP); 
  pinMode(PIN_F7, OUTPUT);
  
  pixels.begin();
  
  // flash LED to indicate startup
  pinMode(LED, OUTPUT);
  for (int i=0; i<6; i++) {  
    digitalWrite(LED, i % 2 ? HIGH : LOW);
    delay(250);
  }
}

void loop() {
  values[0] = enc_one.read();
  values[1] = enc_two.read();
  
  for (int i=0; i<2; i++) {
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
      int offset = i * PIXELS;
      for (int l=0; l < PIXELS; l++) {
        int remainder = min(max((values_previous[i]) / 4 - l * 8, 0), 8);
        pixels.setPixelColor((PIXELS - 1 - l) + offset, pixels.Color(0, remainder, 0));
      }
      pixels.show();
    }
    
    encoders[i].write(values[i]);
    
    // check button values
    knob_buttons[i] = digitalRead(i ? PIN_F4 : PIN_F5);
    if (knob_buttons_previous[i] != knob_buttons[i]) {
      knob_buttons_previous[i] = knob_buttons[i];
      usbMIDI.sendControlChange(2, (1 - knob_buttons_previous[i]) * 127, MIDI_CHANNEL);
    }
  }
  
  for (int i=0; i<1; i++) {
    int button_state = !digitalRead(i ? PIN_B6 : PIN_F6);
    if (button_state != buttons_previous[i]) {
      digitalWrite(i ? PIN_B5 : PIN_F7, button_state);
      if (button_state) {
        usbMIDI.sendNoteOn(i + 1, 127, MIDI_CHANNEL);
      } else {
        usbMIDI.sendNoteOff(i + 1, 0, MIDI_CHANNEL);
      }
      buttons_previous[i] = button_state;
    }
  }
  
  enc_one.write(values[0]);
  enc_two.write(values[1]);
  
  delay(1);
  frame += 1;
}
