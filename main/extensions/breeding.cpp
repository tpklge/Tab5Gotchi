#include "breeding.hpp"
#include <cstdio>
namespace gotchi {
bool compatibleParents(const State &a, const State &b) {
  return a.id && b.id && a.id != b.id && a.profile != b.profile &&
         a.stage == Stage::Adult && b.stage == Stage::Adult && !a.dead &&
         !b.dead && !a.retired && !b.retired;
}
bool offspring(State &out, const State &a, const State &b, uint32_t profile,
               uint32_t seed, uint64_t id, const Rules &r) {
  if (!compatibleParents(a, b) || profile == a.profile ||
      profile == b.profile || profile > 2 || !id)
    return false;
  out = newborn(profile, seed, id, r);
  Rng rng{out.identityRng};
  const State &parent = rng.uniform(2) ? a : b;
  out.parents[0] = a.id;
  out.parents[1] = b.id;
  out.family = parent.family;
  out.palette = parent.palette;
  out.detail = parent.detail;
  if (rng.uniform(8) == 0)
    out.palette = rng.uniform(4);
  out.identityRng = rng.state;
  log(out, "Um ovo de uma nova geração local chegou.");
  return true;
}
} // namespace gotchi
