#pragma once
#include <cstdint>

namespace gotchi::power {
// Voltage-only estimate for the Tab5 NP-F550 (two Li-ion cells in series).
// Not a fuel gauge: load, charging and battery age affect the result.
// Hardware: https://docs.m5stack.com/en/core/Tab5
inline int batteryPercent(int millivolts) {
  // A missing pack/invalid measurement must not look like an empty battery.
  if (millivolts < 5500 || millivolts > 8800)
    return -1;
  constexpr int mv[] = {6000, 6600, 7000, 7200, 7400,
                        7600, 7800, 8000, 8200, 8400};
  constexpr int pct[] = {0, 5, 10, 20, 40, 60, 75, 85, 95, 100};
  if (millivolts <= mv[0])
    return 0;
  for (unsigned i = 1; i < sizeof(mv) / sizeof(mv[0]); ++i)
    if (millivolts <= mv[i])
      return pct[i - 1] + (millivolts - mv[i - 1]) * (pct[i] - pct[i - 1]) /
                              (mv[i] - mv[i - 1]);
  return 100;
}

inline bool validTimeout(unsigned seconds) {
  return seconds <= 1800 && seconds % 30 == 0;
}

inline bool shouldBlank(uint32_t inactiveMs, unsigned timeoutSeconds,
                        bool playing) {
  return timeoutSeconds && !playing &&
         inactiveMs >= timeoutSeconds * 1000u;
}
} // namespace gotchi::power
