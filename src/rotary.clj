;(native-declare "#define HALF_STEP")
;(native-declare "#include \"Rotary/Rotary.h\"")
;(native-header "Rotary/Rotary.h")

; Yoinked from original source:
; https://github.com/brianlow/Rotary

(defnative encoder [pin1 pin2]
  (on true
      "//Rotary r = Rotary(number::to<int>(pin1), number::to<int>(pin2));
      //r.begin(true);
      //__result = obj<value<Rotary>>(number::to<char>(pin1), number::to<char>(pin2));

      //PCICR |= (1 << PCIE2);
      //PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
      //sei();

      // value<Rotary>::to_value(__result).begin(true);
      //__result = obj<value<Rotary>>(r);
      __result = obj<value<Rotary>>((char)4, (char)2);
      value<Rotary>::to_value(__result).begin(true);
      "))

(defnative process [enc]
  (on true
      "return obj<number>((char)(value<Rotary>::to_value(enc)).process());"))

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
  };"
  )

(defn update-state [^number_t state ^number_t pin1 ^number_t pin2]
  "
  unsigned char pinstate = (digitalRead(pin2) << 1) | digitalRead(pin1);
  __result = obj<number>(ttable[(unsigned char)state & 0xf][pinstate]);")

(defn get-direction [^number_t state]
  "__result = obj<number>(((unsigned char)state) & 0x30)")
