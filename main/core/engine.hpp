#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
namespace gotchi {
constexpr uint32_t API = 1;
enum class Mode : uint8_t { Quiet, Continuous, Classic };
enum class Action : uint8_t {
  None,
  Meal,
  Fruit,
  Treat,
  Water,
  Pet,
  Toilet,
  Flush,
  Clean,
  Bath,
  Wash,
  Rinse,
  Dry,
  Sleep,
  Wake,
  Medicine,
  Praise,
  Guide,
  Explore,
  Lullaby,
  Nap
};
enum class Game : uint8_t {
  None,
  Rps,
  Parity,
  Direction,
  Higher,
  Memory,
  Sequence,
  Fruit,
  Hide
};
enum Stat : uint8_t {
  Full,
  Happy,
  Energy,
  Hygiene,
  Health,
  Bond,
  Education,
  Urgency
};
enum class Stage : uint8_t { Egg, Baby, Child, Teen, Adult, Elder };
struct Rng {
  uint32_t state;
  uint32_t next();
  uint32_t uniform(uint32_t n);
};
struct Event {
  uint64_t age;
  char text[96];
};
struct Match {
  Game kind;
  uint8_t active, finished, rewarded, round, wins, losses, choice, parity, base,
      opponent;
  uint8_t pairs, cards[16], matched[16], first, second, sequence[32], length,
      input, showing;
  uint32_t id, rng;
  int32_t score;
  uint32_t elapsed, phase, spawn;
  int16_t basket, fruitX, fruitY;
  char result[128];
};
struct State {
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
static_assert(std::is_trivially_copyable<State>::value,
              "Save must be fixed data");
struct Food {
  char id[24], name[48], shortName[32];
  Action action;
  float fullness, happiness, health;
  uint8_t icon;
};
struct Rules {
  uint32_t incubationMin = 180, incubationMax = 300;
  uint64_t baby = 7200000, child = 86400000, teen = 259200000,
           adult = 1209600000;
  float decay[5]{1.8f, 1.1f, 1.0f, 1.2f, 3.2f};
  uint32_t tolerance = 7200, offlineLimit = 43200;
  uint32_t classicRiskHours = 48;
  uint8_t familyCount = 6;
};
struct Result {
  bool changed;
  const char *message;
};
State newborn(uint32_t profile, uint32_t seed, uint64_t id,
              const Rules &rules = {});
void log(State &, const char *);
void tick(State &, uint64_t milliseconds, const Rules &rules = {});
void resume(State &, int64_t wallSeconds, const Rules &rules = {});
Result act(State &, Action, const Food *food = nullptr);
void evolve(State &, const Rules &rules = {});
bool startGame(State &, Game, uint8_t pairs = 6);
Result play(State &, int input);
void tickGame(State &, uint32_t milliseconds);
void cancelGame(State &);
bool victory(const Match &);
int rps(int user, int pet);
bool even(int a, int b);
bool valid(const State &, uint32_t profile);
uint8_t presentationForm(const State &);
const char *stageName(Stage);
} // namespace gotchi
