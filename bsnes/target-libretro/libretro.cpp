#include "libretro.h"
#include <snes/snes.hpp>

#include <nall/snes/cartridge.hpp>
#include <nall/gb/cartridge.hpp>
using namespace nall;

const uint8 iplrom[64] = {
/*ffc0*/  0xcd, 0xef,        //mov   x,#$ef
/*ffc2*/  0xbd,              //mov   sp,x
/*ffc3*/  0xe8, 0x00,        //mov   a,#$00
/*ffc5*/  0xc6,              //mov   (x),a
/*ffc6*/  0x1d,              //dec   x
/*ffc7*/  0xd0, 0xfc,        //bne   $ffc5
/*ffc9*/  0x8f, 0xaa, 0xf4,  //mov   $f4,#$aa
/*ffcc*/  0x8f, 0xbb, 0xf5,  //mov   $f5,#$bb
/*ffcf*/  0x78, 0xcc, 0xf4,  //cmp   $f4,#$cc
/*ffd2*/  0xd0, 0xfb,        //bne   $ffcf
/*ffd4*/  0x2f, 0x19,        //bra   $ffef
/*ffd6*/  0xeb, 0xf4,        //mov   y,$f4
/*ffd8*/  0xd0, 0xfc,        //bne   $ffd6
/*ffda*/  0x7e, 0xf4,        //cmp   y,$f4
/*ffdc*/  0xd0, 0x0b,        //bne   $ffe9
/*ffde*/  0xe4, 0xf5,        //mov   a,$f5
/*ffe0*/  0xcb, 0xf4,        //mov   $f4,y
/*ffe2*/  0xd7, 0x00,        //mov   ($00)+y,a
/*ffe4*/  0xfc,              //inc   y
/*ffe5*/  0xd0, 0xf3,        //bne   $ffda
/*ffe7*/  0xab, 0x01,        //inc   $01
/*ffe9*/  0x10, 0xef,        //bpl   $ffda
/*ffeb*/  0x7e, 0xf4,        //cmp   y,$f4
/*ffed*/  0x10, 0xeb,        //bpl   $ffda
/*ffef*/  0xba, 0xf6,        //movw  ya,$f6
/*fff1*/  0xda, 0x00,        //movw  $00,ya
/*fff3*/  0xba, 0xf4,        //movw  ya,$f4
/*fff5*/  0xc4, 0xf4,        //mov   $f4,a
/*fff7*/  0xdd,              //mov   a,y
/*fff8*/  0x5d,              //mov   x,a
/*fff9*/  0xd0, 0xdb,        //bne   $ffd6
/*fffb*/  0x1f, 0x00, 0x00,  //jmp   ($0000+x)
/*fffe*/  0xc0, 0xff         //reset vector location ($ffc0)
};

struct Interface : public SNES::Interface {
  retro_video_refresh_t pvideo_refresh;
  retro_audio_sample_t paudio_sample;
  retro_input_poll_t pinput_poll;
  retro_input_state_t pinput_state;
  retro_environment_t penviron;
  bool overscan;

  string basename;
  uint16_t *buffer;
  SNES::Video video;

  static unsigned snes_to_retro(SNES::Input::Device device) {
    switch (device) {
       default:
       case SNES::Input::Device::None:       return RETRO_DEVICE_NONE;
       case SNES::Input::Device::Joypad:     return RETRO_DEVICE_JOYPAD;
       case SNES::Input::Device::Multitap:   return RETRO_DEVICE_JOYPAD_MULTITAP;
       case SNES::Input::Device::Mouse:      return RETRO_DEVICE_MOUSE;
       case SNES::Input::Device::SuperScope: return RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE;
       case SNES::Input::Device::Justifier:  return RETRO_DEVICE_LIGHTGUN_JUSTIFIER;
       case SNES::Input::Device::Justifiers: return RETRO_DEVICE_LIGHTGUN_JUSTIFIERS;
    }
  }

  // TODO: Properly map Mouse/Lightguns.
  static unsigned snes_to_retro(SNES::Input::Device, unsigned id) {
    return id;
  }

  static SNES::Input::Device retro_to_snes(unsigned device) {
    switch (device) {
       default:
       case RETRO_DEVICE_NONE:                 return SNES::Input::Device::None;
       case RETRO_DEVICE_JOYPAD:               return SNES::Input::Device::Joypad;
       case RETRO_DEVICE_JOYPAD_MULTITAP:      return SNES::Input::Device::Multitap;
       case RETRO_DEVICE_MOUSE:                return SNES::Input::Device::Mouse;
       case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE: return SNES::Input::Device::SuperScope;
       case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:   return SNES::Input::Device::Justifier;
       case RETRO_DEVICE_LIGHTGUN_JUSTIFIERS:  return SNES::Input::Device::Justifiers;
    }
  }

  void videoRefresh(const uint32_t *data, bool hires, bool interlace, bool overscan) {
    unsigned width = hires ? 512 : 256;
    unsigned height = overscan ? 239 : 224;
    unsigned pitch = 1024 >> interlace;
    if(interlace) height <<= 1;
    data += 9 * 1024;  //skip front porch

    for(unsigned y = 0; y < height; y++) {
      const uint32_t *sp = data + y * pitch;
      uint16_t *dp = buffer + y * pitch;
      for(unsigned x = 0; x < width; x++) {
        *dp++ = video.palette[*sp++];
      }
    }

    pvideo_refresh(buffer, width, height, pitch << 1);
    pinput_poll();
  }

  void audioSample(int16_t left, int16_t right) {
    if(paudio_sample) return paudio_sample(left, right);
  }

  int16_t inputPoll(bool port, SNES::Input::Device device, unsigned index, unsigned id) {
    if(id > 11) return 0;

    if (device == SNES::Input::Device::Multitap && port)
      return pinput_state(index + 1, RETRO_DEVICE_JOYPAD, 0, id);
    else
      return pinput_state(port, snes_to_retro(device), index, snes_to_retro(device, id));
  }

  void message(const string &text) {
    fprintf(stderr, "%s", (const char*)text);
  }

  string path(SNES::Cartridge::Slot slot, const string &hint) {
    return string(basename, hint);
  }

  Interface() : pvideo_refresh(0), paudio_sample(0), pinput_poll(0), pinput_state(0) {
    buffer = new uint16_t[512 * 480];
    video.generate(SNES::Video::Format::RGB15);
  }

  void setCheats(const lstring &list = lstring()) {
    if(SNES::cartridge.mode() == SNES::Cartridge::Mode::SuperGameBoy) {
      GB::cheat.reset();
      for(auto &code : list) {
        lstring codelist;
        codelist.split("+", code);
        for(auto &part : codelist) {
          unsigned addr, data, comp;
          if(GB::Cheat::decode(part, addr, data, comp)) {
            GB::cheat.append({addr, data, comp});
          }
        }
      }
      GB::cheat.synchronize();
      return;
    }

    SNES::cheat.reset();
    for(auto &code : list) {
      lstring codelist;
      codelist.split("+", code);
      for(auto &part : codelist) {
        unsigned addr, data;
        if(SNES::Cheat::decode(part, addr, data)) {
          SNES::cheat.append({addr, data});
        }
      }
    }
    SNES::cheat.synchronize();
  }

  ~Interface() {
    delete[] buffer;
  }
};

static Interface interface;

unsigned retro_api_version(void) {
  return RETRO_API_VERSION;
}

void retro_set_environment(retro_environment_t environ_cb)        { interface.penviron       = environ_cb; }
void retro_set_video_refresh(retro_video_refresh_t video_refresh) { interface.pvideo_refresh = video_refresh; }
void retro_set_audio_sample(retro_audio_sample_t audio_sample)    { interface.paudio_sample  = audio_sample; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t)     {}
void retro_set_input_poll(retro_input_poll_t input_poll)          { interface.pinput_poll    = input_poll; }
void retro_set_input_state(retro_input_state_t input_state)       { interface.pinput_state   = input_state; }

void retro_set_controller_port_device(unsigned port, unsigned device) {
  if (port < 2)
    SNES::input.connect(port, Interface::retro_to_snes(device));
}

void retro_init(void) {
  SNES::interface = &interface;
  memcpy(SNES::smp.iplrom, iplrom, 64);
  SNES::system.init();
  SNES::input.connect(SNES::Controller::Port1, SNES::Input::Device::Joypad);
  SNES::input.connect(SNES::Controller::Port2, SNES::Input::Device::Joypad);
}

void retro_deinit(void) {
  SNES::system.term();
}

void retro_reset(void) {
  SNES::system.reset();
}

void retro_run(void) {
  SNES::system.run();
}

size_t retro_serialize_size(void) {
  return SNES::system.serialize_size();
}

bool retro_serialize(void *data, size_t size) {
  SNES::system.runtosave();
  serializer s = SNES::system.serialize();
  if(s.size() > size) return false;
  memcpy(data, s.data(), s.size());
  return true;
}

bool retro_unserialize(const void *data, size_t size) {
  serializer s((const uint8_t*)data, size);
  return SNES::system.unserialize(s);
}

struct CheatList {
  bool enable;
  string code;
  CheatList() : enable(false) {}
};

static linear_vector<CheatList> cheatList;

void retro_cheat_reset(void) {
  cheatList.reset();
  interface.setCheats();
}

void retro_cheat_set(unsigned index, bool enable, const char *code) {
  cheatList[index].enable = enable;
  cheatList[index].code = code;
  lstring list;

  for(unsigned n = 0; n < cheatList.size(); n++) {
    if(cheatList[n].enable) list.append(cheatList[n].code);
  }

  interface.setCheats(list);
}

void retro_get_system_info(struct retro_system_info *info) {
  static string version("v", Version, " (", SNES::Info::Profile, ")");
  info->library_name     = "bSNES";
  info->library_version  = version;
  info->valid_extensions = 0;
  info->need_fullpath    = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info) {
  struct retro_system_timing timing = { 0.0, 32040.5 };
  timing.fps = retro_get_region() == RETRO_REGION_NTSC ? 21477272.0 / 357366.0 : 21281370.0 / 425568.0;

  if (!interface.penviron(RETRO_ENVIRONMENT_GET_OVERSCAN, &interface.overscan))
     interface.overscan = false;

  unsigned base_width = 256;
  unsigned base_height = interface.overscan ? 239 : 224;
  struct retro_game_geometry geom = { base_width, base_height, base_width << 1, base_height << 1 };

  info->timing   = timing;
  info->geometry = geom;
}

static bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
) {
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::Normal, xmlrom);
  SNES::system.power();
  return true;
}

static bool snes_load_cartridge_bsx_slotted(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) {
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  if(bsx_data) SNES::bsxflash.memory.copy(bsx_data, bsx_size);
  string xmlbsx = (bsx_xml && *bsx_xml) ? string(bsx_xml) : SuperFamicomCartridge(bsx_data, bsx_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::BsxSlotted, xmlrom);
  SNES::system.power();
  return true;
}

static bool snes_load_cartridge_bsx(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) {
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  if(bsx_data) SNES::bsxflash.memory.copy(bsx_data, bsx_size);
  string xmlbsx = (bsx_xml && *bsx_xml) ? string(bsx_xml) : SuperFamicomCartridge(bsx_data, bsx_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::Bsx, xmlrom);
  SNES::system.power();
  return true;
}

static bool snes_load_cartridge_sufami_turbo(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *sta_xml, const uint8_t *sta_data, unsigned sta_size,
  const char *stb_xml, const uint8_t *stb_data, unsigned stb_size
) {
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  if(sta_data) SNES::sufamiturbo.slotA.rom.copy(sta_data, sta_size);
  string xmlsta = (sta_xml && *sta_xml) ? string(sta_xml) : SuperFamicomCartridge(sta_data, sta_size).markup;
  if(stb_data) SNES::sufamiturbo.slotB.rom.copy(stb_data, stb_size);
  string xmlstb = (stb_xml && *stb_xml) ? string(stb_xml) : SuperFamicomCartridge(stb_data, stb_size).markup;
  SNES::cartridge.load(SNES::Cartridge::Mode::SufamiTurbo, xmlrom);
  SNES::system.power();
  return true;
}

static bool snes_load_cartridge_super_game_boy(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *dmg_xml, const uint8_t *dmg_data, unsigned dmg_size
) {
  if(rom_data) SNES::cartridge.rom.copy(rom_data, rom_size);
  string xmlrom = (rom_xml && *rom_xml) ? string(rom_xml) : SuperFamicomCartridge(rom_data, rom_size).markup;
  if(dmg_data) {
    //GameBoyCartridge needs to modify dmg_data (for MMM01 emulation); so copy data
    uint8_t *data = new uint8_t[dmg_size];
    memcpy(data, dmg_data, dmg_size);
    string xmldmg = (dmg_xml && *dmg_xml) ? string(dmg_xml) : GameBoyCartridge(data, dmg_size).markup;
    GB::cartridge.load(GB::System::Revision::SuperGameBoy, xmldmg, data, dmg_size);
    delete[] data;
  }
  SNES::cartridge.load(SNES::Cartridge::Mode::SuperGameBoy, xmlrom);
  SNES::system.power();
  return true;
}

bool retro_load_game(const struct retro_game_info *info) {
  retro_cheat_reset();
  if (info->path) {
    interface.basename = info->path;
    char *dot = strrchr(interface.basename(), '.');
    if (dot)
       *dot = '\0';
  }

  return snes_load_cartridge_normal(info->meta, (const uint8_t*)info->data, info->size);
}

bool retro_load_game_special(unsigned game_type,
      const struct retro_game_info *info, size_t num_info) {

  retro_cheat_reset();
  if (info[0].path) {
    interface.basename = info[0].path;
    char *dot = strrchr(interface.basename(), '.');
    if (dot)
       *dot = '\0';
  }

  switch (game_type) {
     case RETRO_GAME_TYPE_BSX:
       return num_info == 2 && snes_load_cartridge_bsx(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size);
       
     case RETRO_GAME_TYPE_BSX_SLOTTED:
       return num_info == 2 && snes_load_cartridge_bsx_slotted(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size);

     case RETRO_GAME_TYPE_SUPER_GAME_BOY:
       return num_info == 2 && snes_load_cartridge_super_game_boy(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size);

     case RETRO_GAME_TYPE_SUFAMI_TURBO:
       return num_info == 3 && snes_load_cartridge_sufami_turbo(info[0].meta, (const uint8_t*)info[0].data, info[0].size,
             info[1].meta, (const uint8_t*)info[1].data, info[1].size,
             info[2].meta, (const uint8_t*)info[2].data, info[2].size);

     default:
       return false;
  }
}

void retro_unload_game(void) {
  SNES::cartridge.unload();
}

unsigned retro_get_region(void) {
  return SNES::system.region() == SNES::System::Region::NTSC ? 0 : 1;
}

void* retro_get_memory_data(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;

  switch(id) {
    case RETRO_MEMORY_SAVE_RAM:
      return SNES::cartridge.ram.data();
    case RETRO_MEMORY_RTC:
      if(SNES::cartridge.has_srtc()) return SNES::srtc.rtc;
      if(SNES::cartridge.has_spc7110rtc()) return SNES::spc7110.rtc;
      return 0;
    case RETRO_MEMORY_SNES_BSX_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::Bsx) break;
      return SNES::bsxcartridge.sram.data();
    case RETRO_MEMORY_SNES_BSX_PRAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::Bsx) break;
      return SNES::bsxcartridge.psram.data();
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::SufamiTurbo) break;
      return SNES::sufamiturbo.slotA.ram.data();
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::SufamiTurbo) break;
      return SNES::sufamiturbo.slotB.ram.data();
    case RETRO_MEMORY_SNES_GAME_BOY_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::SuperGameBoy) break;
      return GB::cartridge.ramdata;

    case RETRO_MEMORY_SYSTEM_RAM:
      return SNES::cpu.wram;
    case RETRO_MEMORY_VIDEO_RAM:
      return SNES::ppu.vram;
  }

  return 0;
}

size_t retro_get_memory_size(unsigned id) {
  if(SNES::cartridge.loaded() == false) return 0;
  size_t size = 0;

  switch(id) {
    case RETRO_MEMORY_SAVE_RAM:
      size = SNES::cartridge.ram.size();
      break;
    case RETRO_MEMORY_RTC:
      if(SNES::cartridge.has_srtc() || SNES::cartridge.has_spc7110rtc()) size = 20;
      break;
    case RETRO_MEMORY_SNES_BSX_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::Bsx) break;
      size = SNES::bsxcartridge.sram.size();
      break;
    case RETRO_MEMORY_SNES_BSX_PRAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::Bsx) break;
      size = SNES::bsxcartridge.psram.size();
      break;
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_A_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::SufamiTurbo) break;
      size = SNES::sufamiturbo.slotA.ram.size();
      break;
    case RETRO_MEMORY_SNES_SUFAMI_TURBO_B_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::SufamiTurbo) break;
      size = SNES::sufamiturbo.slotB.ram.size();
      break;
    case RETRO_MEMORY_SNES_GAME_BOY_RAM:
      if(SNES::cartridge.mode() != SNES::Cartridge::Mode::SuperGameBoy) break;
      size = GB::cartridge.ramsize;
      break;

    case RETRO_MEMORY_SYSTEM_RAM:
      size = 128 * 1024;
      break;
    case RETRO_MEMORY_VIDEO_RAM:
      size = 64 * 1024;
      break;
  }

  if(size == -1U) size = 0;
  return size;
}

