#pragma once
#include "core/engine.hpp"
#include <string>
namespace gotchi {
bool inspectArchive(const std::string &, State &);
uint32_t crc32(const void *, size_t);
enum class Load { Missing, Ok, Corrupt };
class Store {
  std::string root;

public:
  explicit Store(std::string path) : root(std::move(path)) {}
  Load load(uint32_t profile, State &);
  bool save(State &);
  bool backup(uint32_t profile, const std::string &destination);
  bool restore(uint32_t profile, const std::string &source);
};
struct Settings {
  uint32_t version = 2;
  uint8_t master = 55, music = 30, sfx = 65, brightness = 70, retro = 0,
          abc = 0, tilt = 1, profile = 0, silentSleep = 1;
  uint16_t screenTimeoutSeconds = 120; // 0 = always on; 30-second steps.
};
bool loadSettings(const std::string &, Settings &);
bool saveSettings(const std::string &, const Settings &);
} // namespace gotchi
