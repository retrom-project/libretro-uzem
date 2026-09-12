#include <cassert>
#include <cstdio>
#include <cstring>
#include <queue>
#include <vector>
#define private public
#include "avr8.h"
#undef private
#include "libretro.h"
extern avr8 uzebox;
extern input_driver_t input_driver_libretro;
static uint32_t pixels[720 * 224];
static bool environment(unsigned, void*) { return false; }
int main() {
  retro_set_environment(environment);
  retro_init();
  uzebox.vdrv->framebuffer = pixels;
  uzebox.vdrv->stride = 720;
  uzebox.cycleCounter = 12345678;
  uzebox.TCNT1 = 200;
  uzebox.timer1_next = 37;
  uzebox.itd_TIFR1 = 8;
  uzebox.eeprom[5] = 91;
  uzebox.latched_buttons[0] = 0x1234;
  uzebox.sram[19] = 42;
  std::vector<uint8_t> saved(retro_serialize_size());
  assert(retro_serialize(saved.data(), saved.size()));
  uzebox.cycleCounter = 9;
  uzebox.TCNT1 = 10;
  uzebox.timer1_next = 11;
  uzebox.itd_TIFR1 = 0;
  uzebox.eeprom[5] = 0;
  uzebox.latched_buttons[0] = 0;
  uzebox.sram[19] = 0;
  assert(retro_unserialize(saved.data(), saved.size()));
  assert(uzebox.cycleCounter == 12345678);
  assert(uzebox.TCNT1 == 200 && uzebox.timer1_next == 37 && uzebox.itd_TIFR1 == 8);
  assert(uzebox.eeprom[5] == 91 && uzebox.latched_buttons[0] == 0x1234);
  assert(uzebox.sram[19] == 42);
  std::vector<uint8_t> again(saved.size());
  assert(retro_serialize(again.data(), again.size()) && saved == again);
  assert(!retro_unserialize(saved.data(), saved.size() - 1));
  saved.back() ^= 1;
  assert(!retro_unserialize(saved.data(), saved.size()));
  assert(uzebox.sram[19] == 42);
  uzebox.spiState = SPI_IDLE_STATE;
  uzebox.spiByte = 0x40;
  uzebox.update_spi();
  assert(uzebox.SPDR == 0xff && uzebox.spiState == SPI_IDLE_STATE);
  puts("state timer/EEPROM/input roundtrip and corrupt/truncated rejection: PASS");
}
