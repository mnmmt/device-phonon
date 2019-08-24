//*** EC-11 rotary encoder ***//
// https://little-scale.blogspot.com.au/2016/07/simple-midi-rotary-encoder-with-teensy.html
// http://milkcrate.com.au/_other/downloads/arduino/Teensy_Rotary_Encoder.ino
// Thanks Seb!

// TODO: fix knob speed

// TODO: incoming midi sets virtual knob position
// TODO: send "select" button midi sysex messages
// TODO: incoming midi sets LED
// TODO: when no SPP seen switch back to analogue input mode

// https://github.com/brianlow/Rotary
#define HALF_STEP
#include "Rotary.h"

#define PIN_LED 13
#define PIN_SYNC 26

#define MIDI_CHANNEL 1

int frame = 0;
unsigned long last = 0;
int val[4] = {0, 0, 0, 0};
int btn[4] = {0, 0, 0, 0};
int toggles[4] = {0, 0, 0, 0};

int pins[2] = {0, 0};
int buttons[4] = {8, 10, 21, 19};
int pulls[5] = {6, 12, 23, 16, 17};
int selected = 0;
int clock_ring[3] = {0, 0, 0};
unsigned long clock_last = 0;
unsigned long check = 0;
int clock_in = 1;
int clock_send = 0;
const uint8_t clock_sysex[3] = {3, 14, 59};
const uint8_t clock_sysex_2[2] = {99, 97};
int spp = 0;
Rotary r = Rotary(4, 2);

void log() {
  Serial.print(micros());
  Serial.print(",");
  Serial.print(pins[0]);
  Serial.print(",");
  Serial.println(pins[1]);
}

void setup() {
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);

  // button reference to low
  for (int i=0; i<5; i++) {
    pinMode(pulls[i], OUTPUT);
    digitalWrite(pulls[i], LOW);
  }
  
  for (int i=0; i<4; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
  }
  
  r.begin();
  
  pinMode(PIN_SYNC, INPUT_PULLDOWN);
  
  // Serial.begin(9600);
  // Serial.println("timestamp,pin1,pin2");
  
  // flash LED to indicate startup
  pinMode(PIN_LED, OUTPUT);
  for (int i=0; i<6; i++) {
    digitalWrite(PIN_LED, i % 2 ? HIGH : LOW);
    delay(250);
  }
}

void loop() {
  // update selected knob
  selected = (!digitalRead(buttons[0])) | ((!digitalRead(buttons[1])) << 1);

  // check rotary encoder for changes
  unsigned char result = r.process();
  if (result) {
    // double speed accelerator
    int m = (last > millis() - 5) ? 2 : 1;
    last = millis();
    val[selected] = constrain(val[selected] + (result == DIR_CW ? 1 : -1) * m, 0, 127);
    usbMIDI.sendControlChange(selected, val[selected], MIDI_CHANNEL);
  }

  // poll physical hardware buttons
  for (int i = 0; i < 4; i++) {
    int v = digitalRead(buttons[i]);
    if (toggles[i] != v) {
      toggles[i] = v;
      // virtual knob selectors
      if (i < 2) {
        // TODO: send sysex message about "select" button mode
        for (int j = 0; j < 4; j++) {
          updatebtn(j, 0);
	}
      } else if (i == 2) {
        // regular buttons ganged to selected
        updatebtn(selected, (!toggles[2]) * 127);
      } else if (i == 3) {
        // independent trigger button
        usbMIDI.sendControlChange(8, (!toggles[i]) * 127, MIDI_CHANNEL);
      }
    }
  }

  // check analogue in for cv sync signal
  if (clock_in == 1) {
    clock_ring[0] = clock_ring[1];
    clock_ring[1] = clock_ring[2];
    clock_ring[2] = analogRead(PIN_SYNC) > 256;
    if (clock_ring[0] == 0 && clock_ring[1] == 1 && clock_ring[2] == 1) {
      if (millis() > clock_last + 25) {
      	// if incoming ticks have been off for a second
      	// reset the song position pointer
      	if (millis() > clock_last + 1250) {
      	  spp = 0;
      	}
        //usbMIDI.sendRealTime(0xF2);
        //usbMIDI.sendRealTime(0xF8);
        //usbMIDI.sendRealTime(0xFD);
        //usbMIDI.sendSysEx(3, clock_sysex);
        //usbMIDI.sendSysEx(1, clock_sysex_2);
        usbMIDI.sendSongPosition(spp);
        //usbMIDI.sendControlChange(127, spp % 127, MIDI_CHANNEL);
        spp += 1;
        //usbMIDI.send(0xf2, 1, 2, MIDI_CHANNEL)
      }
      clock_last = millis();
    }
  }

  // check for incoming midi messages
  if (usbMIDI.read()) {
    int type = usbMIDI.getType();
    //int channel = usbMIDI.getChannel();
    //if (channel == MIDI_CHANNEL) {
      // 8 = all realtime clock messages
      // 0xF8 = usbMIDI.Clock
      if (type == usbMIDI.SongPosition) {
        digitalWrite(PIN_LED, 1);
        if (clock_in) {
          clock_in = 0;
          pinMode(PIN_SYNC, OUTPUT);
        }
        clock_send = 4;
      }
    //}
  } else {
    digitalWrite(PIN_LED, 0);
  }

  // check for clock sends
  if (clock_in == 0 && clock_send) {
    if (clock_send > 1) {
      digitalWrite(PIN_SYNC, 1);
    } else {
      digitalWrite(PIN_SYNC, 0);
    }
    clock_send -= 1;
  }

  //delay(1);
  //frame += 1;
}

void updatebtn(int i, int newbtn) {
  if (btn[i] != newbtn) {
    usbMIDI.sendControlChange(i + 4, newbtn, MIDI_CHANNEL);
    btn[i] = newbtn;
  }
}
