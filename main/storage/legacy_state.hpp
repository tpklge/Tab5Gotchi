#pragma once
#include "core/engine.hpp"
namespace gotchi {
struct StateV1 {
  uint32_t format = 1, profile = 0, sequence = 0;
  uint64_t id = 0;
  uint32_t identityRng = 0, eventRng = 0, gameRng = 0;
  char name[49]{};
  uint8_t family = 0, palette = 0, detail = 0, personality = 0,
          preferredFood = 0;
  Stage stage = Stage::Egg;
  uint8_t form = 0, adult = 0;
  Mode mode = Mode::Quiet;
  uint8_t sleeping = 0, sick = 0, poop = 0, dead = 0, retired = 0, light = 1,
          bathStep = 0, pendingEvolution = 0;
  uint8_t sleepHour = 22, wakeHour = 7, decor = 0, favorite = 0;
  uint64_t age = 0, incubation = 0, incubationTarget = 240000, lastWall = 0,
           neglect = 0;
  float stats[8]{75, 75, 80, 90, 95, 25, 20, 0};
  uint32_t tendencies[3]{1, 1, 1};
  uint64_t cooldown[20]{};
  uint64_t gameRewardAt[8]{};
  Action action = Action::None;
  uint32_t actionLeft = 0, careCount = 0, gameCount = 0;
  uint64_t discovered = 0;
  uint32_t decorations = 1, transaction = 0;
  Match match{};
  Event events[40]{};
  uint8_t logHead = 0, logCount = 0;
};
static_assert(sizeof(StateV1) == 4872, "Legacy ABI");
}
