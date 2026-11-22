#include "extensions/breeding.hpp"
#include <cassert>
#include <iostream>
using namespace gotchi;
int main() {
  State a = newborn(0, 42, 1), b = newborn(1, 43, 2), c{};
  assert(!compatibleParents(a, b));
  a.stage = b.stage = Stage::Adult;
  a.family = 0;
  b.family = 4;
  a.palette = 2;
  b.palette = 3;
  a.detail = 1;
  b.detail = 0;
  for (unsigned seed = 1; seed < 500; seed++) {
    assert(offspring(c, a, b, 2, seed, seed + 100));
    assert(c.parents[0] == a.id && c.parents[1] == b.id);
    assert(c.stage == Stage::Egg && (c.family == 0 || c.family == 4));
    assert(c.detail == (c.family == 0 ? a.detail : b.detail));
    assert(c.profile == 2 && c.id == seed + 100);
  }
  assert(!offspring(c, a, b, 0, 42, 999));
  b.dead = 1;
  assert(!compatibleParents(a, b));
  std::cout << "PASS: herança local, pais compatíveis e ausência de híbridos\n";
}
