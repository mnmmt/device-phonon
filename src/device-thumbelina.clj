(require '[ferret.arduino :as gpio]
         'rotary
         'usbmidi)

;***** Constants *****;

(def midi-channel 1)

; pins
(def pin-led 13)
(def pins-start-low (list 3 6 12 23 16 17))
(def pins-buttons (list 8 10 21 19))
(def pins-encoder (list 4 2))
(def pin-cv-sync 26)

;***** Atoms *****;

(def selected (atom 0))
(def encoder-state-polled (atom 0))
(def direction (atom 0))
(def slider (atom {:last 0 :p 0 :v 0}))
(def frame (atom 0))

;***** Functions *****;

(defn toggle-pin [pin]
  (->> (gpio/digital-read pin)
       (bit-xor 1)
       (gpio/digital-write pin)))

;***** Setup *****;

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

; attach interrupt to encoder pins
(doseq [p pins-encoder]
  ; (gpio/pin-mode p :input_pullup)
  (gpio/attach-interrupt p :change rotary/interrupt))

(rotary/setup (first pins-encoder) (second pins-encoder))

;***** Main *****;

(defn send-slider [controller value]
  (usbmidi/sendControlChange controller value midi-channel))

(forever
  ; check rotary encoder for changes
  (let [cw (rotary/get-count-cw)
        ccw (rotary/get-count-ccw)]
    (when (or cw ccw)
      (let [d (-> (rotary/get-interrupt-count)
                  ; (min 127)
                  (* (if cw 1 -1))
                  (/ 2.0))
            v (-> (@slider :v) (+ d) (min 127) (max 0))]
        (swap! slider assoc :v v))))

  ; send slider changes through via midi
  (let [now (millis)
        l (@slider :last)
        p (@slider :p)
        v (@slider :v)]
    (when (and
            (> (- now l) 10)
            (not= p v))
      (send-slider 3 v)
      (swap! slider assoc :p v :last now)))

  ; check buttons
  (let [check-selected (- 3 (bit-or (gpio/digital-read (nth pins-buttons 0))
                               (bit-shift-left
                                 (gpio/digital-read (nth pins-buttons 1)) 1)))]
    (when (not= @selected check-selected)
      (println "Select button:" check-selected "\n")
      ; (usbmidi/sendControlChange 2 check-selected midi-channel)
      (reset! selected check-selected))))
