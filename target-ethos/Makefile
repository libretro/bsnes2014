name := higan

processors := arm gsu hg51b lr35902 r6502 r65816 spc700 upd96050
include processor/Makefile

include fc/Makefile
include sfc/Makefile
include gb/Makefile
include gba/Makefile

ui_objects := ui-ethos ui-configuration ui-interface ui-utility
ui_objects += ui-input ui-window ui-general ui-settings ui-tools
ui_objects += phoenix ruby
ui_objects += $(if $(call streq,$(platform),windows),resource)

# platform
ifeq ($(platform),windows)
  ruby := video.direct3d video.wgl video.directdraw video.gdi
  ruby += audio.directsound audio.xaudio2
  ruby += input.windows
else ifeq ($(platform),macosx)
  ruby := video.cgl
  ruby += audio.openal
  ruby += input.carbon
else ifeq ($(platform),linux)
  ruby := video.glx video.xv video.xshm video.sdl
  ruby += audio.alsa audio.openal audio.oss audio.pulseaudio audio.pulseaudiosimple audio.ao
  ruby += input.udev input.sdl input.x
else ifeq ($(platform),bsd)
  ruby := video.glx
  ruby += audio.openal audio.oss
  ruby += input.x
endif

# phoenix
include phoenix/Makefile
link += $(phoenixlink)

# ruby
include ruby/Makefile
link += $(rubylink)

# rules
objects := $(ui_objects) $(objects)
objects := $(patsubst %,obj/%-$(profile).o,$(objects))

obj/ui-ethos-$(profile).o: $(ui)/ethos.cpp $(call rwildcard,$(ui)/)
obj/ui-configuration-$(profile).o: $(ui)/configuration/configuration.cpp $(call rwildcard,$(ui)/)
obj/ui-interface-$(profile).o: $(ui)/interface/interface.cpp $(call rwildcard,$(ui)/)
obj/ui-utility-$(profile).o: $(ui)/utility/utility.cpp $(call rwildcard,$(ui)/)
obj/ui-input-$(profile).o: $(ui)/input/input.cpp $(call rwildcard,$(ui)/)
obj/ui-window-$(profile).o: $(ui)/window/window.cpp $(call rwildcard,$(ui)/)
obj/ui-general-$(profile).o: $(ui)/general/general.cpp $(call rwildcard,$(ui)/)
obj/ui-settings-$(profile).o: $(ui)/settings/settings.cpp $(call rwildcard,$(ui)/)
obj/ui-tools-$(profile).o: $(ui)/tools/tools.cpp $(call rwildcard,$(ui)/)

obj/ruby-$(profile).o: ruby/ruby.cpp $(call rwildcard,ruby/*)
	$(compiler) $(rubyflags) -c $< -o $@

obj/phoenix-$(profile).o: phoenix/phoenix.cpp $(call rwildcard,phoenix/*)
	$(compiler) $(phoenixflags) -c $< -o $@

obj/resource-$(profile).o: $(ui)/resource.rc
ifeq ($(arch),win32)
	windres --target=pe-i386 $(ui)/resource.rc obj/resource-$(profile).o
else
	windres $(ui)/resource.rc obj/resource-$(profile).o
endif

# targets
build: $(objects)
ifeq ($(platform),windows)
	$(strip $(compiler) -shared -o out/phoenix.dll obj/phoenix-$(profile).o $(phoenixlink))
	$(strip $(compiler) -o out/$(name) $(subst obj/phoenix-$(profile).o,,$(objects)) $(link) -Lout -lphoenix)
else ifeq ($(platform),macosx)
	if [ -d out/$(name).app ]; then rm -r out/$(name).app; fi
	mkdir out/$(name).app
	mkdir out/$(name).app/Contents
	mkdir out/$(name).app/Contents/MacOS
	mkdir out/$(name).app/Contents/Resources
	cp data/Info.plist out/$(name).app/Contents/Info.plist
	sips -s format icns data/higan.png --out out/$(name).app/Contents/Resources/higan.icns
	$(strip $(compiler) -o out/$(name).app/Contents/MacOS/$(name) $(objects) $(link))
else
	$(strip $(compiler) -o out/$(name) $(objects) $(link))
endif

resource:
	sourcery $(ui)/resource/resource.bml $(ui)/resource/resource.cpp $(ui)/resource/resource.hpp

install:
ifeq ($(platform),windows)
else ifeq ($(platform),macosx)
	sudo mkdir -p /Library/Application\ Support/$(name)
	sudo cp -R profile/* /Library/Application\ Support/$(name)
	sudo cp data/cheats.bml /Library/Application\ Support/$(name)/cheats.bml
	sudo chmod -R 777 /Library/Application\ Support/$(name)
else
	sudo install -D -m 755 out/$(name) $(DESTDIR)$(prefix)/bin/$(name)
	sudo install -D -m 644 data/$(name).png $(DESTDIR)$(prefix)/share/pixmaps/$(name).png
	sudo install -D -m 644 data/$(name).desktop $(DESTDIR)$(prefix)/share/applications/$(name).desktop

	sudo mkdir -p /usr/share/$(name)
	sudo cp -R profile/* /usr/share/$(name)
	sudo cp data/cheats.bml /usr/share/$(name)/cheats.bml
	sudo chmod -R 777 /usr/share/$(name)
endif

uninstall:
ifeq ($(platform),windows)
else ifeq ($(platform),macosx)
else
	sudo rm $(DESTDIR)$(prefix)/bin/$(name)
endif
