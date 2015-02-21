gba_objects := gba-memory gba-interface gba-scheduler gba-system
gba_objects += gba-video gba-cartridge gba-player
gba_objects += gba-cpu gba-ppu gba-apu
objects += $(gba_objects)

obj/gba-memory-$(profile).o:    $(gba)/memory/memory.cpp $(call rwildcard,$(gba)/memory)
obj/gba-interface-$(profile).o: $(gba)/interface/interface.cpp $(call rwildcard,$(gba)/interface)
obj/gba-scheduler-$(profile).o: $(gba)/scheduler/scheduler.cpp $(call rwildcard,$(gba)/scheduler)
obj/gba-system-$(profile).o:    $(gba)/system/system.cpp $(call rwildcard,$(gba)/system)
obj/gba-video-$(profile).o:     $(gba)/video/video.cpp $(call rwildcard,$(gba)/video)
obj/gba-cartridge-$(profile).o: $(gba)/cartridge/cartridge.cpp $(call rwildcard,$(gba)/cartridge)
obj/gba-player-$(profile).o:    $(gba)/player/player.cpp $(call rwildcard,$(gba)/player)
obj/gba-cpu-$(profile).o:       $(gba)/cpu/cpu.cpp $(call rwildcard,$(gba)/cpu)
obj/gba-ppu-$(profile).o:       $(gba)/ppu/ppu.cpp $(call rwildcard,$(gba)/ppu)
obj/gba-apu-$(profile).o:       $(gba)/apu/apu.cpp $(call rwildcard,$(gba)/apu)
