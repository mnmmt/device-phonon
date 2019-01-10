//*** EC-11 rotary encoder ***//
// https://little-scale.blogspot.com.au/2016/07/simple-midi-rotary-encoder-with-teensy.html
// http://milkcrate.com.au/_other/downloads/arduino/Teensy_Rotary_Encoder.ino
// Thanks Seb!

// TODO:
// * pulses on pin 1 generate midi clock message (default)
// * incoming midi clock reconfigures pin 14 and sends pulse
// * incoming midi for knob values
// * incoming midi for LED
// * fix stuck buttons issue below

#define LED	13

#define MIDI_CHANNEL 1

// how many changes to collect
#define RING 32
// 5 milliseconds
#define RECENT 5

int frame = 0;
int last[4] = {0, 0, 0, 0};
int val[4] = {0, 0, 0, 0};
float weighted[4] = {0.0, 0.0, 0.0, 0.0};
int toggles[4] = {0, 0, 0, 0};

int ringbuffer_v[RING];
unsigned long ringbuffer_t[RING];
int ringpos = 0;

int pins[2] = {0, 0};
int buttons[4] = {8, 10, 21, 19};
int pulls[4] = {6, 12, 23, 16};
int selected = 0;
int clock_ring[3] = {0, 0, 0};
unsigned long clock_last = 0;
int clock_in = 1;
int clock_send = 0;
const uint8_t clock_sysex[3] = {3, 14, 59};
const uint8_t clock_sysex_2[2] = {99, 97};
int spp = 0;

void log() {
  Serial.print(micros());
  Serial.print(",");
  Serial.print(pins[0]);
  Serial.print(",");
  Serial.println(pins[1]);
}

void setup() {
  // init buffer
  for (int i=0; i<RING; i++) {
    ringbuffer_v[i] = 0;
    ringbuffer_t[i] = 0;
  }

  //pinMode(3, OUTPUT_OPENDRAIN);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);

  // button reference to low
  for (int i=0; i<4; i++) {
    pinMode(pulls[i], OUTPUT);
    digitalWrite(pulls[i], LOW);
    pinMode(buttons[i], INPUT_PULLUP);
  }
  
  pinMode(2, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  //pinMode(pushPin, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(2), ISRrotAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(4), ISRrotBChange, CHANGE);
  
  pinMode(26, INPUT_PULLDOWN);
  
  // Serial.begin(9600);
  // Serial.println("timestamp,pin1,pin2");
  
  // flash LED to indicate startup
  pinMode(LED, OUTPUT);
  for (int i=0; i<6; i++) {
    digitalWrite(LED, i % 2 ? HIGH : LOW);
    delay(250);
  }
}

void loop() {
  // update selected knob
  selected = (!digitalRead(buttons[0])) | ((!digitalRead(buttons[1])) << 1);
  
  weighted[selected] += (val[selected] - weighted[selected]) / 8.0;
  if (last[selected] != round(weighted[selected])) {
    last[selected] = round(weighted[selected]);
    usbMIDI.sendControlChange(selected * 3, val[selected], MIDI_CHANNEL);
  }
  
  // TODO: buttons can get stuck if "selected" and then pressed
  // and then "deselected" before release
  for (int i = 0; i < 4; i++) {
    if (toggles[i] != digitalRead(buttons[i])) {
      toggles[i] = digitalRead(buttons[i]);
      // virtual knob selectors
      if (i < 2) {
        usbMIDI.sendControlChange(i + 12, (!toggles[i]) * 127, MIDI_CHANNEL);
      } else {
        // regular buttons ganged to selected
        usbMIDI.sendControlChange(selected * 3 + i - 1, (!toggles[i]) * 127, MIDI_CHANNEL);
      }
    }
  }
  
  // check analogue in for clock signal yeh
  if (clock_in == 1) {
    clock_ring[0] = clock_ring[1];
    clock_ring[1] = clock_ring[2];
    clock_ring[2] = analogRead(26) > 256;
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

  if (usbMIDI.read()) {
    int type = usbMIDI.getType();
    //int channel = usbMIDI.getChannel();
    //if (channel == MIDI_CHANNEL) {
      // 8 = all realtime clock messages
      // 0xF8 = usbMIDI.Clock
      if (type == usbMIDI.SongPosition) {
        digitalWrite(LED, 1);
        if (clock_in) {
          clock_in = 0;
          pinMode(26, OUTPUT);
        }
        clock_send = 4;
      }
    //}
  } else {
    digitalWrite(LED, 0);
  }

  // check for clock sends
  if (clock_in == 0 && clock_send) {
    if (clock_send > 1) {
      digitalWrite(26, 1);
      //analogWrite(26, 255);
    } else {
      digitalWrite(26, 0);
      //analogWrite(26, 0);
    }
    clock_send -= 1;
  }

  delay(1);
  frame += 1;
}

int gray_decode(int n) {
    int p = n;
    while (n >>= 1) p ^= n;
    return p;
}

int previous(int lookup) {
  return (((lookup - 1) % RING) + RING) % RING;
}

int diff(int lookup) {
  int v1 = ringbuffer_v[lookup];
  int v2 = ringbuffer_v[previous(lookup)];
  for (int i = 0; i<4; i++) {
    if (v1 == i && v2 == ((i + 1) % 4)) {
      return 1;
    }
    if (v2 == i && v1 == ((i + 1) % 4)) {
      return -1;
    }
  }
  return 0;
}

int compute_change(int lookup) {
  unsigned long now = millis();
  int c = diff(lookup);
  int up = 0;
  int down = 0;
  for (int i=0; i<RING; i++) {
    int timediff = now - ringbuffer_t[((lookup + i) % RING)];
    // only consider recent changes
    if (timediff < RECENT && timediff != 0) {
      int o = diff(lookup);
      if (o == 1) {
        up++;
      }
      if (o == -1) {
      	down++;
      }
    }
  }
  if (up || down) {
    if (up > down) {
      return 1;
    }
    if (up < down) {
      return -1;
    }
  }
  return c;
}

int updateval(int *pins) {
  int lookup = ringpos % RING;
  ringbuffer_v[lookup] = gray_decode(pins[0] | (pins[1] << 1));;
  ringbuffer_t[lookup] = millis();
  int change = compute_change(lookup);
  ringpos++;
  return change;
}

void sendmidi() {
  val[selected] = constrain(val[selected] - updateval(pins), 0, 127);
}

// Interrupt routines
void ISRrotAChange() {
  // log();
  pins[0] = digitalRead(2);
  // log();
  sendmidi();
}

void ISRrotBChange() {
  // log();
  pins[1] = digitalRead(4);
  // log();
  sendmidi();
}

