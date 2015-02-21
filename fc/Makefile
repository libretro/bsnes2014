fc_objects := fc-interface fc-system fc-scheduler fc-input
fc_objects += fc-memory fc-cartridge fc-cpu fc-apu fc-ppu
fc_objects += fc-cheat fc-video
objects += $(fc_objects)

obj/fc-interface-$(profile).o: $(fc)/interface/interface.cpp $(call rwildcard,$(fc)/interface/)
obj/fc-system-$(profile).o: $(fc)/system/system.cpp $(call rwildcard,$(fc)/system/)
obj/fc-scheduler-$(profile).o: $(fc)/scheduler/scheduler.cpp $(call rwildcard,$(fc)/scheduler/)
obj/fc-input-$(profile).o: $(fc)/input/input.cpp $(call rwildcard,$(fc)/input/)
obj/fc-memory-$(profile).o: $(fc)/memory/memory.cpp $(call rwildcard,$(fc)/memory/)
obj/fc-cartridge-$(profile).o: $(fc)/cartridge/cartridge.cpp $(call rwildcard,$(fc)/cartridge/)
obj/fc-cpu-$(profile).o: $(fc)/cpu/cpu.cpp $(call rwildcard,$(fc)/cpu/)
obj/fc-apu-$(profile).o: $(fc)/apu/apu.cpp $(call rwildcard,$(fc)/apu/)
obj/fc-ppu-$(profile).o: $(fc)/ppu/ppu.cpp $(call rwildcard,$(fc)/ppu/)
obj/fc-cheat-$(profile).o: $(fc)/cheat/cheat.cpp $(call rwildcard,$(fc)/cheat/)
obj/fc-video-$(profile).o: $(fc)/video/video.cpp $(call rwildcard,$(fc)/video/)
