#include "engine.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <iterator>
namespace gotchi {
uint32_t Rng::next() {
  uint32_t x = state ? state : 0x6d2b79f5;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return state = x;
}
uint32_t Rng::uniform(uint32_t n) {
  if (!n)
    return 0;
  uint32_t x, limit = uint32_t(-n) % n;
  do {
    x = next();
  } while (x < limit);
  return x % n;
}
static void clamp(State &s) {
  for (float &v : s.stats)
    v = std::clamp(v, 0.f, 100.f);
}
void log(State &s, const char *t) {
  auto &e = s.events[s.logHead];
  e.age = s.age;
  snprintf(e.text, sizeof e.text, "%s", t);
  s.logHead = (s.logHead + 1) % 40;
  s.logCount = std::min(40, int(s.logCount) + 1);
}
State newborn(uint32_t profile, uint32_t seed, uint64_t id, const Rules &r) {
  State s{};
  s.profile = profile;
  s.id = id;
  s.identityRng = seed;
  Rng rng{seed};
  s.family = rng.uniform(std::clamp<int>(r.familyCount, 1, 8));
  s.palette = rng.uniform(4);
  s.detail = rng.uniform(2);
  s.personality = rng.uniform(3);
  s.tendencies[s.personality] += 2;
  s.preferredFood = rng.uniform(12);
  s.favorite = s.decor = rng.uniform(6);
  s.decorations |= 1u << s.favorite;
  s.incubationTarget =
      (r.incubationMin + rng.uniform(r.incubationMax - r.incubationMin + 1)) *
      1000ULL;
  s.identityRng = rng.state;
  s.eventRng = seed ^ 0x8da6b343;
  s.gameRng = seed ^ 0xd8163841;
  snprintf(s.name, sizeof s.name, "Bichinho %u", unsigned(profile + 1));
  log(s, "Um ovo chegou! A família será revelada no nascimento.");
  return s;
}
const char *stageName(Stage s) {
  static const char *n[] = {"Ovo",         "Bebê",   "Criança",
                            "Adolescente", "Adulto", "Idoso"};
  return n[std::min(5, int(s))];
}
uint8_t presentationForm(const State &s) {
  if (!s.pendingEvolution)
    return s.form;
  int stage = std::max(1, int(s.stage) - int(s.pendingEvolution) + 1);
  switch (Stage(stage)) {
  case Stage::Baby:
    return 0;
  case Stage::Child:
    return 1;
  case Stage::Teen:
    return s.adult & 0x80 ? 3 : 2;
  case Stage::Adult:
    return 4 + (s.adult & 3);
  case Stage::Elder:
    return 7;
  default:
    return s.form;
  }
}
static void tendency(State &s, const Rules &r, unsigned activity, unsigned kind, float points) {
  if (activity >= 28 || s.activityCredits[activity] >= r.activityCap) return;
  float quality = (s.stats[Health] >= 70 && s.stats[Hygiene] >= 50) ? 1.25f : 1.f;
  unsigned gain = std::max(1, int(points * quality));
  gain = std::min<unsigned>(gain, r.activityCap - s.activityCredits[activity]);
  s.activityCredits[activity] += gain;
  s.tendencies[kind] += gain;
}
void evolve(State &s, const Rules &r) {
  if (s.stage == Stage::Egg || s.dead || s.retired)
    return;
  Stage target = s.age < r.baby    ? Stage::Baby
                 : s.age < r.child ? Stage::Child
                 : s.age < r.teen  ? Stage::Teen
                 : s.age < r.adult ? Stage::Adult
                                   : Stage::Elder;
  while (s.stage < target) {
    s.stage = Stage(int(s.stage) + 1);
    if (s.stage == Stage::Child)
      s.form = 1;
    else if (s.stage == Stage::Teen) {
      s.form =
          (s.tendencies[0] >= s.tendencies[1] + s.tendencies[2] / 2) ? 2 : 3;
      s.adult = s.form == 3 ? 0x80 : 0;
    } else if (s.stage == Stage::Adult) {
      Rng rng{s.identityRng};
      unsigned best = 0;
      for (unsigned i = 1; i < 3; i++)
        if (s.tendencies[i] > s.tendencies[best] ||
            (s.tendencies[i] == s.tendencies[best] && rng.uniform(2)))
          best = i;
      s.identityRng = rng.state;
      s.adult = (s.adult & 0x80) | best;
      s.form = 4 + best;
    } else if (s.stage == Stage::Elder)
      s.form = 7;
    if (s.stage == Stage::Teen)
      for (auto &t : s.tendencies)
        t = 1 + t / 4;
    s.milestoneAge[unsigned(s.stage)] = s.age;
    std::fill(std::begin(s.activityCredits), std::end(s.activityCredits), 0);
    s.gamesSeen = 0;
    s.pendingEvolution++;
    s.discovered |= 1ULL << (s.family * 8 + s.form);
    char msg[96];
    snprintf(msg, sizeof msg, "Evolução: %s! Tendência %u/%u/%u.",
             stageName(s.stage), unsigned(s.tendencies[0]),
             unsigned(s.tendencies[1]), unsigned(s.tendencies[2]));
    log(s, msg);
  }
}
static void needs(State &s, uint64_t ms, const Rules &r) {
  float minutes = ms / 60000.f;
  for (int i = 0; i < 4; i++) {
    float d = r.decay[i] * minutes / 10.f;
    if (i == Energy && s.sleeping)
      d = -6 * minutes;
    if (i == Hygiene && s.poop)
      d *= 3;
    s.stats[i] -= d;
  }
  s.stats[Urgency] += r.decay[4] * minutes / 10.f;
  if (s.stats[Urgency] >= 100) {
    s.stats[Urgency] = 30;
    s.poop = 1;
    s.stats[Hygiene] -= 12;
    log(s, "Um acidente aconteceu. A limpeza resolve!");
  }
  bool danger =
      s.stats[Full] < 12 || s.stats[Hygiene] < 18 || s.stats[Energy] < 8;
  if (danger) {
    s.stats[Health] -= minutes * .12f;
    s.neglect += ms;
  } else {
    s.stats[Health] += minutes * .04f;
    s.neglect = 0;
  }
  if (!s.sick && s.stats[Health] < 45) {
    s.sick = 1;
    log(s, "Indisposição: confira alimentação, limpeza e descanso.");
  }
  if (s.sick && s.stats[Health] > 70) {
    s.sick = 0;
    log(s, "Recuperado! Obrigado pelos cuidados.");
  }
  if (s.mode == Mode::Classic && s.neglect > uint64_t(r.classicRiskHours) * 3600000 &&
      s.stats[Health] < 1) {
    s.dead = 1;
    log(s, "O ciclo chegou ao fim após longa falta de cuidados.");
  }
  if (s.mode != Mode::Classic)
    s.stats[Health] = std::max(10.f, s.stats[Health]);
  clamp(s);
}
void tick(State &s, uint64_t ms, const Rules &r) {
  if (s.dead || s.retired)
    return;
  if (s.stage == Stage::Egg) {
    uint64_t remaining = s.incubationTarget - s.incubation;
    if (ms < remaining) {
      s.incubation += ms;
      return;
    }
    ms -= remaining;
    s.incubation = s.incubationTarget;
    s.stage = Stage::Baby;
    s.form = 0;
    s.discovered |= 1ULL << (s.family * 8);
    s.pendingEvolution++;
    log(s, "Nasceu! Um novo amigo para conhecer.");
  }
  // Integrate bounded chunks; monotonic caller only passes active elapsed time.
  while (ms) {
    uint64_t step = std::min<uint64_t>(ms, 60000);
    s.age += step;
    evolve(s, r);
    needs(s, step, r);
    if (s.actionLeft) {
      s.actionLeft = step >= s.actionLeft ? 0 : s.actionLeft - uint32_t(step);
      if (!s.actionLeft) {
        if (s.action == Action::Explore) {
          Rng rng{s.eventRng};
          unsigned event = rng.uniform(6);
          s.eventRng = rng.state;
          static const char *messages[] = {
            "No quintal, encontramos uma folha com formato de coração!",
            "Uma borboleta acompanhou nosso passeio.",
            "Descobrimos uma pedrinha brilhante perto do lago.",
            "Ouvimos um passarinho cantar. Que passeio tranquilo!",
            "Encontramos um cata-vento para decorar a casa!",
            "Uma nuvem engraçada parecia outro bichinho."};
          if (event == 4) s.decorations |= 1u << 7;
          s.stats[Happy] = std::min(100.f, s.stats[Happy] + 1);
          log(s, messages[event]);
        }
        if (s.action == Action::Nap) {
          s.sleeping = 0;
          s.light = 1;
          log(s, "Soneca concluída. Energia recuperada!");
        }
        s.action = Action::None;
      }
    }
    ms -= step;
    if (s.dead)
      break;
  }
  evolve(s, r);
  s.decorations |= (s.careCount >= 3 ? 3 : 1) | (s.gameCount >= 1 ? 12 : 0) |
                   (s.age >= 3600000 ? 48 : 0) |
                   (s.stats[Bond] >= 50 ? 4032 : 0);
}
void resume(State &s, int64_t wall, const Rules &r) {
  constexpr int64_t minWall = 1704067200, maxWall = 4102444800;
  if (wall < minWall || wall > maxWall) {
    log(s, "Relógio indisponível; ausência suspensa.");
    return;
  }
  if (s.lastWall && wall < int64_t(s.lastWall)) {
    log(s, "Relógio voltou; ausência suspensa.");
    s.lastWall = wall;
    return;
  }
  uint64_t elapsed = s.lastWall ? wall - s.lastWall : 0;
  s.lastWall = wall;
  if (s.mode == Mode::Quiet || !elapsed)
    return;
  if (elapsed > 31ULL * 86400) {
    log(s, "Salto grande no relógio; ausência limitada a 12 horas.");
    elapsed = r.offlineLimit;
  }
  uint64_t wear =
      elapsed > r.tolerance
          ? std::min<uint64_t>(elapsed - r.tolerance, r.offlineLimit)
          : 0;
  Mode mode = s.mode;
  s.mode = Mode::Continuous; // Clock/offline changes cannot cause death.
  if (s.stage == Stage::Egg)
    tick(s, std::min<uint64_t>(elapsed, r.offlineLimit) * 1000, r);
  else {
    uint64_t old = s.age;
    needs(s, wear * 1000, r);
    s.age = old + elapsed * 1000;
    evolve(s, r);
  }
  s.mode = mode;
  log(s, "Retomada: crescimento registrado e desgaste de ausência limitado.");
}
Result act(State &s, Action a, const Food *food, const Rules &r) {
  if (s.dead || s.retired)
    return {false, "Este ciclo terminou. Veja o álbum."};
  if (s.stage == Stage::Egg)
    return {false, "O ovo precisa de um pouco mais de tempo."};
  if (s.actionLeft && a != Action::Wake && a != Action::Wash &&
      a != Action::Rinse && a != Action::Dry)
    return {false, "Espere a ação terminar."};
  if (s.sleeping && a != Action::Wake && a != Action::Pet &&
      a != Action::Lullaby)
    return {false, "Seu amigo está dormindo. Você pode acordá-lo."};
  unsigned idx = unsigned(a) - 1;
  if (idx >= 20)
    return {false, "Ação inválida."};
  uint64_t now = s.age + 1;
  bool repeat =
      s.cooldown[idx] &&
      now - s.cooldown[idx] <
          uint64_t((a == Action::Praise || a == Action::Guide) ? r.educationCooldownSeconds : r.actionCooldownSeconds) * 1000;
  if (repeat && a != Action::Water && a != Action::Wake && a != Action::Sleep &&
      a != Action::Wash && a != Action::Rinse && a != Action::Dry &&
      a != Action::Flush)
    return {false, "Vamos variar um pouco? Tente de novo mais tarde."};
  float gain[3];
  float socialScale = (a == Action::Pet || a == Action::Lullaby || a == Action::Praise || a == Action::Guide) ?
      std::max(r.socialMinimumScale, 1.f - float(s.activityCredits[idx]) / std::max<unsigned>(1, r.activityCap)) : 1.f;
  for (int i = 0; i < 3; ++i) gain[i] = r.gains[idx][i] * socialScale;
  const char *msg = "Tudo certo!";
  switch (a) {
  case Action::Meal:
  case Action::Fruit:
  case Action::Treat:
    if (s.stats[Full] > 88)
      return {false, "Já estou satisfeito. Guarde para depois!"};
    s.stats[Full] += food                 ? food->fullness
                     : a == Action::Meal  ? gain[0]
                     : a == Action::Fruit ? gain[0]
                                          : gain[0];
    s.stats[Happy] += food ? food->happiness : gain[1];
    s.stats[Health] += food ? food->health : gain[2];
    s.stats[Urgency] += 7;
    msg = food ? food->name : "Que delícia!";
    if (food && food->icon == s.preferredFood) {
      s.stats[Happy] += 2;
      msg = "Minha comida favorita! Obrigado!";
    }
    break;
  case Action::Water:
    s.stats[Health] += repeat ? 0 : gain[0];
    msg = "Água fresquinha!";
    break;
  case Action::Pet:
  case Action::Lullaby:
    s.stats[Bond] += gain[0];
    s.stats[Happy] += gain[1];
    tendency(s, r, idx, 0, r.tendencyWeights[0]);
    msg = s.sleeping ? "Uma canção de ninar e um carinho suave."
                     : "Carinho recebido!";
    break;
  case Action::Toilet:
    if (s.stats[Urgency] < 30)
      return {false, "Ainda não preciso do banheiro."};
    s.stats[Urgency] = 0;
    s.stats[Education] += gain[0];
    s.bathStep = 4;
    msg = "Banheiro usado! Toque em Descarga.";
    break;
  case Action::Flush:
    if (s.bathStep != 4)
      return {false, "O vaso já está limpo."};
    s.bathStep = 0;
    msg = "Descarga! Tudo limpo.";
    break;
  case Action::Clean:
    if (!s.poop)
      return {false, "O ambiente está limpo."};
    s.poop = 0;
    s.stats[Hygiene] += gain[0];
    msg = "Acidente limpo. Obrigado!";
    break;
  case Action::Bath:
    s.bathStep = 1;
    msg = "Hora do banho: esfregue para fazer espuma.";
    break;
  case Action::Wash:
    if (s.bathStep != 1)
      return {false, "Comece o banho antes."};
    s.bathStep = 2;
    msg = "Espuma pronta! Agora enxágue.";
    break;
  case Action::Rinse:
    if (s.bathStep != 2)
      return {false, "Faça espuma primeiro."};
    s.bathStep = 3;
    msg = "Enxaguado! Vamos secar.";
    break;
  case Action::Dry:
    if (s.bathStep != 3)
      return {false, "Enxágue antes de secar."};
    s.bathStep = 0;
    s.stats[Hygiene] = gain[0];
    s.stats[Happy] += gain[1];
    msg = "Limpinho e sequinho!";
    break;
  case Action::Sleep:
    s.sleeping = 1;
    s.light = 0;
    if (!repeat) tendency(s, r, idx, 0, gain[0]);
    msg = "Boa noite! O sono recupera energia.";
    break;
  case Action::Nap:
    s.sleeping = 1;
    s.light = 0;
    tendency(s, r, idx, 0, gain[0]);
    msg = "Uma soneca de dez minutos. Acordo sozinho!";
    break;
  case Action::Wake:
    s.sleeping = 0;
    s.light = 1;
    msg = "Bom dia!";
    break;
  case Action::Medicine:
    if (!s.sick)
      return {false, "Sem indicação de remédio. Seu amigo está saudável."};
    s.stats[Health] += gain[0];
    s.stats[Energy] += gain[1];
    msg = "Tratamento aplicado. Descanso também ajuda.";
    break;
  case Action::Praise:
    if (s.stats[Urgency] > 70 || s.stats[Full] < 20 || s.stats[Energy] < 15 ||
        s.stats[Hygiene] < 20 || s.sick || s.poop)
      return {false, "Primeiro cuide da necessidade de verdade."};
    s.stats[Education] += gain[0];
    s.stats[Bond] += gain[1];
    tendency(s, r, idx, 2, 1);
    msg = "Muito bem! Elogio recebido.";
    break;
  case Action::Guide:
    if (s.stats[Urgency] > 70 || s.stats[Full] < 20 || s.stats[Energy] < 15 ||
        s.stats[Hygiene] < 20 || s.poop || s.sick) {
      s.stats[Bond] -= gain[1];
      msg = "Era uma necessidade real. Vamos cuidar dela.";
    } else {
      s.stats[Education] += gain[0];
      tendency(s, r, idx, 0, 1);
      msg = "Orientação com gentileza.";
    }
    break;
  case Action::Explore:
    s.stats[Energy] -= gain[0];
    s.stats[Happy] += gain[1];
    tendency(s, r, idx, 1, r.tendencyWeights[1]);
    msg = "Exploração no quintal! Uma pequena descoberta.";
    break;
  default:
    return {false, "Ação inválida."};
  }
  s.cooldown[idx] = now;
  s.careCount++;
  s.transaction++;
  s.action = a;
  s.actionLeft = a == Action::Nap ? r.napDurationSeconds * 1000 : a == Action::Explore ? r.exploreDurationMs : r.actionDurationMs;
  clamp(s);
  log(s, msg);
  return {true, msg};
}
int rps(int u, int p) { return u == p ? 0 : ((u - p + 3) % 3 == 1 ? 1 : -1); }
bool even(int a, int b) { return (a + b) % 2 == 0; }
static uint32_t roll(Match &m, unsigned n) {
  Rng rng{m.rng};
  auto v = rng.uniform(n);
  m.rng = rng.state;
  return v;
}
static void prepare(Match &m) {
  switch (m.kind) {
  case Game::Rps:
    m.opponent = roll(m, 3);
    break;
  case Game::Parity:
    m.opponent = roll(m, 6);
    m.parity = m.round % 2 ? roll(m, 2) : 0;
    break;
  case Game::Higher:
    m.opponent = 1 + roll(m, 9);
    break;
  case Game::Hide:
    m.opponent = roll(m, 6);
    break;
  default:
    m.opponent = roll(m, 2);
    break;
  }
  m.phase = 0;
}
bool startGame(State &s, Game game, uint8_t pairs) {
  if (s.stage == Stage::Egg || s.sleeping || s.dead || s.retired ||
      s.stats[Energy] < 10 || game == Game::None)
    return false;
  Match m{};
  m.kind = game;
  m.active = 1;
  m.first = m.second = 255;
  m.base = 1;
  m.id = ++s.transaction;
  Rng rng{s.gameRng};
  m.rng = rng.next();
  s.gameRng = rng.state;
  m.pairs = pairs == 4 || pairs == 8 ? pairs : 6;
  m.basket = 320;
  m.fruitY = -40;
  m.fruitX = 60 + roll(m, 520);
  m.length = 1;
  m.showing = 1;
  for (auto &n : m.sequence)
    n = roll(m, 4);
  for (int i = 0; i < m.pairs * 2; i++)
    m.cards[i] = i / 2;
  for (int i = m.pairs * 2 - 1; i > 0; i--)
    std::swap(m.cards[i], m.cards[roll(m, i + 1)]);
  prepare(m);
  s.match = m;
  log(s, "Uma nova brincadeira começou.");
  return true;
}
bool victory(const Match &m) {
  if (!m.finished)
    return false;
  if (m.kind == Game::Memory)
    return m.wins == m.pairs;
  if (m.kind == Game::Sequence)
    return m.wins >= 10;
  if (m.kind == Game::Fruit)
    return m.score > 0;
  return m.wins >= m.losses;
}
static void finish(State &s, const Rules &r) {
  auto &m = s.match;
  m.finished = 1;
  m.active = 0;
  if (!m.rewarded) {
    m.rewarded = 1;
    s.gameCount++;
    unsigned gameIndex = unsigned(m.kind) - 1;
    if (!s.gameRewardAt[gameIndex] ||
        s.age + 1 - s.gameRewardAt[gameIndex] >= uint64_t(r.rewardCooldownSeconds) * 1000) {
      s.gameRewardAt[gameIndex] = s.age + 1;
      s.stats[Happy] += r.gameHappy + (victory(m) ? r.gameVictory : 0);
      s.stats[Bond] += r.gameBond;
      s.stats[Energy] -= r.gameEnergy;
      bool varied = !(s.gamesSeen & (1u << gameIndex));
      tendency(s, r, 20 + gameIndex, 2, r.tendencyWeights[2] + (varied ? 2 : 0));
      s.gamesSeen |= 1u << gameIndex;
    }
    s.transaction++;
    clamp(s);
    log(s, "Brincadeira concluída! A participação também vale carinho.");
  }
}
Result play(State &s, int input, const Rules &r) {
  auto &m = s.match;
  if (!m.active || m.finished)
    return {false, "Partida encerrada."};
  if (m.phase)
    return {false, "Veja o resultado da rodada."};
  int outcome = 0;
  switch (m.kind) {
  case Game::Rps:
    if (input < 0 || input > 2)
      return {false, "Escolha pedra, papel ou tesoura."};
    outcome = rps(input, m.opponent);
    snprintf(m.result, sizeof m.result, "Você: %s • amigo: %s. %s",
             (const char *[]){"pedra", "papel", "tesoura"}[input],
             (const char *[]){"pedra", "papel", "tesoura"}[m.opponent],
             outcome > 0   ? "Você venceu!"
             : outcome < 0 ? "Seu amigo venceu!"
                           : "Empate, vamos repetir.");
    break;
  case Game::Parity:
    if (input < 0 || input > 5)
      return {false, "Escolha um número de 0 a 5."};
    outcome = int(even(input, m.opponent)) == int(m.parity == 0) ? 1 : -1;
    snprintf(m.result, sizeof m.result, "%d + %d = %d (%s). %s", input,
             m.opponent, input + m.opponent,
             even(input, m.opponent) ? "par" : "ímpar",
             outcome > 0 ? "Ponto seu!" : "Ponto do amigo!");
    break;
  case Game::Direction:
    if (input < 0 || input > 1)
      return {false, "Escolha um lado."};
    outcome = input == m.opponent ? 1 : -1;
    snprintf(m.result, sizeof m.result, "Olhei para a %s! %s",
             m.opponent ? "direita" : "esquerda",
             outcome > 0 ? "Acertou!" : "Vamos de novo!");
    break;
  case Game::Higher:
    if (input < 0 || input > 1)
      return {false, "Escolha maior ou menor."};
    outcome = m.opponent == m.base
                  ? 0
                  : ((m.opponent > m.base) == bool(input) ? 1 : -1);
    snprintf(m.result, sizeof m.result, "Anterior %d • próximo %d. %s", m.base,
             m.opponent,
             outcome > 0   ? "Acertou!"
             : outcome < 0 ? "Outra tentativa!"
                           : "Igual: empate.");
    m.base = m.opponent;
    break;
  case Game::Memory:
    if (input < 0 || input >= m.pairs * 2 || m.matched[input] ||
        m.first == input)
      return {false, "Escolha outra carta."};
    if (m.first == 255) {
      m.first = input;
      return {true, "Primeira carta."};
    }
    m.second = input;
    m.phase = 900;
    if (m.cards[m.first] == m.cards[m.second]) {
      m.matched[m.first] = m.matched[m.second] = 1;
      m.wins++;
      snprintf(m.result, sizeof m.result, "Um par! %u/%u", m.wins, m.pairs);
    } else {
      m.losses++;
      snprintf(m.result, sizeof m.result, "Observe e tente outra vez.");
    }
    if (m.wins == m.pairs)
      finish(s, r);
    return {true, m.result};
  case Game::Sequence:
    if (m.showing)
      return {false, "Observe a sequência antes de tocar."};
    if (input < 0 || input > 3)
      return {false, "Toque em uma das quatro notas."};
    if (input != m.sequence[m.input]) {
      m.losses++;
      snprintf(m.result, sizeof m.result,
               "Boa tentativa! Você chegou à sequência %u.", m.length);
      finish(s, r);
    } else if (++m.input == m.length) {
      m.wins++;
      if (m.length >= 10) {
        snprintf(m.result, sizeof m.result,
                 "Dez sequências! Que memória musical!");
        finish(s, r);
      } else {
        m.length++;
        m.input = 0;
        m.showing = 1;
        m.elapsed = 0;
      }
    }
    return {true, m.result};
  case Game::Fruit:
    m.basket = std::clamp(input, 24, 616);
    return {true, ""};
  case Game::Hide:
    if (input < 0 || input > 5)
      return {false, "Toque em um objeto."};
    outcome = input == m.opponent ? 1 : -1;
    snprintf(m.result, sizeof m.result, "Eu estava atrás de %s! %s",
             (const char *[]){"vaso", "árvore", "banco", "caixa", "arbusto",
                              "fonte"}[m.opponent],
             outcome > 0 ? "Você me encontrou!"
                         : "Você pode tentar na próxima rodada.");
    break;
  default:
    return {false, "Jogo inválido."};
  }
  m.choice = input;
  if (outcome > 0)
    m.wins++;
  else if (outcome < 0)
    m.losses++;
  if (outcome)
    m.round++;
  m.phase = 1500;
  if ((m.kind == Game::Rps && (m.wins == 3 || m.losses == 3)) ||
      (m.kind == Game::Hide && m.round == 3) ||
      (m.kind != Game::Rps && m.kind != Game::Hide && m.round == 5))
    finish(s, r);
  return {true, m.result};
}
void tickGame(State &s, uint32_t ms, const Rules &r) {
  auto &m = s.match;
  if (!m.active)
    return;
  if (m.phase) {
    if (ms >= m.phase) {
      m.phase = 0;
      if (m.kind == Game::Memory)
        m.first = m.second = 255;
      else
        prepare(m);
    } else
      m.phase -= ms;
    return;
  }
  if (m.kind == Game::Sequence && m.showing) {
    m.elapsed += ms;
    if (m.elapsed > m.length * 800u + 500) {
      m.showing = 0;
      m.input = 0;
      m.elapsed = 0;
    }
  }
  if (m.kind == Game::Fruit) {
    m.elapsed += ms;
    m.spawn += ms;
    uint32_t steps = m.spawn / 30;
    m.spawn %= 30;
    for (uint32_t i = 0; i < steps; i++) {
      m.fruitY += 3 + std::min<int>(3, m.elapsed / 10000);
      if (m.fruitY > 270 && m.fruitY < 295 &&
          std::abs(m.fruitX - m.basket) < 45) {
        m.score++;
        m.wins++;
        m.fruitY = 340;
      }
      if (m.fruitY >= 330) {
        m.fruitY = -20;
        m.fruitX = 40 + roll(m, 560);
      }
    }
    if (m.elapsed >= 30000) {
      snprintf(m.result, sizeof m.result,
               "Cesta cheia: %d frutas em 30 segundos!", int(m.score));
      finish(s, r);
    }
  }
}
void cancelGame(State &s) {
  if (!s.match.active)
    return;
  s.match.active = 0;
  s.match.finished = 0;
  log(s, "Brincadeira interrompida, sem penalidade.");
}
bool valid(const State &s, uint32_t profile) {
  if (s.format != 2 || s.profile != profile || !s.id || s.family >= 8 ||
      s.palette >= 4 || s.detail >= 2 || s.personality >= 3 ||
      unsigned(s.stage) > 5 || s.form > 7 || unsigned(s.mode) > 2 ||
      s.sleepHour > 23 || s.wakeHour > 23 || s.logHead >= 40 ||
      s.logCount > 40 || s.incubation > s.incubationTarget ||
      !memchr(s.name, 0, sizeof s.name))
    return false;
  for (float v : s.stats)
    if (!std::isfinite(v) || v < 0 || v > 100)
      return false;
  auto &m = s.match;
  if (unsigned(m.kind) > 8 || m.length > 32 || m.input > 32 || m.pairs > 8 ||
      (m.active && m.kind == Game::None) || (m.first != 255 && m.first >= 16) ||
      (m.second != 255 && m.second >= 16))
    return false;
  for (auto card : m.cards)
    if (card > 7)
      return false;
  for (auto note : m.sequence)
    if (note > 3)
      return false;
  return true;
}
} // namespace gotchi
