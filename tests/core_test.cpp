#include <unistd.h>

#include "core/engine.hpp"
#include "storage/store.hpp"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace gotchi;
int main() {
  Rules r;
  auto egg = newborn(0, 42, 123, r);
  assert(egg.stage == Stage::Egg && egg.family < 6 &&
         egg.incubationTarget >= 180000);
  auto original = egg;
  tick(egg, 1000);
  assert(egg.family == original.family && egg.id == original.id);
  tick(egg, egg.incubationTarget - egg.incubation);
  assert(egg.stage == Stage::Baby && egg.age == 0);
  std::filesystem::path root =
      "/tmp/tab5gotchi-tests-" + std::to_string(getpid());
  std::filesystem::create_directories(root);
  Store store(root.string());
  assert(store.save(egg));
  State loaded;
  assert(store.load(0, loaded) == Load::Ok && loaded.id == egg.id);
  auto seeded = loaded;
  assert(store.save(loaded));
  {
    std::ofstream out(root / "profile0b.sav", std::ios::trunc);
    out << "truncated";
  }
  assert(store.load(0, loaded) == Load::Ok && loaded.sequence == 1);
  assert(!valid(loaded, 1));
  for (int u = 0; u < 3; u++)
    for (int p = 0; p < 3; p++) {
      assert(rps(u, p) == -rps(p, u));
      assert((rps(u, p) == 0) == (u == p));
    }
  for (int a = 0; a < 6; a++)
    for (int b = 0; b < 6; b++)
      assert(even(a, b) == ((a + b) % 2 == 0));
  assert(even(0, 0));
  auto s = seeded;
  s.stats[Full] = 95;
  assert(!act(s, Action::Meal).changed);
  s.stats[Full] = 20;
  assert(act(s, Action::Meal).changed);
  float full = s.stats[Full];
  assert(!act(s, Action::Meal).changed && s.stats[Full] == full);
  tick(s, 2000);
  s.stats[Urgency] = 90;
  assert(act(s, Action::Toilet).changed && s.stats[Urgency] == 0);
  tick(s, 2000);
  assert(act(s, Action::Flush).changed);
  tick(s, 2000);
  assert(act(s, Action::Bath).changed);
  assert(act(s, Action::Wash).changed);
  assert(act(s, Action::Rinse).changed);
  assert(act(s, Action::Dry).changed && s.stats[Hygiene] == 100);
  tick(s, 2000);
  s.sick = 0;
  assert(!act(s, Action::Medicine).changed);
  s.sick = 1;
  assert(act(s, Action::Medicine).changed);
  tick(s, 2000);
  s.cooldown[unsigned(Action::Sleep) - 1] = 0;
  assert(act(s, Action::Sleep).changed);
  float energy = s.stats[Energy];
  tick(s, 60000);
  assert(s.stats[Energy] > energy);
  assert(act(s, Action::Wake).changed);
  auto napping = seeded;
  napping.stats[Energy] = 20;
  assert(act(napping, Action::Nap).changed && napping.sleeping);
  tick(napping, 599000);
  assert(napping.sleeping && napping.action == Action::Nap);
  tick(napping, 1000);
  assert(!napping.sleeping && napping.action == Action::None &&
         napping.stats[Energy] > 75);
  for (int fam = 0; fam < 6; fam++)
    for (int teen = 2; teen <= 3; teen++)
      for (int adult = 0; adult < 3; adult++) {
        auto x = seeded;
        x.family = fam;
        x.stage = Stage::Teen;
        x.form = teen;
        x.tendencies[0] = x.tendencies[1] = x.tendencies[2] = 1;
        x.tendencies[adult] = 200;
        x.age = r.teen;
        evolve(x, r);
        assert(x.form == 4 + adult);
        auto branch = x.form;
        evolve(x, r);
        assert(x.form == branch);
      }
  auto quiet = seeded;
  quiet.lastWall = 1800000000;
  resume(quiet, 1800086400);
  assert(quiet.age == seeded.age);
  auto q = quiet;
  resume(quiet, 1790000000);
  assert(quiet.age == q.age);
  quiet.mode = Mode::Continuous;
  quiet.lastWall = 1800000000;
  resume(quiet, 1800086400);
  assert(quiet.age == 86400000 && quiet.stats[Health] >= 10);
  quiet.mode = Mode::Classic;
  resume(quiet, 1900000000);
  assert(!quiet.dead);
  auto neglected = seeded;
  neglected.mode = Mode::Classic;
  tick(neglected, 55ULL * 3600000);
  assert(neglected.dead && neglected.age < 55ULL * 3600000);
  assert(neglected.stage >= Stage::Teen);
  auto endedAge = neglected.age;
  tick(neglected, 3600000);
  assert(neglected.age == endedAge);
  auto protectedPet = seeded;
  tick(protectedPet, 55ULL * 3600000);
  assert(!protectedPet.dead && protectedPet.stats[Health] >= 10);
  for (Game g :
       {Game::Rps, Game::Parity, Game::Direction, Game::Higher, Game::Hide}) {
    s = seeded;
    assert(startGame(s, g));
    unsigned guard = 0;
    while (s.match.active && guard++ < 200) {
      int input = g == Game::Rps ? (s.match.opponent + 1) % 3
                  : g == Game::Parity
                      ? (s.match.parity == 0 ? s.match.opponent % 2
                                             : 1 - s.match.opponent % 2)
                  : g == Game::Direction || g == Game::Hide
                      ? s.match.opponent
                      : s.match.opponent > s.match.base;
      play(s, input);
      tickGame(s, 2000);
    }
    assert(s.match.finished && s.match.rewarded);
    auto count = s.gameCount;
    play(s, 0);
    tickGame(s, 2000);
    assert(s.gameCount == count);
  }
  s = seeded;
  assert(startGame(s, Game::Memory));
  while (s.match.active) {
    int first = -1, second = -1;
    for (int i = 0; i < 12; i++)
      if (!s.match.matched[i]) {
        if (first < 0)
          first = i;
        else if (s.match.cards[i] == s.match.cards[first]) {
          second = i;
          break;
        }
      }
    assert(first >= 0 && second >= 0);
    play(s, first);
    play(s, second);
    tickGame(s, 1000);
  }
  assert(s.match.wins == 6);
  s = seeded;
  startGame(s, Game::Sequence);
  assert(!play(s, 0).changed);
  tickGame(s, 2000);
  assert(play(s, s.match.sequence[0]).changed && s.match.length == 2);
  s = seeded;
  startGame(s, Game::Fruit);
  for (int i = 0; i < 1000; i++) {
    play(s, s.match.fruitX);
    tickGame(s, 30);
  }
  assert(s.match.finished && s.match.score > 0);
  s = seeded;
  startGame(s, Game::Hide);
  auto opponent = s.match.opponent;
  assert(store.save(s));
  assert(store.load(0, loaded) == Load::Ok &&
         loaded.match.opponent == opponent);
  for (int i = 0; i < 2; i++) {
    std::ofstream out(root / (i ? "profile0a.sav" : "profile0b.sav"),
                      std::ios::trunc);
    out << "bad";
  }
  assert(store.load(0, loaded) == Load::Corrupt);
  assert(!store.save(s));
  auto bedtime = seeded;
  auto tendency = bedtime.tendencies[0];
  assert(act(bedtime, Action::Sleep).changed);
  act(bedtime, Action::Wake);
  tick(bedtime, 1500);
  act(bedtime, Action::Sleep);
  assert(bedtime.tendencies[0] == tendency + 4);
  auto queued = seeded;
  queued.tendencies[0] = 1;
  queued.tendencies[1] = 200;
  queued.tendencies[2] = 1;
  queued.age = r.teen;
  evolve(queued, r);
  assert(queued.form == 5 && queued.pendingEvolution == 4);
  const int expected[] = {0, 1, 3, 5};
  for (int form : expected) {
    assert(presentationForm(queued) == form);
    queued.pendingEvolution--;
  }
  assert(presentationForm(queued) == 5);
  assert(store.restore(0, (root / "export.sav").string()) ==
         false); // nonexistent backup leaves evidence untouched
  auto profile1 = newborn(1, 999, 999);
  tick(profile1, profile1.incubationTarget);
  assert(store.save(profile1));
  State isolated{};
  assert(store.load(1, isolated) == Load::Ok && isolated.id == 999);
  assert(store.backup(1, (root / "export.sav").string()));
  {
    std::ofstream bad(root / "profile1a.sav", std::ios::trunc);
    bad << "bad A";
  }
  assert(store.load(1, isolated) == Load::Corrupt);
  assert(store.restore(1, (root / "export.sav").string()));
  assert(store.load(1, isolated) == Load::Ok && isolated.id == 999);
  assert(std::filesystem::exists(root /
                                 "recovery/profile1-restore0/profile1a.sav"));
  Settings cfg{};
  assert(saveSettings((root / "options").string(), cfg));
  cfg.music = 70;
  assert(saveSettings((root / "options").string(), cfg));
  Settings readCfg{};
  assert(loadSettings((root / "options").string(), readCfg) &&
         readCfg.music == 70);
  std::filesystem::remove_all(root);
  std::cout << "PASS: identidade, recuperação A/B, cuidados, evolução 36 "
               "caminhos, tempo e oito jogos\n";
}
