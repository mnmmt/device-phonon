(require '[ferret.arduino :as gpio]
         'usbmidi)

(defmacro attach-interrupt [pin mode callback]
  (let [mode    (-> mode name .toUpperCase)
        isr-fn  (gensym)
        isr-pin (gensym)]
    `(do
       (def ~isr-fn  ~callback)
       (def ~isr-pin ~pin)
       (cxx
        ~(str "::pinMode(number::to<int>(" isr-pin ") , INPUT_PULLUP);\n"
              "auto isr_pin = digitalPinToInterrupt(number::to<int>(" isr-pin "));\n"
              "::attachInterrupt(isr_pin, [](){ run(" isr-fn ");}, " mode ");")))))


(defn pin-pull-up [^number_t pin mode]
  "::pinMode(pin, INPUT_PULLUP);")

;***** Constants *****;

(def midi-channel 1)

; pins
(def pin-led 13)
(def pins-start-low (list 3 6 12 23 16 17))
(def pins-buttons (list 8 10 21 19))
(def pin-cv-sync 26)

;***** Atoms *****;

(def selected (atom 0))

;***** Functions *****;

(defn toggle-pin [pin]
  (->> (gpio/digital-read pin)
       (bit-xor 1)
       (gpio/digital-write pin)))

;***** Setup *****;

(sleep 1000)

; pull pins low
(doseq [p pins-start-low]
  (gpio/pin-mode p :output)
  (gpio/digital-write p 0))

; pull buttons up
(doseq [p pins-buttons]
  (pin-pull-up p))

; kick off rotary encoder

; pull down cv-sync -> midi clock pin
(gpio/pin-mode pin-cv-sync :input)

; set LED pin up for output
(gpio/pin-mode pin-led :output)

; flash LED to indicate successful start
(doseq [t (range 7)]
  (gpio/digital-write pin-led (mod (inc t) 2))
  (sleep 125))

;***** Main *****;

(defn rotary-interrupt []
  (println "Interrupt! pins:" (list (gpio/digital-read 4) (gpio/digital-read 2))))

; TODO: (gpio/attach-interrupt 4 :change rotary-interrupt)
; [ once fix is in ferret ]
(attach-interrupt 2 :change rotary-interrupt)
(attach-interrupt 4 :change rotary-interrupt)

(forever
  (let [check-selected (bit-or (gpio/digital-read (nth pins-buttons 0))
                               (bit-shift-left
                                 (gpio/digital-read (nth pins-buttons 1)) 1))]
    (when (not= @selected check-selected)
      (println "Bang!" "\n")
      (usbmidi/sendControlChange 2 check-selected midi-channel)
      (reset! selected check-selected))))

