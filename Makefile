name=$(notdir $(shell pwd))
aflags=--board teensy:avr:teensyLC --pref build.extra_flags="-D USB_MIDI" --pref custom_usb="teensyLC_midi"

build/$(name).ino: src/$(name).clj src/*.clj
	mkdir -p $(name)
	ferret -i $< -o $@

verify: build/$(name).ino
	arduino $(aflags) --verify build/*.ino

upload: build/$(name).ino
	arduino $(aflags) --upload build/*.ino

clean:
	rm -rf build/$(name).ino
