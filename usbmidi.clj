(native-header "MIDIUSB.h")

(defnative sendControlChange [controller value channel]
  (on true
  ("MIDIUSB.h")
  "usbMIDI.sendControlChange(number::to<number_t>(controller), number::to<number_t>(value), number::to<number_t>(channel));"))

(defnative sendSongPosition [spp]
  "usbMIDI.sendSongPosition(number::to<number_t>(spp));")
