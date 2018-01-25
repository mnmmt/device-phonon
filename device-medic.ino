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

#define RATEBUFFER 16
#define NUM_VIRTUAL 6

int frame = 0;

uint8_t values[2][RATEBUFFER];
int change[2] = {0, 0};

int value_virtual_knobs[6] = {0, 0, 0, 0, 0, 0};
int value_virtual_button[6] = {0, 0, 0, 0, 0, 0};
int selected = 0;

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

uint32_t colors[6] = {
  pixels.Color(1, 0, 2),
  pixels.Color(0, 0, 2),
  pixels.Color(0, 2, 0),
  pixels.Color(2, 2, 0),
  pixels.Color(2, 1, 0),
  pixels.Color(2, 0, 0),
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
  
  for (int i=0; i<2; i++) {
    for (int r=0; r<RATEBUFFER; r++) {
      values[i][r] = 0;
    }
  }
  
  pixels.begin();
  
  // flash LED to indicate startup
  pinMode(LED, OUTPUT);
  for (int i=0; i<6; i++) {
    digitalWrite(LED, i % 2 ? HIGH : LOW);
    delay(250);
  }
  
  //Serial.begin(9600);
  //Serial.println("Hello World...");
}

void loop() {
  for (int i=0; i<2; i++) {
    // index into virtual knob/button tables
    int lookup = (selected + i) % NUM_VIRTUAL;

    // take a bead on each encoder
    int raw = encoders[i].read();
    int bead = raw / 4;
    if (raw != change[i]) {
      change[i] = raw;
      usbMIDI.sendPitchBend(raw + 1024, MIDI_CHANNEL);
    }
    encoders[i].write(encoders[i].read() % 4);

    values[i][frame % RATEBUFFER] = bead;
    int rate = 0;
    for (int r=0; r<RATEBUFFER; r++) {
      rate = rate + min(abs(values[i][r]), 1);
    }
    
    int new_knob_value = min(max(0, value_virtual_knobs[lookup] + bead * rate), 127);
    if (new_knob_value != value_virtual_knobs[lookup]) {
      value_virtual_knobs[lookup] = new_knob_value;
      usbMIDI.sendControlChange(lookup, new_knob_value, MIDI_CHANNEL);
    }
    
    int offset = (!i) * PIXELS;
    for (int l=0; l < PIXELS; l++) {
      int remainder = min(max(value_virtual_knobs[lookup] - l * 8, 0), 8);
      pixels.setPixelColor((PIXELS - 1 - l) + offset, remainder * colors[lookup]);
    }
    pixels.show();
    
    // check knob button values
    knob_buttons[i] = digitalRead(knob_buttons_source[i]);
    if (knob_buttons_previous[i] != knob_buttons[i]) {
      knob_buttons_previous[i] = knob_buttons[i];
      if (knob_buttons[i]) {
        selected = ((selected + (((!i) * 2) - 1)) + NUM_VIRTUAL) % NUM_VIRTUAL;
        usbMIDI.sendControlChange(13, selected, MIDI_CHANNEL);
      }
      usbMIDI.sendControlChange(14 + (!i), (1 - knob_buttons[i]) * 127, MIDI_CHANNEL);
    }
    
    int button_state = !digitalRead(buttons_source[!i]);
    if (button_state != buttons_previous[!i]) {
      usbMIDI.sendControlChange(lookup + NUM_VIRTUAL, button_state * 127, MIDI_CHANNEL);
      buttons_previous[!i] = button_state;
    }

    // update button LEDs
    digitalWrite(buttons_leds[!i], min(1, value_virtual_button[lookup]));
  }
  
  // check if there is any incoming MIDI data
  if (usbMIDI.read()) {
    int type = usbMIDI.getType();
    int channel = usbMIDI.getChannel();
    if (channel == MIDI_CHANNEL) {
      if (type == 1) {
          int note = usbMIDI.getData1();
          int velocity = usbMIDI.getData2();
          /*if (note == 1 || note == 2) {
            digitalWrite(buttons_leds[note - 1], 1);
	  }*/
        } else if (type == 0) {
          int note = usbMIDI.getData1();
          int velocity = usbMIDI.getData2();
          /*if (note == 1 || note == 2) {
            digitalWrite(buttons_leds[note - 1], 0);
	  }*/
        } else if (type == 3) {
          int controller = usbMIDI.getData1();
          int value = usbMIDI.getData2();
          if (controller >= 0 && controller < NUM_VIRTUAL) {
            value_virtual_knobs[controller] = value;
	  } else if (controller >= NUM_VIRTUAL && controller < NUM_VIRTUAL * 2 ) {
	    value_virtual_button[controller - NUM_VIRTUAL] = value;
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
