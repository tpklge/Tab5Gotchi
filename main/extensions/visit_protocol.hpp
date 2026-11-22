#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace visit {
constexpr size_t WIRE_SIZE = 36;
struct Packet {
  uint8_t kind = 1, family = 0, stage = 1, palette = 0, decor = 0;
  uint32_t code = 0;
  uint64_t request = 0, individual = 0;
};
std::array<uint8_t, WIRE_SIZE> encode(const Packet &);
bool decode(const uint8_t *, size_t, uint32_t expectedCode, Packet &);
} // namespace visit
