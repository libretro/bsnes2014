gb_objects := gb-interface gb-system gb-scheduler
gb_objects += gb-memory gb-cartridge
gb_objects += gb-cpu gb-ppu gb-apu
gb_objects += gb-cheat gb-video
objects += $(gb_objects)

obj/gb-interface-$(profile).o: $(gb)/interface/interface.cpp $(call rwildcard,$(gb)/interface/)
obj/gb-system-$(profile).o:    $(gb)/system/system.cpp $(call rwildcard,$(gb)/system/)
obj/gb-scheduler-$(profile).o: $(gb)/scheduler/scheduler.cpp $(call rwildcard,$(gb)/scheduler/)
obj/gb-cartridge-$(profile).o: $(gb)/cartridge/cartridge.cpp $(call rwildcard,$(gb)/cartridge/)
obj/gb-memory-$(profile).o:    $(gb)/memory/memory.cpp $(call rwildcard,$(gb)/memory/)
obj/gb-cpu-$(profile).o:       $(gb)/cpu/cpu.cpp $(call rwildcard,$(gb)/cpu/)
obj/gb-ppu-$(profile).o:       $(gb)/ppu/ppu.cpp $(call rwildcard,$(gb)/ppu/)
obj/gb-apu-$(profile).o:       $(gb)/apu/apu.cpp $(call rwildcard,$(gb)/apu/)
obj/gb-cheat-$(profile).o:     $(gb)/cheat/cheat.cpp $(call rwildcard,$(gb)/cheat/)
obj/gb-video-$(profile).o:     $(gb)/video/video.cpp $(call rwildcard,$(gb)/video/)
