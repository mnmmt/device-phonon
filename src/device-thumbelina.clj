(require '[ferret.arduino :as gpio]
         'rotary
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

(sleep 1000)

; pull pins low
(doseq [p pins-start-low]
  (gpio/pin-mode p :output)
  (gpio/digital-write p 0))

; pull buttons up
(doseq [p pins-buttons]
  (gpio/pin-mode p :input_pullup))

; pull down cv-sync -> midi clock pin
(gpio/pin-mode pin-cv-sync :input)

; set LED pin up for output
(gpio/pin-mode pin-led :output)

; flash LED to indicate successful start
(doseq [t (range 7)]
  (gpio/digital-write pin-led (mod (inc t) 2))
  (sleep 125))

;***** Main *****;

(def encoder-state (atom 0))

(defn rotary-interrupt []
  ; (println "Interrupt! pins:" (list (gpio/digital-read 4) (gpio/digital-read 2)))
  (swap! encoder-state rotary/update-state 4 2)
  (let [change (rotary/get-direction @encoder-state)]
    (when (not= change 0)
      (println change))))

(gpio/attach-interrupt 2 :change rotary-interrupt)
(gpio/attach-interrupt 4 :change rotary-interrupt)

(forever
  (let [check-selected (bit-or (gpio/digital-read (nth pins-buttons 0))
                               (bit-shift-left
                                 (gpio/digital-read (nth pins-buttons 1)) 1))]
    (when (not= @selected check-selected)
      (println "Bang!" check-selected "\n")
      (usbmidi/sendControlChange 2 check-selected midi-channel)
      (reset! selected check-selected))))

