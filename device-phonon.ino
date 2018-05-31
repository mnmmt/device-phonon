//*** EC-11 rotary encoder ***//
// https://little-scale.blogspot.com.au/2016/07/simple-midi-rotary-encoder-with-teensy.html
// http://milkcrate.com.au/_other/downloads/arduino/Teensy_Rotary_Encoder.ino
// Thanks Seb!

#include <Encoder.h>

#define LED	13

#define MIDI_CHANNEL 1

// how many changes to collect
#define RING 32
// 5 milliseconds
#define RECENT 5

int frame = 0;
int last = 0;
int val = 0;
float weighted = 0.0;
int ringbuffer_v[RING];
unsigned long ringbuffer_t[RING];
int ringpos = 0;
int pins[] = {0, 0};
unsigned long lastInterrupt = 0;

// Encoder knob = Encoder(2,4);

void setup() {
  // init buffer
  for (int i=0; i<RING; i++) {
    ringbuffer_v[i] = 0;
    ringbuffer_t[i] = 0;
  }

  //pinMode(3, OUTPUT_OPENDRAIN);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
  
  // flash LED to indicate startup
  pinMode(LED, OUTPUT);
  for (int i=0; i<6; i++) {
    digitalWrite(LED, i % 2 ? HIGH : LOW);
    delay(250);
  }
  
  pinMode(2, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  //pinMode(pushPin, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(2), ISRrotAChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(4), ISRrotBChange, CHANGE);
  
  lastInterrupt = millis();
  //knob.write(64);
  
  //Serial.begin(9600);
  //Serial.println("Booted.");
}

void loop() {
  weighted += (val - weighted) / 8.0;
  if (last != round(weighted)) {
    last = round(weighted);
    usbMIDI.sendControlChange(1, val, MIDI_CHANNEL);
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

// lol this is awful
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
  val = constrain(val - updateval(pins), 0, 127);
  // usbMIDI.sendControlChange(1, val, MIDI_CHANNEL);
}

// Interrupt routines
void ISRrotAChange() {
  pins[0] = digitalRead(2);
  sendmidi();
}

void ISRrotBChange() {
  pins[1] = digitalRead(4);
  sendmidi();
}

