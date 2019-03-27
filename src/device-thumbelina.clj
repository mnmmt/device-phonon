(require '[ferret.arduino :as gpio]
         'usbmidi)

;***** Constants *****;

(def led 13)
(def midi-channel 1)
(def start-low (list 3 6 12 23 16 17))
(def buttons (list 8 10 21 19))

;***** Atoms *****;

(def selected (atom 0))

;***** Functions *****;

(defn toggle-pin [pin]
  (->> (gpio/digital-read pin)
       (bit-xor 1)
       (gpio/digital-write pin)))

;***** Setup *****;

(gpio/pin-mode led :output)

;(map (fn [p] (gpio/pin-mode p :output)) start-low)

(doseq [p start-low]
  (do
    ;(gpio/pin-mode p :output)
    (gpio/digital-write p :low)))

;(doseq [p buttons]
;  (gpio/pin-mode p :input_pullup))

;***** Main *****;

(forever
  (toggle-pin 13)
  (sleep 1000)
  (usbmidi/sendControlChange 2 (swap! selected (fn [i] (mod (inc i) 127))) midi-channel))

