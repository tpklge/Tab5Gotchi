#include "core/engine.hpp"
#include "storage/store.hpp"
#include "storage/legacy_state.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace gotchi;
int main() {
  Rules rules;
  auto pet = newborn(0, 42, 123);
  tick(pet, pet.incubationTarget);
  auto capped = pet;
  rules.actionCooldownSeconds = 1;
  rules.actionDurationMs = 100;
  rules.activityCap = 7;
  auto before = capped.tendencies[0];
  for (int i = 0; i < 20; ++i) { assert(act(capped, Action::Pet, nullptr, rules).changed); tick(capped, 1000, rules); }
  assert(capped.tendencies[0] == before + 7);
  auto noCredit = capped.tendencies[0];
  capped.age = rules.baby; evolve(capped, rules);
  assert(act(capped, Action::Pet, nullptr, rules).changed);
  assert(capped.tendencies[0] > noCredit); // budget renews at next stage
  auto tuned = pet; rules.gains[4][0] = 1; rules.gains[4][1] = 7;
  auto bond = tuned.stats[Bond], happy = tuned.stats[Happy];
  assert(act(tuned, Action::Pet, nullptr, rules).changed);
  assert(tuned.stats[Bond] == bond + 1 && tuned.stats[Happy] == happy + 7);
  assert(!act(tuned, Action::Pet, nullptr, rules).changed);
  tick(tuned, 1000, rules); assert(act(tuned, Action::Pet, nullptr, rules).changed);
  auto diminishing = pet;
  auto firstBond = diminishing.stats[Bond];
  assert(act(diminishing, Action::Pet, nullptr, rules).changed);
  float firstGain = diminishing.stats[Bond] - firstBond;
  tick(diminishing, 1000, rules);
  auto nextBond = diminishing.stats[Bond];
  assert(act(diminishing, Action::Pet, nullptr, rules).changed);
  assert(diminishing.stats[Bond] - nextBond < firstGain);
  auto explorer = pet, control = pet;
  auto identity = explorer.identityRng, game = explorer.gameRng, event = explorer.eventRng;
  assert(act(explorer, Action::Explore, nullptr, rules).changed);
  tick(explorer, rules.exploreDurationMs, rules);
  assert(explorer.eventRng != event && explorer.identityRng == identity && explorer.gameRng == game);
  assert(explorer.action == Action::None && explorer.logCount > control.logCount);
  auto root = std::filesystem::temp_directory_path() / ("gotchi-migration-" + std::to_string(getpid()));
  std::filesystem::create_directories(root);
  StateV1 legacy{}; std::memcpy(static_cast<void *>(&legacy), &pet, sizeof legacy); legacy.format = 1;
  legacy.sequence = 37;
  uint32_t header[] = {0x31475442, 1, sizeof legacy, crc32(&legacy, sizeof legacy)};
  {
    std::ofstream out(root / "profile0a.sav", std::ios::binary);
    out.write(reinterpret_cast<char *>(header), sizeof header);
    out.write(reinterpret_cast<char *>(&legacy), sizeof legacy);
  }
  Store store(root.string()); State migrated;
  assert(store.load(0, migrated) == Load::Ok && migrated.format == 2 && migrated.id == pet.id && migrated.sequence == 37);
  assert(migrated.birthWall == 0 && migrated.parents[0] == 0 && migrated.activityCredits[0] == 0);
  assert(migrated.identityRng == pet.identityRng && migrated.family == pet.family && migrated.discovered == pet.discovered);
  assert(store.backup(0, (root / "migrated.sav").string()));
  assert(store.save(migrated) && migrated.sequence == 38);
  State reloaded; assert(store.load(0, reloaded) == Load::Ok && reloaded.sequence == 38);
  std::ofstream(root / "profile0b.sav", std::ios::trunc) << "bad";
  assert(store.load(0, reloaded) == Load::Ok && reloaded.sequence == 37 && reloaded.format == 2);
  assert(inspectArchive((root / "profile0a.sav").string(), reloaded));
  assert(store.restore(0, (root / "migrated.sav").string()));
  std::filesystem::remove_all(root);
  std::cout << "PASS: limites por etapa, ganhos configuráveis, exploração e migração v1/v2\n";
}
