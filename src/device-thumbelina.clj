(require '[ferret.arduino :as gpio]
         'usbmidi)

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

; pull pins low
(doseq [p pins-start-low]
  (gpio/pin-mode p :output)
  (gpio/digital-write p :low))

; pull buttons up
(doseq [p pins-buttons]
  (gpio/pin-mode p :input_pullup))

; kick off rotary encoder

; pull down cv-sync -> midi clock pin
(gpio/pin-mode pin-cv-sync :input_pulldown)

; set LED pin up for output
(gpio/pin-mode pin-led :output)

; flash LED to indicate successful start
(doseq [t (range 7)]
  (gpio/digital-write pin-led (mod (inc t) 2))
  (sleep 125))

;***** Main *****;

(forever
  (toggle-pin 13)
  (sleep 1000)
  (usbmidi/sendControlChange 2 (swap! selected (fn [i] (mod (inc i) 127))) midi-channel))

