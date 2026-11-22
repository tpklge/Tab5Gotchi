#include "visit_protocol.hpp"
#include "storage/store.hpp"
#include <cstring>
namespace visit {
static void put(uint8_t *p, uint64_t v, unsigned n) {
  for (unsigned i = 0; i < n; i++)
    p[i] = v >> (i * 8);
}
static uint64_t get(const uint8_t *p, unsigned n) {
  uint64_t v = 0;
  for (unsigned i = 0; i < n; i++)
    v |= uint64_t(p[i]) << (i * 8);
  return v;
}
std::array<uint8_t, WIRE_SIZE> encode(const Packet &p) {
  std::array<uint8_t, WIRE_SIZE> b{};
  memcpy(b.data(), "T5GV", 4);
  b[4] = 1;
  b[5] = 0;
  b[6] = p.kind;
  b[7] = WIRE_SIZE;
  put(b.data() + 8, p.code, 4);
  put(b.data() + 12, p.request, 8);
  put(b.data() + 20, p.individual, 8);
  b[28] = p.family;
  b[29] = p.stage;
  b[30] = p.palette;
  b[31] = p.decor;
  put(b.data() + 32, gotchi::crc32(b.data(), 32), 4);
  return b;
}
bool decode(const uint8_t *b, size_t n, uint32_t code, Packet &p) {
  if (n != WIRE_SIZE || memcmp(b, "T5GV", 4) || b[4] != 1 || b[5] != 0 ||
      b[6] < 1 || b[6] > 2 || b[7] != WIRE_SIZE || get(b + 8, 4) != code ||
      code < 100000 || code > 999999 ||
      get(b + 32, 4) != gotchi::crc32(b, 32) || !get(b + 12, 8) ||
      !get(b + 20, 8) || b[28] >= 8 || b[29] < 1 || b[29] > 5 || b[30] > 3 ||
      b[31] >= 12)
    return false;
  p.kind = b[6];
  p.code = code;
  p.request = get(b + 12, 8);
  p.individual = get(b + 20, 8);
  p.family = b[28];
  p.stage = b[29];
  p.palette = b[30];
  p.decor = b[31];
  return true;
}
} // namespace visit
