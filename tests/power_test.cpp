#include "board/power.hpp"
#include "storage/store.hpp"
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unistd.h>

using namespace gotchi;

template <typename T>
static void fixture(const std::string &path, const T &payload) {
  uint32_t header[] = {0x31475442, 1, sizeof(T), crc32(&payload, sizeof(T))};
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  out.write(reinterpret_cast<const char *>(header), sizeof(header));
  out.write(reinterpret_cast<const char *>(&payload), sizeof(payload));
  assert(out.good());
}

int main() {
  assert(power::batteryPercent(-1) == -1);
  assert(power::batteryPercent(0) == -1);
  assert(power::batteryPercent(4200) == -1); // not a 2S pack reading
  assert(power::batteryPercent(9000) == -1);
  assert(power::batteryPercent(6000) == 0);
  assert(power::batteryPercent(7400) == 40);
  assert(power::batteryPercent(8400) == 100);
  assert(power::batteryPercent(8600) == 100);
  int previous = 0;
  for (int mv = 5500; mv <= 8800; ++mv) {
    int percent = power::batteryPercent(mv);
    assert(percent >= previous && percent <= 100);
    previous = percent;
  }
  assert(power::validTimeout(0) && power::validTimeout(30) &&
         power::validTimeout(1800));
  assert(!power::validTimeout(1) && !power::validTimeout(1830));
  assert(!power::shouldBlank(119999, 120, false));
  assert(power::shouldBlank(120000, 120, false));
  assert(power::shouldBlank(1800000, 1800, false));
  assert(!power::shouldBlank(UINT32_MAX, 0, false));
  assert(!power::shouldBlank(200000, 120, true)); // don't interrupt a game
  assert(!power::shouldBlank(0, 120, false)); // wake resets inactivity

  auto root = std::filesystem::temp_directory_path() /
              ("tab5gotchi-power-" + std::to_string(getpid()));
  std::filesystem::create_directories(root);
  const std::string path = (root / "settings").string();
  struct LegacySettings {
    uint32_t version;
    uint8_t master, music, sfx, brightness, retro, abc, tilt, profile,
        silentSleep;
  } legacy;
  std::memset(&legacy, 0xff, sizeof legacy); // padding is not a timeout
  legacy.version = 1;
  legacy.master = 55;
  legacy.music = 25;
  legacy.sfx = 65;
  legacy.brightness = 0;
  legacy.retro = 1;
  legacy.abc = 0;
  legacy.tilt = 0;
  legacy.profile = 2;
  legacy.silentSleep = 1;
  fixture(path + "a.cfg", legacy);
  Settings settings;
  assert(loadSettings(path, settings));
  assert(settings.version == 2 && settings.screenTimeoutSeconds == 120 &&
         settings.brightness == 10 && settings.profile == 2 &&
         settings.master == 55 && settings.music == 25 && settings.sfx == 65 &&
         settings.retro == 1 && settings.tilt == 0 && settings.silentSleep == 1);
  settings.screenTimeoutSeconds = 300;
  assert(saveSettings(path, settings));
  Settings loaded;
  assert(loadSettings(path, loaded) && loaded.screenTimeoutSeconds == 300 &&
         loaded.profile == 2);
  settings.screenTimeoutSeconds = 0;
  assert(saveSettings(path, settings));
  assert(loadSettings(path, loaded) && loaded.screenTimeoutSeconds == 0);

  // CRC-valid but semantically invalid primary must fall back to B.
  settings.screenTimeoutSeconds = 7;
  fixture(path + "a.cfg", settings);
  assert(loadSettings(path, loaded) && loaded.screenTimeoutSeconds == 300);
  assert(!saveSettings(path, settings));
  settings.screenTimeoutSeconds = 1800;
  settings.brightness = 101;
  fixture(path + "a.cfg", settings);
  assert(loadSettings(path, loaded) && loaded.brightness == 10);
  settings.brightness = 70;
  settings.version = 99;
  fixture(path + "a.cfg", settings);
  assert(loadSettings(path, loaded) && loaded.version == 2);
  settings.version = 2;
  settings.tilt = 2;
  assert(!saveSettings(path, settings));

  // Failed reads leave the caller's settings intact, including defaults.
  std::filesystem::remove(path + "b.cfg");
  loaded = Settings{};
  assert(!loadSettings(path, loaded));
  assert(loaded.brightness == 70 && loaded.screenTimeoutSeconds == 120);
  settings = Settings{};
  settings.screenTimeoutSeconds = 1800;
  assert(saveSettings(path, settings));
  settings.brightness = 45;
  assert(saveSettings(path, settings));
  {
    std::ofstream corrupt(path + "a.cfg", std::ios::trunc);
    corrupt << "truncated";
  }
  assert(loadSettings(path, loaded) && loaded.brightness == 70 &&
         loaded.screenTimeoutSeconds == 1800);
  std::filesystem::remove_all(root);
  std::cout << "PASS: bateria 2S, inatividade, migração de ajustes e backup A/B\n";
}
