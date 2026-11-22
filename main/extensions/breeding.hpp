#pragma once
#include "core/engine.hpp"
namespace gotchi {
bool compatibleParents(const State &, const State &);
bool offspring(State &out, const State &a, const State &b, uint32_t profile,
               uint32_t seed, uint64_t id, const Rules &rules = {});
} // namespace gotchi
