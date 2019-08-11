;(native-declare "#define HALF_STEP")
;(native-declare "#include \"Rotary/Rotary.h\"")
;(native-header "Rotary/Rotary.h")

(native-declare "#include <TimerOne.h>")

; Yoinked from original source:
; https://github.com/brianlow/Rotary

(native-declare
  "
  // Clockwise step.
  #define DIR_CW 0x10
  // Counter-clockwise step.
  #define DIR_CCW 0x20

  #define R_START 0x00

  #define R_CCW_BEGIN 0x1
  #define R_CW_BEGIN 0x2
  #define R_START_M 0x3
  #define R_CW_BEGIN_M 0x4
  #define R_CCW_BEGIN_M 0x5

  const unsigned char ttable[6][4] = {
  // R_START (00)
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  // R_CCW_BEGIN
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  // R_CW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  // R_START_M (11)
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  // R_CW_BEGIN_M
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  // R_CCW_BEGIN_M
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
  };")

(native-declare
  "
  volatile int encoder_pin1 = 0;
  volatile int encoder_pin2 = 0;

  volatile float encoder_pin1_smoothed = 0;
  volatile float encoder_pin2_smoothed = 0;

  volatile unsigned char encoder_state = 0;

  volatile int encoder_direction = 0;

  volatile int encoder_accumulator = 0;
  volatile int encoder_accumulator_cw = 0;
  volatile int encoder_accumulator_ccw = 0;

  volatile unsigned long encoder_last = 0;
  volatile unsigned char encoder_flag = 0;

  void update_encoder_pins() {
    //encoder_pin1_smoothed += (digitalRead(encoder_pin1) - encoder_pin1_smoothed) / 4;
    //encoder_pin2_smoothed += (digitalRead(encoder_pin2) - encoder_pin2_smoothed) / 4;
    //encoder_pin1_smoothed += (digitalRead(encoder_pin1) - encoder_pin1_smoothed) / 2;
    //encoder_pin2_smoothed += (digitalRead(encoder_pin2) - encoder_pin2_smoothed) / 2;
    encoder_pin1_smoothed = digitalRead(encoder_pin1);
    encoder_pin2_smoothed = digitalRead(encoder_pin2);

    if (encoder_flag) {
      if (micros() - encoder_last > 100) {
        encoder_flag = 0;
        encoder_accumulator += 1;

        // Serial.print((int)encoder_pin1_smoothed);
        // Serial.print(\" \");
        // Serial.print((int)encoder_pin2_smoothed);
        // Serial.println(\" <-- \");

        encoder_state = ttable[(unsigned char)encoder_state & 0xf][((int)(encoder_pin2_smoothed + 0.5) << 1) | (int)(encoder_pin1_smoothed + 0.5)];
        encoder_direction = encoder_state & 0x30;

        if (encoder_direction == DIR_CW) {
         encoder_accumulator_cw += 1;
        }
        if (encoder_direction == DIR_CCW) {
         encoder_accumulator_ccw += 1;
        }

      } else {
        // Serial.println(\"- -\");
      }
    }
  }
  ")

(defn get-interrupt-count []
  "if (encoder_accumulator) { __result = obj<number>(encoder_accumulator); encoder_accumulator = 0; }")

(defn get-count-cw []
  "if (encoder_accumulator_cw) { __result = obj<number>(encoder_accumulator_cw); encoder_accumulator_cw = 0; }")

(defn get-count-ccw []
  "if (encoder_accumulator_ccw) { __result = obj<number>(encoder_accumulator_ccw); encoder_accumulator_ccw = 0; }")

(defn interrupt []
  (cxx "// Serial.println(micros() - encoder_last);
        // Serial.print(digitalRead(encoder_pin1));
        // Serial.print(\" \");
        // Serial.println(digitalRead(encoder_pin2));
        encoder_last = micros();
        encoder_flag = 1;")
  ; (cxx "encoder_accumulator += 1;")
  ; (cxx "update_encoder_pins();")
  )

(defn setup [^number_t pin1 ^number_t pin2]
  "
  encoder_pin1 = pin1;
  encoder_pin2 = pin2;

  Timer1.initialize(50);
  Timer1.attachInterrupt(update_encoder_pins);
  ")

;*** unusued old experiments ***;

(defn update-smooth-pins []
  "encoder_pin1_smoothed += (digitalRead(pin1) - encoder_pin1_smoothed) / 4;
   encoder_pin2_smoothed += (digitalRead(pin2) - encoder_pin2_smoothed) / 4;")

(defn update-state [^number_t state ^number_t pin1 ^number_t pin2]
  "unsigned char pinstate = ((int)(encoder_pin2_smoothed + 0.5) << 1) | (int)(encoder_pin1_smoothed + 0.5);
   __result = obj<number>(ttable[(unsigned char)state & 0xf][pinstate]);")

(defn get-direction [^number_t state]
  "unsigned char d = (unsigned char)state & 0x30;
  if (d > 0) {
    __result = obj<number>(d == DIR_CW ? 1 : -1);
  }")

(defn get-encoder-direction []
  "
  unsigned char s;
  noInterrupts();
  s = encoder_state;
  interrupts();
  s = s & 0x30;
  if (s) {
    __result = obj<number>(s & 0x30);
  }
  ")

