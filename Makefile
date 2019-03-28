name=$(notdir $(shell pwd))
noserial= --pref build.extra_flags="-D USB_MIDI" --pref custom_usb="teensyLC_midi"
serial=--pref build.extra_flags="-D USB_MIDI_SERIAL" --pref custom_usb="teensyLC_serialmidi"
aflags=--board teensy:avr:teensyLC $(serial)

build/$(name).ino: src/$(name).clj src/*.clj
	mkdir -p $(name)
	ferret -i $< -o $@

verify: build/$(name).ino
	arduino $(aflags) --verify build/*.ino

upload: build/$(name).ino
	arduino $(aflags) --upload build/*.ino

clean:
	rm -rf build/$(name).ino
