#pragma once
#include <cstddef>
#include <cstdint>
struct avr8;
size_t uzem_state_size();
uint32_t uzem_state_hash(const uint8_t*, size_t);
bool uzem_state_save(avr8&, uint32_t, void*, size_t);
bool uzem_state_load(avr8&, uint32_t, const void*, size_t);
