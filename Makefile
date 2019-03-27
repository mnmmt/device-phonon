name=$(notdir $(shell pwd))
aflags=--board teensy:avr:teensyLC --pref build.extra_flags="-D USB_MIDI" --pref custom_usb="teensyLC_midi"

$(name).ino: $(name).clj *.clj
	mkdir -p $(name)
	ferret -i $< -o $@

verify: $(name).ino
	arduino $(aflags) --verify *.ino

upload: $(name).ino
	arduino $(aflags) --upload *.ino

clean:
	rm -rf $(name).ino
