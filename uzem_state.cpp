// MIT. Retrom Uzem state v1: explicit little-endian fields, no process pointers.
#include "avr8.h"
#include "uzem_state.h"
#include <cstring>
#include <vector>
#include <memory>
namespace {
constexpr size_t frame_pixels = 720 * 224;
constexpr size_t header_size = 20;
struct Writer {
    std::vector<uint8_t> bytes;
    template<typename T> void scalar(T& value) {
        uint32_t v = static_cast<uint32_t>(value);
        for (int i = 0; i < 4; ++i) bytes.push_back(v >> (8 * i));
    }
    template<typename T, size_t N> void array(T (&values)[N]) {
        for (size_t i = 0; i < N; ++i) scalar(values[i]);
    }
};
struct Reader {
    const uint8_t* next;
    template<typename T> void scalar(T& value) {
        uint32_t v = uint32_t(next[0]) | uint32_t(next[1]) << 8 |
            uint32_t(next[2]) << 16 | uint32_t(next[3]) << 24;
        value = static_cast<T>(v); next += 4;
    }
    template<typename T, size_t N> void array(T (&values)[N]) {
        for (size_t i = 0; i < N; ++i) scalar(values[i]);
    }
};
}
struct UzemStateAccess {
    template<typename Archive> static void transfer(avr8& cpu, Archive& a) {
        a.scalar(cpu.pc);
        a.scalar(cpu.currentPc);
        a.scalar(cpu.cycleCounter);
        a.scalar(cpu.elapsedCycles);
        a.scalar(cpu.prevCyclesCounter);
        a.scalar(cpu.elapsedCyclesSleep);
        a.scalar(cpu.lastCyclesSleep);
        a.scalar(cpu.prevPortB);
        a.scalar(cpu.prevWDR);
        a.scalar(cpu.watchdogTimer);
        a.scalar(cpu.cycle_ctr_ins);
        a.scalar(cpu.T16_latch);
        a.scalar(cpu.TCNT1);
        a.scalar(cpu.timer1_next);
        a.scalar(cpu.timer1_base);
        a.scalar(cpu.itd_TIFR1);
        a.scalar(cpu.dly_out);
        a.scalar(cpu.dly_TCCR1B);
        a.scalar(cpu.dly_TCNT1L);
        a.scalar(cpu.dly_TCNT1H);
        a.scalar(cpu.rngState);
        a.scalar(cpu.scanline_count);
        a.scalar(cpu.left_edge_cycle);
        a.scalar(cpu.scanline_top);
        a.scalar(cpu.left_edge);
        a.scalar(cpu.inset);
        a.scalar(cpu.pixel_raw);
        a.scalar(cpu.new_input_mode);
        a.scalar(cpu.spiByte);
        a.scalar(cpu.spiTransfer);
        a.scalar(cpu.spiClock);
        a.scalar(cpu.spiCycleWait);
        a.scalar(cpu.spiState);
        a.scalar(cpu.spiCommand);
        a.scalar(cpu.spiCommandDelay);
        a.scalar(cpu.spiArg);
        a.scalar(cpu.spiByteCount);
        a.array(cpu.r); a.array(cpu.io); a.array(cpu.sram); a.array(cpu.eeprom);
        a.array(cpu.progmem); a.array(cpu.scanline_buf); a.array(cpu.latched_buttons);
        a.array(cpu.spiResponseBuffer);
    }
};
uint32_t uzem_state_hash(const uint8_t* bytes, size_t size) {
    uint32_t hash = 2166136261U;
    for (size_t i = 0; i < size; ++i) hash = (hash ^ bytes[i]) * 16777619U;
    return hash;
}
size_t uzem_state_size() {
    static const size_t size = []() {
        std::unique_ptr<avr8> cpu(new avr8()); Writer a;
        UzemStateAccess::transfer(*cpu, a);
        return header_size + a.bytes.size() + frame_pixels * 4;
    }();
    return size;
}
bool uzem_state_save(avr8& cpu, uint32_t cartridge, void* data, size_t size) {
    if (!data || size != uzem_state_size() || !cpu.vdrv->framebuffer) return false;
    Writer a;
    uint32_t magic = 0x315a5552, version = 1, length = uint32_t(size), checksum = 0;
    a.scalar(magic); a.scalar(version); a.scalar(length); a.scalar(cartridge); a.scalar(checksum);
    UzemStateAccess::transfer(cpu, a);
    for (size_t y = 0; y < 224; ++y)
        for (size_t x = 0; x < 720; ++x) a.scalar(cpu.vdrv->framebuffer[y * cpu.vdrv->stride + x]);
    checksum = uzem_state_hash(a.bytes.data() + header_size, size - header_size);
    for (int i = 0; i < 4; ++i) a.bytes[16 + i] = checksum >> (8 * i);
    std::memcpy(data, a.bytes.data(), size);
    return true;
}
bool uzem_state_load(avr8& cpu, uint32_t cartridge, const void* data, size_t size) {
    if (!data || size != uzem_state_size() || !cpu.vdrv->framebuffer) return false;
    Reader a{static_cast<const uint8_t*>(data)};
    uint32_t magic, version, length, identity, checksum;
    a.scalar(magic); a.scalar(version); a.scalar(length); a.scalar(identity); a.scalar(checksum);
    if (magic != 0x315a5552 || version != 1 || length != size || identity != cartridge ||
        checksum != uzem_state_hash(a.next, size - header_size)) return false;
    // Validate guest-controlled positions before changing the live machine.
    Reader check = a; std::unique_ptr<avr8> probe(new avr8());
    UzemStateAccess::transfer(*probe, check);
    if (probe->pc >= progSize / 2 || probe->currentPc >= progSize / 2 ||
        (probe->scanline_count != -999 && (probe->scanline_count < -100 || probe->scanline_count > 1000))) return false;
    UzemStateAccess::transfer(cpu, a);
    for (size_t y = 0; y < 224; ++y)
        for (size_t x = 0; x < 720; ++x) a.scalar(cpu.vdrv->framebuffer[y * cpu.vdrv->stride + x]);
    // No SD/keyboard peripherals are mounted by this standalone-ROM target.
    cpu.spiResponsePtr = cpu.spiResponseEnd = nullptr;
    cpu.decodeFlash();
    return true;
}
