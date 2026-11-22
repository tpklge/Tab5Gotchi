#include "app.hpp"
#include "keyboard.hpp"
#include "board/board.hpp"
#include "board/power.hpp"
#include "cJSON.h"
#include "core/engine.hpp"
#include "driver/i2c_master.h"
#include "driver/usb_serial_jtag.h"
#include "esp_heap_caps.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "extensions/breeding.hpp"
#include "mbedtls/sha256.h"
#include "storage/store.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include <dirent.h>
#if CONFIG_GOTCHI_LAN
#include "extensions/lan.hpp"
#endif
#include <memory>
#include <map>
#include <set>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
using namespace gotchi;
namespace ui {
static constexpr const char *ROOT = "/sdcard/tamagotchi";
static const char *families[] = {"capim",  "brotito", "brasquito", "gotim",
                                 "nuvito", "parafim", "",          ""};
static const char *scenes[] = {"casa",     "cozinha", "banheiro", "banho",
                               "quarto",   "quintal", "cuidados", "jogos",
                               "bercario", "album"};
static lv_obj_t *nameField = nullptr, *keyboardLabel = nullptr;
static std::string nameDraft;
static int32_t nameCursor = LV_TEXTAREA_CURSOR_LAST;
static keyboard::Decoder keyboardDecoder;
static void nameKey(uint8_t modifiers, uint8_t usage);
static lv_obj_t *homeBars[4]{}, *homeLabels[4]{};
static bool keyboardOnline = false;
static unsigned keyboardEvents = 0;
static bool profileVacant = false;
static State state;
static Settings settings;
static Rules rules;
struct FormAsset {
  std::string file;
  int width = 96, height = 96, frames = 32, anchorX = 48, anchorY = 81;
  unsigned frameMs = 160;
  std::map<std::string, std::vector<unsigned>> animations;
};
static std::array<std::array<FormAsset, 9>, 8> formAssets;
static std::map<std::string, std::string> resourceHashes;
static std::set<std::string> verifiedAssets;
static unsigned checkpointSeconds = 60;
static unsigned currentPose = ~0u;
static int dayOpacity = -1;
static std::vector<std::string> historyFiles;
static size_t historyPage = 0;
static std::unique_ptr<State> historical;

static Store store(std::string(ROOT) + "/saves");
static std::array<std::array<std::string, 8>, 8> names;
static std::array<std::string, 8> familyIds;
static std::vector<Food> foods;
static std::vector<std::string> decorNames, gameNames, instructions;
static bool ready = false, dirty = true, fault = false;
static std::string faultText,
    page = "home", notice = "Bem-vindo! Vamos cuidar e brincar.", pending;
static int foodPage = 0, albumFamily = 0, selected = 0, selectedCode = -1,
           battery = -1;
static uint32_t frame = 0;
static int64_t last = 0, checkpoint = 0, metrics = 0;
static int64_t lastBattery = 0;
static uint8_t lastForm = 255, lastFamily = 255;
static bool lastEgg = false;
static lv_obj_t *screen, *header, *footer, *sceneImage, *petImage, *panel,
    *message, *status, *gameArea, *night, *foreground, *toyImage;
static lv_obj_t *clockLabel = nullptr, *batteryLabel = nullptr;
static lv_obj_t *batteryDetails = nullptr;
static lv_obj_t *wakeShield = nullptr;
static bool screenOff = false;
static int appliedBrightness = 70;
static lv_group_t *group;
static lv_font_t font, smallFont;
static std::vector<uint8_t> smallFontBytes;
static std::vector<uint8_t> fallingFruit, basketSprite;
static lv_image_dsc_t fallingDsc{}, basketDsc{};
static std::array<std::vector<uint8_t>, 6> hideArt;
static std::array<lv_image_dsc_t, 6> hideDsc;
static bool pageTransition = false;
static std::array<std::vector<uint8_t>, 8> careIcons;
static std::array<lv_image_dsc_t, 8> careDsc;
static int64_t voiceAt = 0;
static bool faultRestoreConfirm = false;
static int64_t sdProbeAt = 0;
static std::vector<uint8_t> fontBytes;
static uint32_t fontCount = 0;
static std::vector<uint8_t> sceneBytes, bank, toyBytes;
static lv_image_dsc_t sceneDsc{}, petDsc{}, toyDsc{};
static lv_obj_t *gameStatus;
static std::vector<lv_obj_t *> buttons, gameButtons;
static std::vector<int> buttonCodes;
static bool rebuildGame = true;
static int lastPhase = -1, lastFirst = -2, lastShow = -1;
static lv_obj_t *fruitObj = nullptr, *basketObj = nullptr;
static uint64_t nowSim = 0;
static unsigned renderedFrames = 0, uiTicks = 0;
static float measuredUiHz = 0;
static float measuredFps = 0;
static int64_t fpsStart = 0;
static lv_indev_t *testPointer = nullptr;
static lv_point_t testPoint{};
static int64_t testRelease = 0;
static unsigned memoryPairs = 6;
static std::string path(const std::string &);
static void applyBrightness() {
  int wanted = screenOff ? 0 : settings.brightness;
  if (wanted != appliedBrightness &&
      bsp_display_brightness_set(wanted) == ESP_OK)
    appliedBrightness = wanted;
}
static std::string batteryText(bool details = false) {
  int percent = power::batteryPercent(battery);
  if (!details)
    return percent < 0 ? "Bat.: --"
                      : "Bat.: ~" + std::to_string(percent) + "%" +
                            (percent <= 15 ? " !" : "");
  char text[160];
  if (percent < 0) {
    if (battery >= 0)
      snprintf(text, sizeof text,
               "Bateria: leitura indisponível (%.2f V).", battery / 1000.);
    else
      snprintf(text, sizeof text, "Bateria: leitura indisponível.");
  } else
    snprintf(text, sizeof text, "Bateria: ~%d%% • %.2f V%s", percent,
             battery / 1000., percent <= 15 ? " • bateria baixa" : "");
  return text;
}
static void refreshBatteryLabels() {
  if (batteryLabel)
    lv_label_set_text(batteryLabel, batteryText().c_str());
  if (batteryDetails)
    lv_label_set_text(batteryDetails, batteryText(true).c_str());
}
static void displayActivity() {
  lv_display_trigger_activity(board::display);
  if (screenOff) {
    screenOff = false;
    if (wakeShield) {
      lv_obj_delete_async(wakeShield);
      wakeShield = nullptr;
    }
    // Keep existing widgets (and any unfinished text entry) unless the game
    // itself changed while hidden and has already requested a render.
    refreshBatteryLabels();
    applyBrightness();
  }
}
static std::string timeoutText() {
  unsigned seconds = settings.screenTimeoutSeconds;
  if (!seconds)
    return "Nunca";
  return std::to_string(seconds / 60) + " min " +
         std::to_string(seconds % 60) + " s";
}
static void persistSettings() {
  board::lockIO();
  bool ok = saveSettings(path("saves/settings"), settings);
  board::unlockIO();
  notice = ok ? "Ajustes salvos." : "Falha ao salvar ajustes no microSD.";
  dirty = true;
}
#if CONFIG_GOTCHI_LAN
static uint32_t ownCode = 0, remoteCode = 0;
static std::string lanSsid, lanPassword, peerIP;
static int lanField = 0;
static visit::Packet pendingVisit{};
static bool hasVisit = false, visitorVisible = false;
static int64_t visitUntil = 0, pendingUntil = 0;
static bool save();
static void acceptVisit() {
  if (!hasVisit)
    return;
  std::string receipt =
      path("album/visit-" + std::to_string(pendingVisit.request) + ".json");
  struct stat st{};
  if (stat(receipt.c_str(), &st) == 0) {
    notice = "Esta visita já foi recebida.";
    hasVisit = false;
    return;
  }
  if (pendingVisit.kind == 2)
    state.decorations |= 1u << pendingVisit.decor;
  log(state, pendingVisit.kind == 2 ? "Um presente de uma visita local chegou!"
                                    : "Uma visita local veio brincar.");
  if (!save())
    return;
  board::lockIO();
  FILE *f = fopen(receipt.c_str(), "wb");
  if (f) {
    fprintf(f, "{\"request\":\"%llu\",\"profile\":%u,\"visitor\":\"%llu\"}\n",
            pendingVisit.request, unsigned(state.profile),
            pendingVisit.individual);
    fflush(f);
    fsync(fileno(f));
    fclose(f);
  }
  board::unlockIO();
  visitorVisible = true;
  visitUntil = esp_timer_get_time() + 30000000;
  hasVisit = false;
  notice = "Visita recebida! O amigo fica um pouquinho por aqui.";
}
#endif
static std::array<std::vector<uint8_t>, 4> foodIcons;
static std::array<lv_image_dsc_t, 4> foodIconDsc;
static void render();
static void renderGame();
static void click(int);
static bool save();
static bool portrait(const char *);
static void setPage(const std::string &);
static std::string path(const std::string &p) {
  return std::string(ROOT) + "/" + p;
}
static std::vector<uint8_t> readFile(const std::string &p,
                                     size_t maximum = 1024 * 1024) {
  board::lockIO();
  FILE *f = fopen(p.c_str(), "rb");
  std::vector<uint8_t> data;
  if (f) {
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    rewind(f);
    if (n > 0 && size_t(n) <= maximum) {
      data.resize(n);
      if (fread(data.data(), 1, n, f) != size_t(n))
        data.clear();
    }
    fclose(f);
  }
  board::unlockIO();
  auto expected = resourceHashes.find(p);
  if (expected != resourceHashes.end()) {
    uint8_t digest[32];
    mbedtls_sha256(data.data(), data.size(), digest, 0);
    char hex[65];
    for (int i = 0; i < 32; ++i) snprintf(hex + i * 2, 3, "%02x", digest[i]);
    if (data.empty() || expected->second != hex) {
      notice = "Recurso inválido: " + p.substr(strlen(ROOT) + 1);
      faultText = notice;
      data.clear();
    } else verifiedAssets.insert(p);
  }
  return data;
}
static bool validateAudioAsset(const std::string &p) {
  auto expected = resourceHashes.find(p);
  if (expected == resourceHashes.end()) return false;
  FILE *f = fopen(p.c_str(), "rb");
  if (!f) return false;
  mbedtls_sha256_context ctx; mbedtls_sha256_init(&ctx); mbedtls_sha256_starts(&ctx, 0);
  uint8_t block[4096], digest[32]; size_t n;
  while ((n = fread(block, 1, sizeof block, f))) mbedtls_sha256_update(&ctx, block, n);
  bool good = !ferror(f); fclose(f);
  mbedtls_sha256_finish(&ctx, digest); mbedtls_sha256_free(&ctx);
  char hex[65]; for (int i = 0; i < 32; ++i) snprintf(hex + i * 2, 3, "%02x", digest[i]);
  return good && expected->second == hex;
}
static cJSON *json(const std::string &p) {
  auto data = readFile(p);
  data.push_back(0);
  return cJSON_Parse((char *)data.data());
}
static const char *str(cJSON *j, const char *k) {
  auto *v = cJSON_GetObjectItem(j, k);
  return cJSON_IsString(v) ? v->valuestring : "";
}
static int number(cJSON *j, const char *k, int fallback) {
  auto *v = cJSON_GetObjectItem(j, k);
  return cJSON_IsNumber(v) ? v->valueint : fallback;
}
static bool glyph(const lv_font_t *which, lv_font_glyph_dsc_t *d, uint32_t c,
                  uint32_t) {
  const bool small = which == &smallFont &&
                     smallFontBytes.size() == 16 + fontCount * (8 + 28 * 34);
  auto *table =
      (const uint32_t *)((small ? smallFontBytes : fontBytes).data() + 16);
  for (unsigned i = 0; i < fontCount; i++)
    if (table[i * 2] == c) {
      d->adv_w = table[i * 2 + 1];
      d->box_w = small ? 28 : 36;
      d->box_h = small ? 34 : 40;
      d->ofs_x = 0;
      d->ofs_y = -8;
      d->stride = small ? 28 : 36;
      d->format = LV_FONT_GLYPH_FORMAT_A8;
      d->gid.index = i;
      return true;
    }
  return false;
}
static const void *bitmap(lv_font_glyph_dsc_t *d, lv_draw_buf_t *) {
  if (d->resolved_font == &smallFont &&
      smallFontBytes.size() == 16 + fontCount * (8 + 28 * 34))
    return smallFontBytes.data() + 16 + fontCount * 8 + d->gid.index * 28 * 34;
  return fontBytes.data() + 16 + fontCount * 8 + d->gid.index * 36 * 40;
}
static bool content() {
  resourceHashes.clear();
  verifiedAssets.clear();
  foods.clear(); decorNames.clear(); gameNames.clear(); instructions.clear();
  auto *j = json(path("manifest.json"));
  if (!j) {
    faultText =
        "Pacote ausente: copie a pasta tamagotchi para a raiz do microSD.";
    return false;
  }
  bool ok = number(j, "schema_version", 0) == 1 &&
            number(j, "engine_api_version", 0) == 1;
  if (!ok) {
    cJSON_Delete(j);
    faultText = "Pacote incompatível com este motor.";
    return false;
  }
  auto *catalog = cJSON_GetObjectItem(j, "families");
  rules.familyCount = cJSON_GetArraySize(catalog);
  if (rules.familyCount < 1 || rules.familyCount > 8) {
    cJSON_Delete(j);
    return false;
  }
  for (int i = 0; i < rules.familyCount; i++) {
    auto *v = cJSON_GetArrayItem(catalog, i);
    if (!cJSON_IsString(v)) {
      cJSON_Delete(j);
      return false;
    }
    familyIds[i] = v->valuestring;
    families[i] = familyIds[i].c_str();
  }
  auto *files = cJSON_GetObjectItem(j, "files");
  cJSON *entry;
  int count = 0;
  cJSON_ArrayForEach(entry, files) {
    std::string file = str(entry, "path");
    if (file.empty() || file[0] == '/' ||
        file.find("..") != std::string::npos) {
      ok = false;
      break;
    }
    std::string digest = str(entry, "sha256");
    if (digest.size() != 64) { ok = false; break; }
    resourceHashes[path(file)] = digest;
    struct stat st{};
    if (stat(path(file).c_str(), &st) ||
        st.st_size != number(entry, "bytes", -1)) {
      faultText = "Recurso ausente ou tamanho inválido: " + file;
      ok = false;
      break;
    }
    count++;
    if (count > 512) {
      ok = false;
      break;
    }
  }
  cJSON_Delete(j);
  if (!ok || count < 100)
    return false;
  fontBytes = readFile(path("ui/font.bin"));
  if (fontBytes.size() < 16)
    return false;
  uint32_t head[4];
  memcpy(head, fontBytes.data(), 16);
  fontCount = head[1];
  if (fontCount < 1 || fontCount > 512)
    return false;
  if (head[0] != 0x31464742 || head[2] != 36 || head[3] != 40 ||
      fontBytes.size() != 16 + fontCount * (8 + 36 * 40))
    return false;
  font.get_glyph_dsc = glyph;
  font.get_glyph_bitmap = bitmap;
  font.line_height = 40;
  font.base_line = 8;
  font.static_bitmap = 1;
  font.fallback = LV_FONT_DEFAULT;
  smallFontBytes = readFile(path("ui/font-small.bin"));
  if (smallFontBytes.size() == 16 + fontCount * (8 + 28 * 34)) {
    smallFont = font;
    smallFont.line_height = 34;
  } else {
    smallFont = font;
    smallFontBytes = fontBytes;
    smallFont.get_glyph_dsc = font.get_glyph_dsc;
  }
  for (int f = 0; f < rules.familyCount; f++) {
    j = json(path(std::string("pets/") + families[f] + "/family.json"));
    if (!j)
      return false;
    auto *forms = cJSON_GetObjectItem(j, "forms");
    if (cJSON_GetArraySize(forms) != 9) { cJSON_Delete(j); return false; }
    for (int i = 0; i < 9; i++) {
      auto *v = cJSON_GetArrayItem(forms, i);
      if (i < 8) names[f][i] = str(v, "name");
      auto &asset = formAssets[f][i];
      asset.file = str(v, "file");
      asset.width = number(v, "width", 0); asset.height = number(v, "height", 0);
      asset.frames = number(v, "frames", 0);
      asset.frameMs = number(v, "frame_ms", 160);
      auto *anchor = cJSON_GetObjectItem(v, "anchor");
      asset.anchorX = cJSON_GetNumberValue(cJSON_GetArrayItem(anchor, 0));
      asset.anchorY = cJSON_GetNumberValue(cJSON_GetArrayItem(anchor, 1));
      if (asset.width < 1 || asset.width > 192 || asset.height < 1 || asset.height > 192 ||
          asset.frames < 1 || asset.frames > 128 || size_t(asset.width) * asset.height * asset.frames * 3 > 4 * 1024 * 1024 || asset.frameMs < 40 || asset.frameMs > 2000 ||
          asset.anchorX < 0 || asset.anchorX > asset.width || asset.anchorY < 0 || asset.anchorY > asset.height ||
          !resourceHashes.count(path(asset.file)) || str(v, "format") != std::string("RGB565A8_PLANAR")) {
        cJSON_Delete(j); return false;
      }
      asset.animations.clear();
      auto *animationObject = cJSON_GetObjectItem(v, "animations");
      if (!cJSON_IsObject(animationObject)) { cJSON_Delete(j); return false; }
      cJSON *anim;
      cJSON_ArrayForEach(anim, animationObject) {
        auto &seq = asset.animations[anim->string];
        cJSON *pose;
        cJSON_ArrayForEach(pose, anim) {
          double index = cJSON_GetNumberValue(pose);
          if (!cJSON_IsNumber(pose) || !std::isfinite(index) || index < 0 || index >= asset.frames || index != int(index)) { cJSON_Delete(j); return false; }
          seq.push_back(unsigned(index));
        }
        if (seq.empty() || seq.size() > 128) { cJSON_Delete(j); return false; }
      }
      if (!asset.animations.count("idle")) { cJSON_Delete(j); return false; }
    }
    cJSON_Delete(j);
  }
  j = json(path("data/foods.json"));
  cJSON_ArrayForEach(entry, j) {
    Food f{};
    snprintf(f.id, sizeof f.id, "%s", str(entry, "id"));
    snprintf(f.name, sizeof f.name, "%s", str(entry, "name"));
    snprintf(f.shortName, sizeof f.shortName, "%.31s",
             strlen(str(entry, "short_name")) ? str(entry, "short_name")
                                              : f.name);
    int kind = number(entry, "kind", 0);
    if (kind < 0 || kind > 2) {
      cJSON_Delete(j);
      return false;
    }
    f.action = Action(int(Action::Meal) + kind);
    f.fullness = number(entry, "fullness", 0);
    f.happiness = number(entry, "happiness", 0);
    f.health = number(entry, "health", 0);
    f.icon = number(entry, "icon", 0);
    foods.push_back(f);
  }
  cJSON_Delete(j);
  if (foods.size() != 12)
    return false;
  j = json(path("data/decor.json"));
  cJSON_ArrayForEach(entry, j) decorNames.emplace_back(str(entry, "name"));
  cJSON_Delete(j);
  j = json(path("data/games.json"));
  cJSON_ArrayForEach(entry, j) {
    gameNames.emplace_back(str(entry, "name"));
    instructions.emplace_back(str(entry, "instruction"));
  }
  cJSON_Delete(j);
  j = json(path("data/rules.json"));
  if (!j)
    return false;
  rules.incubationMin = number(j, "incubation_min", 180);
  rules.incubationMax = number(j, "incubation_max", 300);
  rules.baby = number(j, "baby_ms", 7200000);
  rules.child = number(j, "child_ms", 86400000);
  rules.teen = number(j, "teen_ms", 259200000);
  rules.adult = number(j, "adult_ms", 1209600000);
  rules.tolerance = number(j, "offline_tolerance_seconds", 7200);
  rules.offlineLimit = number(j, "offline_limit_seconds", 43200);
  rules.classicRiskHours = number(j, "classic_risk_hours", 48);
  checkpointSeconds = number(j, "checkpoint_seconds", 60);
  if (checkpointSeconds < 10 || checkpointSeconds > 3600) { cJSON_Delete(j); return false; }
  auto bounded = [&](const char *key, unsigned fallback, unsigned lo, unsigned hi) {
    double value = number(j, key, fallback);
    if (value < lo || value > hi) ok = false;
    return unsigned(std::clamp(value, double(lo), double(hi)));
  };
  rules.actionCooldownSeconds = bounded("action_cooldown_seconds", 60, 1, 3600);
  rules.educationCooldownSeconds = bounded("education_cooldown_seconds", 300, 1, 7200);
  rules.rewardCooldownSeconds = bounded("reward_cooldown_seconds", 300, 1, 7200);
  rules.actionDurationMs = bounded("action_duration_ms", 1300, 100, 10000);
  rules.exploreDurationMs = bounded("explore_duration_ms", 4000, 1000, 60000);
  rules.napDurationSeconds = bounded("nap_duration_seconds", 600, 30, 7200);
  rules.activityCap = bounded("evolution_activity_cap", 40, 1, 1000);
  auto *gains = cJSON_GetObjectItem(j, "action_gains");
  if (gains) {
    if (cJSON_GetArraySize(gains) != 20) ok = false;
    for (int i = 0; i < 20; ++i) for (int k = 0; k < 3; ++k) {
      auto *v = cJSON_GetArrayItem(cJSON_GetArrayItem(gains, i), k);
      if (!cJSON_IsNumber(v) || v->valuedouble < 0 || v->valuedouble > 100) ok = false;
      else rules.gains[i][k] = v->valuedouble;
    }
  }
  auto *weights = cJSON_GetObjectItem(j, "evolution_weights");
  if (weights) for (int i = 0; i < 3; ++i) {
    auto *v = cJSON_GetArrayItem(weights, i);
    if (!cJSON_IsNumber(v) || v->valuedouble < 1 || v->valuedouble > 20) ok = false;
    else rules.tendencyWeights[i] = v->valuedouble;
  }
  auto *socialFloor = cJSON_GetObjectItem(j, "social_minimum_scale");
  rules.socialMinimumScale = cJSON_IsNumber(socialFloor) ? socialFloor->valuedouble : .25;
  if (rules.socialMinimumScale < 0 || rules.socialMinimumScale > 1) ok = false;
  auto *reward = cJSON_GetObjectItem(j, "game_reward");
  float *rewardFields[] = {&rules.gameHappy, &rules.gameVictory, &rules.gameBond, &rules.gameEnergy};
  if (reward) for (int i = 0; i < 4; ++i) {
    auto *v = cJSON_GetArrayItem(reward, i);
    if (!cJSON_IsNumber(v) || v->valuedouble < 0 || v->valuedouble > 20) ok = false;
    else *rewardFields[i] = v->valuedouble;
  }
  auto *decay = cJSON_GetObjectItem(j, "decay_per_10_minutes");
  for (int i = 0; i < 5; i++) {
    auto *v = cJSON_GetArrayItem(decay, i);
    if (cJSON_IsNumber(v) && v->valuedouble >= 0)
      rules.decay[i] = v->valuedouble;
  }
  cJSON_Delete(j);
  return ok && gameNames.size() == 8 && decorNames.size() == 12 &&
         rules.incubationMin > 0 &&
         rules.incubationMax >= rules.incubationMin &&
         rules.incubationMax <= 3600 && rules.baby > 0 &&
         rules.child > rules.baby && rules.teen > rules.child &&
         rules.adult > rules.teen && rules.offlineLimit > 0 &&
         rules.offlineLimit <= 604800 &&
         rules.tolerance <= rules.offlineLimit &&
         rules.classicRiskHours >= 12 && rules.classicRiskHours <= 168;
}
static lv_color_t color(uint32_t hex) {
  if (settings.retro) {
    uint8_t g =
        (((hex >> 16) & 255) * 3 + ((hex >> 8) & 255) * 6 + (hex & 255)) / 10;
    return lv_color_make(g * 3 / 4, g, g * 3 / 5);
  }
  unsigned gray =
      (((hex >> 16) & 255) * 3 + ((hex >> 8) & 255) * 6 + (hex & 255)) / 10;
  return lv_color_make(gray, gray, gray);
}
static lv_obj_t *box(lv_obj_t *parent, int x, int y, int w, int h,
                     uint32_t bg) {
  auto *o = lv_obj_create(parent);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, color(bg), 0);
  lv_obj_set_style_border_width(o, 0, 0);
  lv_obj_set_style_radius(o, 20, 0);
  lv_obj_set_style_pad_all(o, 0, 0);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_SCROLLABLE);
  return o;
}
static lv_obj_t *label(lv_obj_t *parent, const std::string &t, int x, int y,
                       int w = 0) {
  auto *o = lv_label_create(parent);
  lv_label_set_text(o, t.c_str());
  lv_obj_set_pos(o, x, y);
  if (w)
    lv_obj_set_width(o, w);
  lv_obj_set_style_text_color(o, color(0x34384e), 0);
  lv_obj_set_style_text_align(o, LV_TEXT_ALIGN_LEFT, 0);
  return o;
}
static lv_obj_t *button(lv_obj_t *parent, const std::string &t, int code, int x,
                        int y, int w, int h, uint32_t bg = 0xe0e0e0) {
  auto *b = lv_button_create(parent);
  lv_obj_set_pos(b, x, y);
  lv_obj_set_size(b, w, h);
  lv_obj_set_style_bg_color(b, color(bg), 0);
  lv_obj_set_style_radius(b, 16, 0);
  lv_obj_set_style_shadow_width(b, 0, 0);
  lv_obj_set_style_pad_all(b, 8, 0);
  lv_obj_set_style_bg_color(b, color(0xc9c9c9), LV_STATE_PRESSED);
  lv_obj_set_style_border_color(b, color(0x947c68), 0);
  lv_obj_set_style_border_width(b, 1, 0);
  lv_obj_set_style_border_color(b, color(0xb9b9b9), 0);
  lv_obj_set_style_border_width(b, 3, LV_STATE_FOCUSED);
  auto *l = label(b, t, 0, 0, w - 20);
  if (ready && w < 225)
    lv_obj_set_style_text_font(l, &smallFont, 0);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(l);
  lv_obj_add_event_cb(
      b,
      [](lv_event_t *e) {
        int code = int(intptr_t(lv_event_get_user_data(e)));
        if (code >= 0 && code < 950)
          selectedCode = code;
        click(code);
      },
      LV_EVENT_CLICKED, (void *)intptr_t(code));
  if (code >= 0) {
    buttons.push_back(b);
    buttonCodes.push_back(code);
    lv_group_add_obj(group, b);
  }
  return b;
}
static void adjustment(lv_obj_t *parent, const std::string &title,
                       const std::string &value, int minusCode, int y,
                       bool atMinimum, bool atMaximum) {
  label(parent, title + ": " + value, 24, y + 18, 600);
  auto *minus = button(parent, "-", minusCode, 644, y, 96, 76);
  auto *plus = button(parent, "+", minusCode + 1, 758, y, 96, 76);
  if (atMinimum)
    lv_obj_add_state(minus, LV_STATE_DISABLED);
  if (atMaximum)
    lv_obj_add_state(plus, LV_STATE_DISABLED);
}
static void focusSelected() {
  if (!settings.abc || buttons.empty())
    return;
  int found = -1;
  for (size_t i = 0; i < buttons.size(); i++)
    if (buttonCodes[i] == selectedCode && buttonCodes[i] < 950 &&
        !lv_obj_has_state(buttons[i], LV_STATE_DISABLED)) {
      found = i;
      break;
    }
  if (found < 0 && page == "match")
    for (auto *b : gameButtons)
      if (!lv_obj_has_state(b, LV_STATE_DISABLED)) {
        for (size_t i = 0; i < buttons.size(); i++)
          if (buttons[i] == b) {
            found = i;
            break;
          }
        if (found >= 0)
          break;
      }
  if (found < 0)
    for (size_t i = 0; i < buttons.size(); i++)
      if (buttonCodes[i] < 950 &&
          !lv_obj_has_state(buttons[i], LV_STATE_DISABLED)) {
        found = i;
        break;
      }
  if (found >= 0) {
    selected = found;
    selectedCode = buttonCodes[found];
    lv_group_focus_obj(buttons[found]);
  }
}
static void sfx(const std::string &name) {
  board::sound(path("audio/sfx/" + name + ".wav"));
}
static bool save() {
  if (profileVacant)
    return true;
  int64_t calendar = board::wall();
  if (calendar > 1704067200) {
    if (!state.createdWall && state.stage == Stage::Egg && state.sequence == 0) state.createdWall = calendar;
    if (!state.birthWall && state.stage == Stage::Baby && state.age < 60000) state.birthWall = calendar;
    if ((state.retired || state.dead) && !state.endedWall) state.endedWall = calendar;
    unsigned stage = unsigned(state.stage);
    if (!state.milestoneWall[stage] && (state.createdWall || state.birthWall)) state.milestoneWall[stage] = calendar;
  }
  state.lastWall = board::wall() > 1704067200 ? board::wall() : state.lastWall;
  board::lockIO();
  bool ok = store.save(state);
  board::unlockIO();
  if (!ok) {
    fault = true;
    faultText = "Falha ao salvar. Simulação pausada; progresso em memória "
                "preservado. Confira o SD e reinicie.";
    dirty = true;
    board::stopAudio();
  }
  return ok;
}
static void loadProfile(unsigned profile) {
  profileVacant = false;
  state = {};
  board::lockIO();
  auto result = store.load(profile, state);
  board::unlockIO();
  if (result == Load::Corrupt) {
    fault = true;
    faultText = "Os dois saves deste perfil estão inválidos. Evidências "
                "preservadas. Restaure um backup pela tela de recuperação.";
    return;
  }
  if (result == Load::Missing &&
      access(path("saves/empty" + std::to_string(profile)).c_str(), F_OK) ==
          0) {
    profileVacant = true;
  } else if (result == Load::Missing) {
    state = newborn(profile, esp_random(),
                    (uint64_t(esp_random()) << 32) | esp_random(), rules);
    if (!save())
      return;
  } else {
    if (state.family >= rules.familyCount) {
      fault = true;
      faultText = "A família deste save não está instalada.";
      return;
    }
    resume(state, board::wall(), rules);
    save();
  }
  settings.profile = profile;
  lastForm = 255;
  last = esp_timer_get_time();
  checkpoint = last;
  page = profileVacant ? "profiles" : state.match.active ? "match" : "home";
  rebuildGame = true;
}
static int sceneIndex() {
  if (state.stage == Stage::Egg)
    return 8;
  if (page == "food")
    return 1;
  if (page == "hygiene")
    return state.bathStep > 0 && state.bathStep < 4 ? 3 : 2;
  if (page == "sleep" || state.sleeping)
    return 4;
  if (page == "health")
    return 6;
  if (page == "games" || page == "match")
    return 7;
  if (page == "album" || page == "diary" || page == "history" || page == "life")
    return 9;
  if (state.action == Action::Explore)
    return 5;
  return 0;
}
static void imageDescriptor(lv_image_dsc_t &d, int w, int h,
                            const uint8_t *data, bool alpha) {
  d = {};
  d.header.magic = LV_IMAGE_HEADER_MAGIC;
  d.header.cf = alpha ? LV_COLOR_FORMAT_RGB565A8 : LV_COLOR_FORMAT_RGB565;
  d.header.w = w;
  d.header.h = h;
  d.header.stride = w * 2;
  d.data_size = w * h * (alpha ? 3 : 2);
  d.data = data;
}

struct Thumb {
  int key = -1;
  std::vector<uint8_t> data;
  lv_image_dsc_t desc{};
};
static std::array<Thumb, 8> thumbs;
static void thumb(lv_obj_t *parent, int family, int form, int x, int y,
                  int scale = 256, bool silhouette = false, const State *appearance = nullptr) {
  int key = family * 9 + form + (silhouette ? 128 : 0) + (appearance ? (appearance->palette + appearance->detail * 4 + (appearance->adult & 3) * 8 + 1) * 256 : 0);
  Thumb *t = nullptr;
  for (auto &v : thumbs)
    if (v.key == key)
      t = &v;
  for (auto &v : thumbs)
    if (!t && v.key < 0)
      t = &v;
  if (!t)
    return;
  if (t->key != key || t->data.empty()) {
    t->key = key;
    const auto &asset = formAssets[family][form];
    size_t bytes = asset.width * asset.height * 3;
    if (!verifiedAssets.count(path(asset.file))) {
      auto full = readFile(path(asset.file), 4 * 1024 * 1024);
      if (full.size() != bytes * asset.frames) { t->data.clear(); return; }
      t->data.assign(full.begin(), full.begin() + bytes);
    } else {
      t->data.resize(bytes);
      board::lockIO(); FILE *f = fopen(path(asset.file).c_str(), "rb");
      bool good = f && fread(t->data.data(), 1, bytes, f) == bytes;
      if (f) fclose(f);
      board::unlockIO();
      if (!good) { t->data.clear(); return; }
    }
    if (appearance) {
      auto *rgb = reinterpret_cast<uint16_t *>(t->data.data());
      static const int tint[4][3] = {{0,0,0},{17,4,-3},{-12,12,10},{9,-8,18}};
      for (int i = 0; i < asset.width * asset.height; ++i) {
        int r = (rgb[i] >> 11) * 8, g = ((rgb[i] >> 5) & 63) * 4, b = (rgb[i] & 31) * 8;
        r = std::clamp(r + tint[appearance->palette][0], 0, 255);
        g = std::clamp(g + tint[appearance->palette][1], 0, 255);
        b = std::clamp(b + tint[appearance->palette][2], 0, 255);
        rgb[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      }
      if (appearance->detail && asset.width >= 30 && asset.height >= 62)
        for (int y = 55; y < 62; ++y) for (int x = 27; x < 30; ++x) {
          rgb[y * asset.width + x] = 0xff39;
          t->data[asset.width * asset.height * 2 + y * asset.width + x] = 255;
        }
      if (form == 7 && asset.width == 96 && asset.height == 96) {
        auto overlay = readFile(path(std::string("pets/") + families[family] + "/elder" + std::to_string(appearance->adult & 3) + ".rgb"));
        if (overlay.size() == 27648) for (int i = 0; i < 9216; ++i) if (overlay[18432 + i]) {
          t->data[i * 2] = overlay[i * 2]; t->data[i * 2 + 1] = overlay[i * 2 + 1]; t->data[18432 + i] = 255;
        }
      }
    }
    if (silhouette) {
      auto *rgb = reinterpret_cast<uint16_t *>(t->data.data());
      std::fill(rgb, rgb + asset.width * asset.height, uint16_t(0x632c));
    }
    imageDescriptor(t->desc, asset.width, asset.height, t->data.data(), true);
  }
  if (t->data.empty()) { label(parent, "Imagem indisponível", x, y, 180); return; }
  auto *o = lv_image_create(parent);
  lv_image_set_src(o, &t->desc);
  lv_image_set_scale(o, scale);
  lv_image_set_antialias(o, false);
  lv_image_set_pivot(o, 0, 0);
  lv_obj_set_pos(o, x, y);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
}

static bool portrait(const char *filename) {
  if (bank.empty())
    return false;
  auto *card = box(screen, 0, 0, 320, 360, 0xe9ddbd);
  auto *img = lv_image_create(card);
  lv_image_set_src(img, &petDsc);
  lv_image_set_scale(img, 768);
  lv_image_set_pivot(img, 0, 0);
  lv_image_set_antialias(img, false);
  lv_obj_set_pos(img, 16, 0);
  label(card, state.name, 10, 302, 300);
  bool ok = board::screenshot(filename, card);
  lv_obj_delete(card);
  return ok;
}
static void loadPet() {
  bool egg = state.stage == Stage::Egg;
  auto visibleForm = presentationForm(state);
  if (lastForm == visibleForm && lastFamily == state.family && lastEgg == egg)
    return;
  lastForm = visibleForm;
  lastFamily = state.family;
  lastEgg = egg;
  currentPose = ~0u;
  const auto &asset = formAssets[state.family][egg ? 8 : visibleForm];
  bank = readFile(path(asset.file), 4 * 1024 * 1024);
  const int pixels = asset.width * asset.height;
  if (bank.size() != size_t(asset.frames * pixels * 3)) {
    notice = "Sprite ausente: " + std::string(families[state.family]);
    bank.clear();
    return;
  }
  if (visibleForm == 7 && !egg && pixels == 9216) {
    auto overlay =
        readFile(path(std::string("pets/") + families[state.family] + "/elder" +
                      std::to_string(state.adult & 3) + ".rgb"));
    if (overlay.size() == 27648) {
      for (int frameIndex = 0; frameIndex < 32; frameIndex++)
        for (int i = 0; i < 9216; i++)
          if (overlay[18432 + i]) {
            bank[frameIndex * 27648 + i * 2] = overlay[i * 2];
            bank[frameIndex * 27648 + i * 2 + 1] = overlay[i * 2 + 1];
            bank[frameIndex * 27648 + 18432 + i] = 255;
          }
    }
  }
  // Four palettes and two details use one shared geometry bank.
  for (int frameIndex = 0; frameIndex < asset.frames; frameIndex++) {
    auto *p = (uint16_t *)(bank.data() + frameIndex * pixels * 3);
    if (settings.retro || state.palette) {
      for (int i = 0; i < pixels; i++) {
        unsigned r = (p[i] >> 11) * 8, g = ((p[i] >> 5) & 63) * 4,
                 b = (p[i] & 31) * 8;
        if (settings.retro) {
          unsigned v = (r * 3 + g * 6 + b) / 10;
          r = v * 3 / 4;
          g = v;
          b = v * 3 / 5;
        } else {
          static const int tints[4][3] = {
              {0, 0, 0}, {17, 4, -3}, {-12, 12, 10}, {9, -8, 18}};
          r = std::clamp(int(r) + tints[state.palette][0], 0, 255);
          g = std::clamp(int(g) + tints[state.palette][1], 0, 255);
          b = std::clamp(int(b) + tints[state.palette][2], 0, 255);
        }
        p[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
      }
    }
    if (state.detail && asset.width >= 30 && asset.height >= 62) {
      for (int y = 55; y < 62; y++)
        for (int x = 27; x < 30; x++) {
          p[y * asset.width + x] = 0xff39;
          bank[frameIndex * pixels * 3 + pixels * 2 + y * asset.width + x] = 255;
        }
    }
  }
}
static const FormAsset &activeAsset() {
  return formAssets[state.family][state.stage == Stage::Egg ? 8 : presentationForm(state)];
}
static int animationFrame() {
  const auto &a = activeAsset();
  std::string name = "idle";
  if (page == "match" && state.match.kind == Game::Direction && state.match.phase) {
    auto look = a.animations.find("olhar");
    if (look != a.animations.end() && look->second.size() >= 3)
      return look->second[state.match.opponent ? 1 : 2];
  }
  if (state.stage == Stage::Egg)
    name = state.incubation + 3000 >= state.incubationTarget ? "eclosao" :
           state.incubation + 20000 >= state.incubationTarget ? "rachadura" :
           frame % 40 < 8 ? "balancar" : "idle";
  else if (state.pendingEvolution) name = "evolucao";
  else if (state.dead) name = "triste";
  else if (page == "match" && state.match.finished) name = victory(state.match) ? "vitoria" : "derrota";
  else if (page == "match" && state.match.phase) name = "olhar";
  else if (state.sleeping) name = "dormir";
  else if (state.sick) name = "doente";
  else if (state.bathStep > 0 && state.bathStep < 4) name = "banho";
  else if (state.stats[Urgency] > 75) name = "urgencia";
  else if (state.actionLeft) {
    switch (state.action) {
    case Action::Meal: case Action::Fruit: case Action::Treat: name = "comer"; break;
    case Action::Water: name = "beber"; break;
    case Action::Pet: case Action::Lullaby: name = "carinho"; break;
    case Action::Toilet: name = "banheiro"; break;
    case Action::Explore: name = "andar"; break;
    default: name = "feliz"; break;
    }
  } else if (state.stats[Full] < 20) name = "fome";
  else if (state.stats[Energy] < 20) name = "cansaco";
  else if (frame % 100 < 6) name = "olhar";
  else if (frame % 40 < 3) name = "piscar";
  auto it = a.animations.find(name);
  if (it == a.animations.end()) it = a.animations.find("idle");
  return it->second[(esp_timer_get_time() / (a.frameMs * 1000ULL)) % it->second.size()];
}
static void setPage(const std::string &p) {
  if (profileVacant && p != "profiles" && p != "settings" && p != "audio" &&
      p != "display" && p != "more" && p != "confirm" && p != "extras") {
    page = "profiles";
    notice = "Escolha um perfil ou crie um ovo para continuar.";
    dirty = true;
    return;
  }
  if (page == "match" && p != "match" && state.match.active) {
    cancelGame(state);
    save();
  }
  if (page != p) {
    selected = 0;
    selectedCode = -1;
  }
  page = p;
  pageTransition = true;
  dirty = true;
  rebuildGame = true;
  sfx("botao");
}
static void doAction(Action a, const Food *f = nullptr) {
  auto before = std::make_unique<State>(state);
  auto result = act(state, a, f, rules);
  notice = result.message;
  if (result.changed) {
    if (!save()) {
      state = *before;
      return;
    }
    sfx(a == Action::Meal || a == Action::Fruit || a == Action::Treat ? "comida"
        : a == Action::Water                                          ? "agua"
        : a == Action::Toilet   ? "banheiro"
        : a == Action::Flush    ? "descarga"
        : a == Action::Sleep    ? "sono"
        : a == Action::Medicine ? "tratamento"
        : a == Action::Explore  ? "explorar"
                                : "carinho");
    if (a == Action::Pet)
      board::voice(path(std::string("audio/sfx/") + families[state.family] +
                        "_alegria.wav"));
    if (a == Action::Sleep || a == Action::Nap || a == Action::Lullaby)
      board::voice(path(std::string("audio/sfx/") + families[state.family] +
                        "_sono.wav"));
  } else
    sfx("recusa");
  dirty = true;
}
static void render() {
  if (screenOff)
    return; // Keep dirty set: rebuild once the user wakes the screen.
  dayOpacity = -1;
  settings.abc = false;
  if (nameField && page == "name") {
    nameDraft = lv_textarea_get_text(nameField);
    nameCursor = lv_textarea_get_cursor_pos(nameField);
  }
  nameField = keyboardLabel = nullptr;
  for (int i = 0; i < 4; i++)
    homeBars[i] = homeLabels[i] = nullptr;
  dirty = false;
  clockLabel = batteryLabel = batteryDetails = nullptr;
  status = nullptr;
  message = nullptr;
  petImage = nullptr;
  gameArea = nullptr;
  buttons.clear();
  buttonCodes.clear();
  lv_group_remove_all_objs(group);
  lv_obj_clean(screen);
  for (auto &t : thumbs)
    t.key = -1;
  header = box(screen, 16, 8, 1248, 60, 0xf6eedc);
  char text[256];
  if (!ready || fault) {
    label(header,
          ready ? "Bichinhos do Tab5 • recuperação"
                : "Bichinhos do Tab5 - recuperacao",
          20, 6);
    label(screen,
          !ready ? "Recursos indisponíveis. " + faultText + " Confira o microSD e a pasta tamagotchi."
                 : faultText,
          50, 150, 1150);
    button(screen, "Tentar novamente", 980, 60, 460, 350, 90);
    button(screen, "Voltar ao launcher", 999, 440, 460, 350, 90);
    if (faultRestoreConfirm) {
      label(screen,
            "Restaurar o backup deste perfil? Os arquivos atuais serão "
            "preservados em saves/recovery.",
            50, 290, 1150);
      button(screen, "Confirmar restauração", 982, 60, 570, 730, 90);
      button(screen, "Cancelar", 983, 820, 570, 350, 90);
    } else if (ready)
      button(screen, "Restaurar backup do perfil", 981, 60, 570, 730, 90);
    return;
  }
  snprintf(text, sizeof text, "%s • %s • Perfil %u", state.name,
           state.dead ? "Fim de ciclo" : stageName(state.stage),
           settings.profile + 1);
  if (profileVacant)
    snprintf(text, sizeof text, "Perfis • escolha ou crie um amigo");
  label(header, text, 20, 4, 890);
  batteryLabel = label(header, batteryText(), 1000, 10, 230);
  lv_obj_set_style_text_font(batteryLabel, &smallFont, 0);
  time_t headerTime = board::wall();
  tm headerLocal{};
  localtime_r(&headerTime, &headerLocal);
  char clockText[16];
  if (headerTime > 1704067200)
    strftime(clockText, sizeof clockText, "%H:%M", &headerLocal);
  else
    snprintf(clockText, sizeof clockText, "--:--");
  clockLabel = label(header, clockText, 876, 4, 120);
  sceneBytes =
      readFile(path(std::string("scenes/") + scenes[sceneIndex()] + ".rgb"));
  sceneImage = lv_image_create(screen);
  if (sceneBytes.size() == 640 * 360 * 2) {
    imageDescriptor(sceneDsc, 640, 360, sceneBytes.data(), false);
    lv_image_set_src(sceneImage, &sceneDsc);
    lv_image_set_scale(sceneImage, 371);
    lv_image_set_antialias(sceneImage, false);
    lv_image_set_pivot(sceneImage, 0, 0);
    lv_obj_set_pos(sceneImage, 16, 80);
  } else
    label(screen, "Cenário ausente", 30, 90);
  night = box(screen, 16, 80, 928, 522, 0x29354b);
  time_t wallNow = board::wall();
  tm local{};
  localtime_r(&wallNow, &local);
  lv_obj_set_style_bg_opa(
      night,
      !state.light || state.sleeping                                       ? 100
      : wallNow > 1704067200 && (local.tm_hour >= 18 || local.tm_hour < 6) ? 32
                                                                           : 0,
      0);
  lv_obj_remove_flag(night, LV_OBJ_FLAG_CLICKABLE);
  loadPet();
  if (state.pendingEvolution && page != "match") {
    auto *halo = box(screen, 330, page == "home" ? 180 : 228, 348, 348, 0xffffff);
    lv_obj_set_style_bg_opa(halo, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(halo, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(halo, 5, 0);
    lv_obj_set_style_border_color(halo, color(0xd5bd72), 0);
    lv_obj_remove_flag(halo, LV_OBJ_FLAG_CLICKABLE);
    lv_anim_t glow; lv_anim_init(&glow); lv_anim_set_var(&glow, halo);
    lv_anim_set_values(&glow, 30, 220); lv_anim_set_duration(&glow, 500);
    lv_anim_set_playback_duration(&glow, 500); lv_anim_set_repeat_count(&glow, 1);
    lv_anim_set_exec_cb(&glow, [](void *obj, int32_t value) { lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), value, 0); });
    lv_anim_start(&glow);
  }
  petImage = lv_image_create(screen);
  lv_obj_set_pos(petImage, 360, page == "home" ? 210 : 258);
  lv_image_set_scale(petImage, 768);
  lv_image_set_antialias(petImage, false);
  lv_image_set_pivot(petImage, 0, 0);
  if (!bank.empty()) {
    imageDescriptor(petDsc, activeAsset().width, activeAsset().height,
                    bank.data() + animationFrame() * activeAsset().width * activeAsset().height * 3, true);
    lv_image_set_src(petImage, &petDsc);
  }
  lv_obj_add_flag(petImage, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(
      petImage,
      [](lv_event_t *e) {
        if (lv_event_get_code(e) == LV_EVENT_PRESSING) {
          if (page == "hygiene" && state.bathStep == 1)
            doAction(Action::Wash);
        } else
          doAction(Action::Pet);
      },
      LV_EVENT_CLICKED, nullptr);
  lv_obj_add_event_cb(
      petImage,
      [](lv_event_t *) {
        if (page == "hygiene" && state.bathStep == 1)
          doAction(Action::Wash);
      },
      LV_EVENT_PRESSING, nullptr);
  if (sceneIndex() == 1 || sceneIndex() == 3) {
    foreground =
        box(screen, 320, 470, 380, 95, sceneIndex() == 1 ? 0xb78f65 : 0xe8ecde);
    lv_obj_set_style_border_width(foreground, 4, 0);
    lv_obj_set_style_border_color(foreground, color(0x627d7b), 0);
  }
  if (state.poop) {
    status =
        label(screen, "Um acidente! Toque em Higiene > Limpar.", 55, 540, 780);
  } else if (state.stage == Stage::Egg) {
    snprintf(text, sizeof text, "Ovo surpresa • eclosão em %u:%02u",
             unsigned((state.incubationTarget - state.incubation) / 60000),
             unsigned((state.incubationTarget - state.incubation) / 1000 % 60));
    status = label(screen, text, 55, 540, 800);
  } else {
    std::string form = names[state.family][state.form];
    status = label(screen,
                   form + " • " +
                       (state.sick                  ? "preciso de cuidados"
                        : state.stats[Urgency] > 75 ? "banheiro, por favor!"
                        : state.sleeping ? "dormindo tranquilamente"
                                         : "toque em mim para fazer carinho"),
                   48, 548, 850);
  }
  panel = box(screen, 962, 80, 302, 522, 0xe4dcc9);
  message = label(screen, notice, 32, 598, 1200);
  lv_obj_set_style_text_font(message, &smallFont, 0);
  lv_label_set_long_mode(message, LV_LABEL_LONG_SCROLL_CIRCULAR);
  auto menu = [&](const std::vector<std::pair<std::string, int>> &items) {
    int i = 0;
    for (auto &item : items) {
      bool grid = page == "home" || page == "food";
      auto *b =
          button(panel, item.first, item.second, grid ? 10 + (i % 2) * 146 : 10,
                 grid ? 12 + (i / 2) * 124 : 12 + i * 62, grid ? 136 : 282,
                 grid ? 112 : 54);
      if (!grid)
        lv_obj_set_style_text_font(lv_obj_get_child(b, 0), &smallFont, 0);
      i++;
    }
  };
  if (page == "home") {
    const char *actions[] = {"Comida", "Higiene", "Sono",
                             "Saúde",  "Jogos",   "Explorar"};
    int codes[] = {10, 11, 12, 13, 14, 118};
    for (int i = 0; i < 6; i++) {
      auto *b =
          button(screen, actions[i], codes[i], 24 + i * 154, 508, 142, 92);
      lv_obj_set_style_text_font(lv_obj_get_child(b, 0), &smallFont, 0);
    }
    label(panel, "Como estou", 18, 4, 266);
    const char *needs[] = {"Saciedade", "Humor", "Energia", "Higiene"};
    int targets[] = {10, 105, 12, 11};
    for (int i = 0; i < 4; i++) {
      auto *tile = button(panel, "", targets[i], 10, 40 + i * 72, 282, 64);
      homeLabels[i] = label(tile, needs[i], 10, -2, 246);
      lv_obj_set_style_text_font(homeLabels[i], &smallFont, 0);
      homeBars[i] = lv_bar_create(tile);
      lv_obj_set_pos(homeBars[i], 10, 37);
      lv_obj_set_size(homeBars[i], 246, 7);
      lv_obj_set_style_bg_color(homeBars[i], color(0xcccccc), LV_PART_MAIN);
      lv_obj_set_style_bg_color(homeBars[i], color(0x777777),
                                LV_PART_INDICATOR);
      lv_bar_set_value(homeBars[i], int(state.stats[i]), LV_ANIM_OFF);
    }
    button(panel, "Ficha completa", 16, 10, 338, 282, 54);
    button(panel, "Educação", 15, 10, 398, 282, 54);
    button(panel, "Perfis", 25, 10, 458, 282, 54);
    if (status) {
      auto *caption = box(screen, 32, 92, 880, 54, 0xeeeeee);
      lv_obj_set_parent(status, caption);
      lv_obj_set_pos(status, 12, 8);
      lv_obj_set_width(status, 856);
      lv_obj_set_style_text_font(status, &smallFont, 0);
    }
  } else if (page == "food") {
    std::vector<std::pair<std::string, int>> items;
    for (int i = foodPage * 4;
         i < std::min(int(foods.size()), foodPage * 4 + 4); i++)
      items.emplace_back(foods[i].shortName, 200 + i);
    items.emplace_back("Água", 100 + int(Action::Water));
    items.emplace_back("Mais pratos", 19);
    items.emplace_back("Voltar", 1);
    menu(items);
  } else if (page == "hygiene")
    menu({{"Usar vaso", 100 + int(Action::Toilet)},
          {"Descarga", 100 + int(Action::Flush)},
          {"Limpar", 100 + int(Action::Clean)},
          {"Banho", 100 + int(Action::Bath)},
          {"Espuma", 100 + int(Action::Wash)},
          {"Enxaguar", 100 + int(Action::Rinse)},
          {"Secar", 100 + int(Action::Dry)},
          {"Voltar", 1}});
  else if (page == "sleep")
    menu({{"Dormir", 100 + int(Action::Sleep)},
          {"Soneca (10 min)", 100 + int(Action::Nap)},
          {"Acordar", 100 + int(Action::Wake)},
          {"Ninar", 100 + int(Action::Lullaby)},
          {state.light ? "Apagar luz" : "Acender luz", 20},
          {"Sono: " + std::to_string(state.sleepHour) + "h", 21},
          {"Acordar: " + std::to_string(state.wakeHour) + "h", 22},
          {"Voltar", 1}});
  else if (page == "health")
    menu({{"Remédio", 115},
          {"Ficha", 16},
          {"Descansar", 12},
          {"Higiene", 11},
          {"Voltar", 1}});
  else if (page == "education")
    menu({{"Brincar", 14},
          {"Elogiar", 100 + int(Action::Praise)},
          {"Orientar", 100 + int(Action::Guide)},
          {"Carinho", 100 + int(Action::Pet)},
          {"Ficha", 16},
          {"Voltar", 1}});
  else if (page == "games") {
    std::vector<std::pair<std::string, int>> items;
    for (int i = 0; i < 8; i++)
      items.emplace_back(gameNames[i], 300 + i);
    menu(items);
  } else if (page == "match") {
    auto *rule =
        label(panel, instructions[int(state.match.kind) - 1], 14, 14, 274);
    lv_obj_set_style_text_font(rule, &smallFont, 0);
    if (!settings.abc)
      button(panel, "Sair do jogo", 43, 18, 390, 268, 108);
    renderGame();
  } else if (page == "info") {
    auto *p = box(screen, 36, 104, 886, 438, 0xf6eedc);
    snprintf(
        text, sizeof text,
        "%s • %s\nIdade ativa: %llu h %llu min • peso lúdico: "
        "%u\nPersonalidade: %s • comida favorita: %s",
        state.stage == Stage::Egg ? "Família surpresa" : families[state.family],
        state.stage == Stage::Egg ? "Incubando"
                                  : names[state.family][state.form].c_str(),
        state.age / 3600000, state.age / 60000 % 60, 10 + state.form * 3,
        (const char *[]){"tranquilo", "explorador",
                         "brincalhão"}[state.personality],
        foods[state.preferredFood].name);
    label(p, text, 18, 12, 850);
    static const char *sn[] = {
        "Saciedade", "Felicidade", "Energia do pet", "Higiene",
        "Saúde",     "Vínculo",    "Educação",       "Urgência banheiro"};
    for (int i = 0; i < 8; i++) {
      int x = 18 + (i % 2) * 430, y = 166 + (i / 2) * 60;
      snprintf(text, sizeof text, "%s: %.0f", sn[i], state.stats[i]);
      label(p, text, x, y, 410);
      auto *bar = lv_bar_create(p);
      lv_obj_set_pos(bar, x, y + 39);
      lv_obj_set_size(bar, 400, 9);
      lv_bar_set_value(bar, int(state.stats[i]), LV_ANIM_OFF);
    }
    menu({{"Mudar nome", 23},
          {"Retrato", 24},
          {"Perfis", 25},
          {"Encerrar ciclo", 26},
          {"Pistas", 27},
          {"Voltar", 1}});
  } else if (page == "album") {
    auto *p = box(screen, 36, 94, 886, settings.abc ? 438 : 490, 0xf6eedc);
    label(p, std::string("Álbum • ") + families[albumFamily], 18, 8, 840);
    for (int i = 0; i < 8; i++) {
      bool seen = state.discovered & (1ULL << (albumFamily * 8 + i));
      auto *card = button(p,
                          seen ? names[albumFamily][i]
                               : "? • " + std::string(i < 2   ? "crescer"
                                                      : i < 4 ? "conviver"
                                                              : "descobrir"),
                          400 + i, 15 + (i % 4) * 215, 70 + (i / 4) * 170, 205,
                          155, seen ? 0xdce6cf : 0xd9d6cf);
      {
        thumb(card, albumFamily, i, 54, 0, 256, !seen);
        auto *l = (lv_obj_t *)lv_obj_get_child(card, 0);
        lv_obj_align(l, LV_ALIGN_BOTTOM_MID, 0, 0);
      }
    }
    menu({{"Outra família", 28},
          {"Histórias", 46},
          {"Decorar", 29},
          {"Retrato", 24},
          {"Voltar", 1}});
  } else if (page == "history") {
    auto *p = box(screen, 36, 94, 886, 490, 0xf6eedc);
    lv_obj_add_flag(p, LV_OBJ_FLAG_SCROLLABLE);
    historyFiles.clear();
    button(p, std::string("Atual: ") + state.name, 850, 18, 12, 820, 80);
    board::lockIO();
    DIR *d = opendir(path("album").c_str());
    if (d) {
      dirent *e;
      while ((e = readdir(d)) && historyFiles.size() < 10000) {
        std::string name = e->d_name;
        if ((name.rfind("life-", 0) == 0 || name.rfind("deleted-", 0) == 0) &&
            name.size() > 4 && name.substr(name.size() - 4) == ".sav") historyFiles.push_back(name);
      }
      closedir(d);
    }
    board::unlockIO();
    std::sort(historyFiles.begin(), historyFiles.end());
    if (historyPage * 48 >= historyFiles.size()) historyPage = 0;
    auto old = std::make_unique<State>();
    int row = 1;
    for (size_t i = historyPage * 48; i < std::min(historyFiles.size(), (historyPage + 1) * 48); ++i) {
      board::lockIO();
      bool good = inspectArchive(path("album/" + historyFiles[i]), *old);
      board::unlockIO();
      if (good && old->profile == settings.profile)
        button(p, std::string(old->name) + " • " + stageName(old->stage) + " • " +
               std::to_string(old->age / 3600000) + " h", 851 + i - historyPage * 48, 18, 12 + row++ * 96, 820, 80);
    }
    std::vector<std::pair<std::string, int>> options;
    if (historyPage) options.emplace_back("Histórias anteriores", 848);
    if ((historyPage + 1) * 48 < historyFiles.size()) options.emplace_back("Mais histórias", 849);
    options.emplace_back("Voltar", 17); menu(options);
  } else if (page == "life" && historical) {
    const auto &old = *historical;
    auto *p = box(screen, 36, 94, 886, 490, 0xf6eedc);
    auto *details = box(p, 325, 16, 540, 450, 0xf6eedc);
    lv_obj_add_flag(details, LV_OBJ_FLAG_SCROLLABLE);
    thumb(p, old.family, old.stage == Stage::Egg ? 8 : old.form, 20, 65, 768, false, &old);
    label(p, old.name, 20, 360, 300);
    auto date = [](uint64_t seconds) {
      if (!seconds) return std::string("não registrado");
      time_t value = seconds; tm t{}; localtime_r(&value, &t);
      char line[32]; strftime(line, sizeof line, "%d/%m/%Y %H:%M", &t); return std::string(line);
    };
    std::string body = std::string(stageName(old.stage)) + " • " + (old.stage == Stage::Egg ? "família surpresa" : families[old.family]) +
      "\nIdade: " + std::to_string(old.age / 3600000) + " h" +
      "\nPaleta: " + std::to_string(old.palette + 1) + " • detalhe: " + std::to_string(old.detail + 1) +
      "\nOvo: " + date(old.createdWall) + "\nNascimento: " + date(old.birthWall) +
      "\nFim do ciclo: " + date(old.endedWall);
    for (int i = 1; i < 6; ++i) if (old.milestoneAge[i] || old.milestoneWall[i])
      body += "\n" + std::string(stageName(Stage(i))) + ": " +
              std::to_string(old.milestoneAge[i] / 3600000) + " h • " + date(old.milestoneWall[i]);
    uint64_t parents[] = {old.parents[0], old.parents[1]};
    if (!parents[0]) {
      auto *relation = json(path("album/parents-" + std::to_string(old.id) + ".json"));
      if (relation) {
        auto *array = cJSON_GetObjectItem(relation, "parents");
        for (int i = 0; i < 2; ++i) { auto *v = cJSON_GetArrayItem(array, i); if (cJSON_IsString(v)) parents[i] = strtoull(v->valuestring, nullptr, 10); }
        cJSON_Delete(relation);
      }
    }
    body += "\n\nOrigem: " + (parents[0] ? std::string("herança local") : "ovo sorteado");
    for (auto parent : parents) if (parent) body += "\nPai/mãe: " + std::to_string(parent);
    // Find children across live profiles and preserved archives.
    auto child = std::make_unique<State>();
    for (int i = 0; i < 3; ++i) {
      board::lockIO(); auto result = store.load(i, *child); board::unlockIO();
      if (result == Load::Ok && (child->parents[0] == old.id || child->parents[1] == old.id))
        body += "\nDescendente: " + std::string(child->name) + " (perfil " + std::to_string(i + 1) + ")";
    }
    std::set<uint64_t> descendants;
    for (const auto &file : historyFiles) {
      board::lockIO(); bool found = inspectArchive(path("album/" + file), *child); board::unlockIO();
      if (found && (child->parents[0] == old.id || child->parents[1] == old.id) && descendants.insert(child->id).second)
        body += "\nDescendente arquivado: " + std::string(child->name) + " • " + stageName(child->stage);
    }
    body += "\n\nDiário preservado:";
    for (unsigned i = 0; i < old.logCount; ++i) {
      const auto &event = old.events[(old.logHead + 40 - old.logCount + i) % 40];
      body += "\n" + std::to_string(event.age / 3600000) + " h: " + event.text;
    }
    auto *text = label(details, body, 10, 8, 510);
    lv_obj_set_style_text_font(text, &smallFont, 0);
    menu({{"Voltar às histórias", 46}, {"Álbum", 17}});
  } else if (page == "breeding") {
    label(panel,
          "Dois adultos podem originar um ovo no terceiro perfil livre. Não há "
          "híbridos.",
          14, 14, 270);
    button(panel, "Voltar", 25, 18, 390, 268, 108);
    button(screen, "Perfis 1 + 2 → perfil 3", 520, 60, 220, 830, 90);
    button(screen, "Perfis 1 + 3 → perfil 2", 521, 60, 330, 830, 90);
    button(screen, "Perfis 2 + 3 → perfil 1", 522, 60, 440, 830, 90);
  } else if (page == "diary") {
    auto *p = box(screen, 36, 94, 886, settings.abc ? 438 : 490, 0xf6eedc);
    lv_obj_add_flag(p, LV_OBJ_FLAG_SCROLLABLE);
    for (int i = 0; i < state.logCount; i++) {
      auto &e = state.events[(state.logHead + 39 - i) % 40];
      snprintf(text, sizeof text, "%llu h: %s", e.age / 3600000, e.text);
      label(p, text, 18, i * 88, 840);
    }
    menu({{"Álbum", 17}, {"Voltar", 1}});
  } else if (page == "settings") {
    auto *p = box(screen, 36, 94, 886, 490, 0xeeeeee);
    label(p, "Ajustes", 24, 20, 820);
    const char *titles[] = {
        "Som e volume",    "Tela e conforto",      "Perfis",
        "Modo de cuidado", "Backup e recuperação", "Voltar ao launcher"};
    int codes[] = {80, 81, 25, 37, 36, 999};
    for (int i = 0; i < 6; i++)
      button(p, titles[i], codes[i], 24 + (i % 2) * 420, 85 + (i / 2) * 122,
             400, 100);
    menu({{"Voltar à casa", 1}});
  } else if (page == "audio") {
    auto *p = box(screen, 36, 94, 886, 490, 0xeeeeee);
    label(p, "Som e volume", 24, 16, 820);
    const uint8_t values[] = {settings.master, settings.music, settings.sfx};
    const char *titles[] = {"Geral", "Música", "Efeitos"};
    for (int i = 0; i < 3; ++i)
      adjustment(p, titles[i], std::to_string(values[i]) + "%", 830 + i * 2,
                 86 + i * 110, values[i] == 0, values[i] == 100);
    menu({{std::string("Silêncio no sono: ") +
               (settings.silentSleep ? "sim" : "não"), 40},
          {"Voltar", 18}});
  } else if (page == "display") {
    auto *p = box(screen, 36, 94, 886, 490, 0xeeeeee);
    label(p, "Tela e bateria", 24, 16, 820);
    adjustment(p, "Brilho", std::to_string(settings.brightness) + "%", 836,
               80, settings.brightness == 10, settings.brightness == 100);
    adjustment(p, "Apagar em", timeoutText(), 838, 180,
               settings.screenTimeoutSeconds == 0,
               settings.screenTimeoutSeconds == 1800);
    auto *help = label(p, "Sem uso: 30 s a 30 min; 0 = nunca.\n"
                          "Toque para reacender. Jogos mantêm a tela ligada.",
                       24, 274, 830);
    lv_obj_set_style_text_font(help, &smallFont, 0);
    batteryDetails = label(p, batteryText(true), 24, 370, 830);
    lv_obj_set_style_text_font(batteryDetails, &smallFont, 0);
    auto *hint = label(p, "Porcentagem estimada pela tensão da bateria.",
                       24, 418, 830);
    lv_obj_set_style_text_font(hint, &smallFont, 0);
    menu({{std::string("Tema retrô: ") + (settings.retro ? "sim" : "não"), 34},
          {std::string("Inclinação: ") + (settings.tilt ? "sim" : "não"), 41},
          {"Extensões", 48}, {"Voltar", 18}});
  } else if (page == "more")
    menu({{"Exportar backup", 38}, {"Restaurar backup", 39}, {"Voltar", 18}});
  else if (page == "extras") {
    menu({{"Inclinação", 41},
#if CONFIG_GOTCHI_BREEDING
          {"Herança local", 47},
#endif
#if CONFIG_GOTCHI_LAN
          {"Visitas LAN", 49},
#endif
          {"Voltar", 36}});
#if CONFIG_GOTCHI_LAN
  } else if (page == "lan") {
    if (!ownCode)
      ownCode = 100000 + esp_random() % 900000;
    auto *p = box(screen, 36, 104, 886, 420, 0xf6eedc);
    label(p, "Visitas na rede local • " + lan::status(), 18, 20, 840);
    label(p,
          "Seu código: " + std::to_string(ownCode) +
              "\nInforme o IP e o código do outro aparelho em Parear. Ambos "
              "confirmam a visita.\nNenhum arquivo, executável ou save é "
              "aceito pela rede.",
          18, 100, 840);
    if (hasVisit)
      label(p, "Pedido de visita recebido. Toque em Aceitar.", 18, 300, 840);
    menu({{"Configurar Wi-Fi", 60},
          {"Parear", 61},
          {"Enviar visita", 62},
          {"Dar presente", 63},
          {"Aceitar", 64},
          {"Recusar", 65},
          {"Desconectar", 66},
          {"Voltar", 48}});
  } else if (page == "lanform") {
    const char *title =
        (const char *[]){"Nome da rede Wi-Fi", "Senha da rede (não será salva)",
                         "IP do outro Tab5", "Código do outro Tab5"}[lanField];
    label(screen, title, 40, 90, 880);
    auto *ta = lv_textarea_create(screen);
    lv_obj_set_pos(ta, 45, 143);
    lv_obj_set_size(ta, 880, 68);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_password_mode(ta, lanField == 1);
    lv_textarea_set_max_length(ta, lanField == 0   ? 32
                                   : lanField == 1 ? 64
                                   : lanField == 2 ? 15
                                                   : 6);
    if (lanField == 0)
      lv_textarea_set_text(ta, lanSsid.c_str());
    else if (lanField == 2)
      lv_textarea_set_text(ta, peerIP.c_str());
    auto *kb = lv_keyboard_create(screen);
    lv_obj_set_pos(kb, 35, 225);
    lv_obj_set_size(kb, 900, 360);
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_add_event_cb(
        kb,
        [](lv_event_t *e) {
          if (lv_event_get_code(e) == LV_EVENT_READY) {
            auto *ta =
                lv_keyboard_get_textarea((lv_obj_t *)lv_event_get_target(e));
            std::string value = lv_textarea_get_text(ta);
            if (lanField == 0) {
              lanSsid = value;
              lanField = 1;
              dirty = true;
            } else if (lanField == 1) {
              lanPassword = value;
              lan::connect(lanSsid.c_str(), lanPassword.c_str(), ownCode);
              lanPassword.clear();
              setPage("lan");
            } else if (lanField == 2) {
              peerIP = value;
              lanField = 3;
              dirty = true;
            } else {
              remoteCode = atoi(value.c_str());
              setPage("lan");
            }
          } else if (lv_event_get_code(e) == LV_EVENT_CANCEL)
            setPage("lan");
        },
        LV_EVENT_ALL, nullptr);
#endif
  } else if (page == "modes") {
    auto *p = box(screen, 36, 104, 886, 430, 0xf6eedc);
    label(p,
          "Tranquilo: pausa tudo ao fechar. Sem morte por "
          "descuido.\n\nContínuo: cresce na ausência, com tolerância de 2 h e "
          "desgaste limitado a 12 h.\n\nClássico: morte possível após 48 h de "
          "negligência ativa. Relógio inválido não causa morte.",
          20, 20, 840);
    menu({{"Tranquilo", 500},
          {"Contínuo", 501},
          {"Clássico", 502},
          {"Voltar", 18}});
  } else if (page == "memorydifficulty")
    menu(
        {{"4 pares", 314}, {"6 pares", 316}, {"8 pares", 318}, {"Voltar", 14}});
  else if (page == "profiles") {
    auto *p = box(screen, 36, 94, 886, 490, 0xeeeeee);
    label(p, "Seus perfis", 24, 16, 820);
    auto entry = std::make_unique<State>();
    for (int i = 0; i < 3; i++) {
      board::lockIO();
      auto result = store.load(i, *entry);
      board::unlockIO();
      bool empty = result == Load::Missing;
      label(p,
            "Perfil " + std::to_string(i + 1) +
                (empty ? " • vazio"
                       : " • " + std::string(result == Load::Ok
                                                 ? entry->name
                                                 : "recuperação necessária")),
            24, 85 + i * 125, 490);
      button(p, empty ? "Criar ovo" : "Abrir", 510 + i, 510, 65 + i * 125, 155,
             95);
      if (!empty)
        button(p, "Apagar", 530 + i, 680, 65 + i * 125, 175, 95);
    }
    menu({{"Voltar", 18}});
  } else if (page == "decor") {
    auto *p = box(screen, 36, 94, 886, settings.abc ? 438 : 490, 0xf6eedc);
    for (int i = 0; i < 12; i++)
      button(p, (state.decorations & (1 << i)) ? decorNames[i] : "A descobrir",
             600 + i, 12 + (i % 4) * 218,
             14 + (i / 4) * (settings.abc ? 140 : 152), 206,
             settings.abc ? 128 : 138);
    menu({{"Voltar", 1}});
  } else if (page == "name") {
    auto *p = box(screen, 36, 94, 886, 490, 0xeeeeee);
    label(p, "Nome do bichinho", 24, 20, 820);
    auto *ta = lv_textarea_create(p);
    lv_obj_set_pos(ta, 24, 78);
    lv_obj_set_size(ta, 838, 70);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, 16);
    lv_textarea_set_text(ta, nameDraft.c_str());
    lv_textarea_set_cursor_pos(ta, nameCursor);
    nameField = ta;
    label(p, "Digite no teclado encaixado no Tab5.\nEnter salva • Esc cancela • Ctrl+A limpa",
          24, 190, 838);
    keyboardLabel = label(p, keyboardOnline ? "Teclado conectado" : "Procurando teclado encaixado…", 24, 310, 838);
    lv_obj_set_style_text_font(keyboardLabel, &smallFont, 0);
    lv_group_focus_obj(ta);
    menu({{"Salvar nome", 82}, {"Cancelar", 16}});
  } else if (page == "confirm") {
    auto *p = box(screen, 36, 150, 886, 350, 0xf6eedc);
    label(p, notice, 24, 30, 830);
    menu({{"Confirmar", 900}, {"Cancelar", 1}});
  }
  if (page == "home") {
    int icon = 0;
    for (size_t i = 0; i < buttonCodes.size() && icon < 6; i++)
      if (lv_obj_get_parent(buttons[i]) == screen) {
        careIcons[icon] = readFile(path(
            "ui/action" +
            std::to_string((const int[]){0, 1, 2, 4, 5, 6}[icon]) + ".rgb"));
        if (careIcons[icon].size() == 6912) {
          imageDescriptor(careDsc[icon], 48, 48, careIcons[icon].data(), true);
          auto *im = lv_image_create(buttons[i]);
          lv_image_set_src(im, &careDsc[icon]);
          lv_obj_align(im, LV_ALIGN_TOP_MID, 0, -8);
          auto *l = (lv_obj_t *)lv_obj_get_child(buttons[i], 0);
          lv_obj_align(l, LV_ALIGN_BOTTOM_MID, 0, -1);
        }
        icon++;
      }
  }
  if (page == "food") {
    int icon = 0;
    for (size_t i = 0; i < buttonCodes.size(); i++)
      if (buttonCodes[i] >= 200 && buttonCodes[i] < 212 && icon < 4) {
        auto *b = buttons[i];
        foodIcons[icon] = readFile(
            path("ui/food" + std::to_string(buttonCodes[i] - 200) + ".rgb"));
        if (foodIcons[icon].size() == 12288) {
          imageDescriptor(foodIconDsc[icon], 64, 64, foodIcons[icon].data(),
                          true);
          auto *im = lv_image_create(b);
          lv_image_set_src(im, &foodIconDsc[icon]);
          lv_obj_set_pos(im, 32, -10);
          auto *l = (lv_obj_t *)lv_obj_get_child(b, 0);
          lv_obj_align(l, LV_ALIGN_BOTTOM_MID, 0, 8);
          lv_obj_set_style_text_font(l, &smallFont, 0);
        }
        icon++;
      }
  }
  footer = box(screen, 16, 632, 1248, 80, 0xe7ddc8);
  const char *ft[] = {"Casa", "Ficha", "Álbum", "Diário", "Ajustes"};
  int fc[] = {1, 16, 17, 42, 18};
  for (int i = 0; i < 5; i++) {
    auto *b = button(footer, ft[i], fc[i], i * 249 + 4, 0, 241, 80);
    bool active = (i == 0 && page == "home") || (i == 1 && page == "info") ||
                  (i == 2 && page == "album") || (i == 3 && page == "diary") ||
                  (i == 4 && page == "settings");
    if (active) {
      lv_obj_set_style_bg_color(b, color(0xc6c6c6), 0);
      lv_obj_set_style_border_width(b, 3, 0);
    }
  }
#if CONFIG_GOTCHI_LAN
  if (page == "home" && visitorVisible) {
    int form = (const int[]){8, 0, 1, 2, 4, 7}[pendingVisit.stage];
    thumb(screen, pendingVisit.family, form, 160, 365, 512);
    label(screen, "Visita local", 170, 540, 250);
  }
#endif
  if (page == "home" && state.stage != Stage::Egg) {
    toyBytes =
        readFile(path("ui/decor" + std::to_string(state.favorite) + ".rgb"));
    if (toyBytes.size() == 64 * 64 * 3) {
      imageDescriptor(toyDsc, 64, 64, toyBytes.data(), true);
      toyImage = lv_image_create(screen);
      lv_image_set_src(toyImage, &toyDsc);
      lv_obj_set_pos(toyImage, 780, 408);
      lv_image_set_scale(toyImage, 384);
    }
  }
  if (state.stage == Stage::Egg)
    board::music(path("audio/music/bercario.wav"));
  else {
    const char *musicName = scenes[sceneIndex()];
    if (sceneIndex() == 3)
      musicName = "banheiro";
    if (sceneIndex() == 9)
      musicName = "casa";
    board::music(path(std::string("audio/music/") + musicName + ".wav"));
  }
  board::mute(settings.silentSleep && state.sleeping);
  board::volume(settings.master, settings.music, settings.sfx);
  applyBrightness();
  if (page == "match" && petImage) {
    lv_image_set_scale(petImage, 384);
    lv_obj_set_pos(petImage, 752, 85);
    lv_obj_move_foreground(petImage);
  }
  if (message) lv_label_set_text(message, notice.c_str());
  if (pageTransition && panel) {
    pageTransition = false;
    lv_anim_t fade; lv_anim_init(&fade); lv_anim_set_var(&fade, panel);
    lv_anim_set_values(&fade, 80, 255); lv_anim_set_duration(&fade, 160);
    lv_anim_set_exec_cb(&fade, [](void *obj, int32_t value) { lv_obj_set_style_opa(static_cast<lv_obj_t *>(obj), value, 0); });
    lv_anim_start(&fade);
  }
  selected = std::min(selected, int(buttons.size()) - 1);
  focusSelected();
}
static void gameButton(const std::string &t, int input, int x, int y, int w,
                       int h, uint32_t bg = 0xe0e0e0) {
  auto *b = button(gameArea, t, 700 + input, x, y, w, h, bg);
  gameButtons.push_back(b);
}
static void renderGame() {
  if (petImage) {
    bool hiding = state.match.kind == Game::Hide && state.match.active && !state.match.phase;
    if (hiding) lv_obj_add_flag(petImage, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_remove_flag(petImage, LV_OBJ_FLAG_HIDDEN);
  }
  auto &m = state.match;
  if (gameArea) {
    for (int i = int(buttons.size()) - 1; i >= 0; i--)
      if (lv_obj_get_parent(buttons[i]) == gameArea) {
        buttons.erase(buttons.begin() + i);
        buttonCodes.erase(buttonCodes.begin() + i);
      }
  }
  if (message) lv_label_set_text(message, notice.c_str());
  rebuildGame = false;
  lastPhase = m.phase > 0;
  lastFirst = m.first;
  int noteKey =
      m.showing
          ? (m.elapsed / 800 < m.length
                 ? int(m.elapsed / 800) * 2 + (m.elapsed % 800 < 500 ? 0 : 1)
                 : -2)
          : -1;
  if (m.kind == Game::Sequence && noteKey >= 0 && noteKey % 2 == 0 &&
      noteKey != lastShow)
    sfx("nota" + std::to_string(m.sequence[m.elapsed / 800]));
  lastShow = noteKey;
  bool newGameArea = !gameArea;
  if (newGameArea) {
    gameArea = box(screen, 36, 80, 886, 522, 0xece4d3);
  } else
    lv_obj_clean(gameArea);
  gameButtons.clear();
  fruitObj = basketObj = nullptr;
  char t[256];
  snprintf(t, sizeof t, "%s • você %u : %u amigo",
           gameNames[int(m.kind) - 1].c_str(), m.wins, m.losses);
  auto *gameTitle = label(gameArea, t, 18, 8, 690);
  lv_obj_set_style_text_font(gameTitle, &smallFont, 0);
  gameStatus = label(gameArea, m.result, 18, 55, 690);
  lv_obj_set_style_text_font(gameStatus, &smallFont, 0);
  if (m.finished) {
    label(gameArea, "Brincadeira concluída!\nObrigado pela companhia.", 80, 150,
          700);
    if (m.kind == Game::Hide) {
      const char *objects[] = {"vaso", "árvore", "banco", "caixa", "arbusto", "fonte"};
      label(gameArea, std::string("Último esconderijo: ") + objects[m.opponent], 50, 245, 780);
      thumb(gameArea, state.family, state.form, 710, 170, 256);
    }
    gameButton("Jogar outra vez", 40, 190, 305, 500, 110);
    focusSelected();
    return;
  }
  if (m.kind == Game::Rps) {
    for (int i = 0; i < 3; i++)
      gameButton((const char *[]){"Pedra", "Papel", "Tesoura"}[i], i,
                 20 + i * 282, 230, 268, 160);
  } else if (m.kind == Game::Parity) {
    snprintf(t, sizeof t,
             m.round % 2 ? "O amigo escolheu %s para você."
                         : "Escolha sua paridade.",
             m.parity ? "ÍMPAR" : "PAR");
    label(gameArea, t, 18, 105, 850);
    button(gameArea, m.parity ? "Par" : "[ Par ]", 44, 30, 160, 395, 88);
    button(gameArea, m.parity ? "[ Ímpar ]" : "Ímpar", 45, 445, 160, 395, 88);
    for (int i = 0; i < 6; i++)
      gameButton(std::to_string(i), i, 25 + i * 141, 300, 130, 125);
  } else if (m.kind == Game::Direction || m.kind == Game::Higher) {
    if (m.kind == Game::Higher)
      label(gameArea, "Número atual: " + std::to_string(m.base), 20, 140, 800);
    gameButton(m.kind == Game::Higher ? "Menor" : "Esquerda", 0, 45, 245, 370,
               155);
    gameButton(m.kind == Game::Higher ? "Maior" : "Direita", 1, 455, 245, 370,
               155);
    if (m.kind == Game::Higher) {
      if (m.base == 1)
        lv_obj_add_state(gameButtons[0], LV_STATE_DISABLED);
      if (m.base == 9)
        lv_obj_add_state(gameButtons[1], LV_STATE_DISABLED);
    }
  } else if (m.kind == Game::Memory) {
    for (int i = 0; i < m.pairs * 2; i++) {
      bool visible = m.matched[i] || m.first == i || m.second == i;
      gameButton(visible ? families[m.cards[i] % 6] : "?", i,
                 14 + (i % 4) * 215, 180 + (i / 4) * 80, 204, 80,
                 visible ? 0xd8e5cd : 0xc4ced4);
      if (m.matched[i])
        lv_obj_add_state(gameButtons.back(), LV_STATE_DISABLED);
      if (visible) {
        auto *b = gameButtons.back();
        lv_obj_add_flag(lv_obj_get_child(b, 0), LV_OBJ_FLAG_HIDDEN);
        thumb(b, m.cards[i] % 6, m.cards[i] / 6, 64, 0, 210);
      }
    }
  } else if (m.kind == Game::Sequence) {
    int highlight =
        m.showing && m.elapsed / 800 < m.length && m.elapsed % 800 < 500
            ? m.sequence[std::min<int>(m.length - 1, m.elapsed / 800)]
            : -1;
    label(gameArea, m.showing ? "Observe as notas..." : "Sua vez!", 20, 118,
          800);
    for (int i = 0; i < 4; i++)
      gameButton(
          (const char *[]){"1 • sol", "2 • gota", "3 • folha", "4 • flor"}[i],
          i, 20 + (i % 2) * 430, 180 + (i / 2) * 135, 410, 120,
          highlight == i
              ? 0xffedb0
              : (const uint32_t[]){0xe8bd83, 0x8cc7d1, 0xa8c58e, 0xc5a5bc}[i]);
  } else if (m.kind == Game::Fruit) {
    fallingFruit = readFile(path("ui/fruit.rgb"));
    basketSprite = readFile(path("ui/basket.rgb"));
    fruitObj = lv_image_create(gameArea);
    basketObj = lv_image_create(gameArea);
    if (fallingFruit.size() == 3072) {
      imageDescriptor(fallingDsc, 32, 32, fallingFruit.data(), true);
      lv_image_set_src(fruitObj, &fallingDsc);
    }
    if (basketSprite.size() == 13824) {
      imageDescriptor(basketDsc, 96, 48, basketSprite.data(), true);
      lv_image_set_src(basketObj, &basketDsc);
    }
    lv_obj_remove_flag(fruitObj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(basketObj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(gameArea, LV_OBJ_FLAG_CLICKABLE);
    if (newGameArea)
      lv_obj_add_event_cb(
          gameArea,
          [](lv_event_t *) {
            auto *indev = lv_indev_active();
            if (indev) {
              lv_point_t pt;
              lv_indev_get_point(indev, &pt);
              play(state, std::clamp<int>((pt.x - 46) * 640 / 830, 0, 640), rules);
            }
          },
          LV_EVENT_PRESSING, nullptr);
  } else if (m.kind == Game::Hide) {
    for (int i = 0; i < 6; i++) {
      gameButton((const char *[]){"Vaso", "Árvore", "Banco", "Caixa", "Arbusto", "Fonte"}[i],
                 i, 20 + (i % 3) * 285, 150 + (i / 3) * 145, 270, 130);
      auto *target = gameButtons.back();
      hideArt[i] = readFile(path("ui/hide" + std::to_string(i) + ".rgb"));
      bool reveal = m.phase && i == m.opponent;
      if (reveal) thumb(target, state.family, state.form, 135, 0, 256);
      if (hideArt[i].size() == 128 * 96 * 3) {
        imageDescriptor(hideDsc[i], 128, 96, hideArt[i].data(), true);
        auto *prop = lv_image_create(target); lv_image_set_src(prop, &hideDsc[i]);
        lv_obj_set_pos(prop, 30, -8); lv_obj_remove_flag(prop, LV_OBJ_FLAG_CLICKABLE);
      } else {
        auto *missing = label(target, "Imagem ausente", 12, 20, 220);
        lv_obj_set_style_text_font(missing, &smallFont, 0);
      }
      auto *caption = lv_obj_get_child(target, 0);
      lv_obj_align(caption, LV_ALIGN_BOTTOM_MID, 0, 4);
      if (m.phase) lv_obj_add_state(target, LV_STATE_DISABLED);
    }

  }
  if (message) lv_label_set_text(message, notice.c_str());
  rebuildGame = false;
  lastPhase = m.phase > 0;
  lastFirst = m.first;
  lastShow =
      m.showing
          ? (m.elapsed / 800 < m.length
                 ? int(m.elapsed / 800) * 2 + (m.elapsed % 800 < 500 ? 0 : 1)
                 : -2)
          : -1;
  focusSelected();
}
static void click(int code) {
  bool waking = screenOff;
  displayActivity();
  if (waking)
    return;
  if (code == 999) {
    if (ready && !fault)
      save();
    board::launcher();
    return;
  }
  if (code == 980) {
    board::launcher();
    return;
  }
  if (code == 981) {
    faultRestoreConfirm = true;
    dirty = true;
    return;
  }
  if (code == 983) {
    faultRestoreConfirm = false;
    dirty = true;
    return;
  }
  if (code == 982 && faultRestoreConfirm) {
    board::lockIO();
    bool ok = store.restore(
        settings.profile,
        path("album/backup" + std::to_string(settings.profile) + ".sav"));
    board::unlockIO();
    if (ok) {
      fault = false;
      faultRestoreConfirm = false;
      loadProfile(settings.profile);
      dirty = true;
    }
    return;
  }
  if (!ready || fault)
    return;
  if (code == 848 || code == 849) {
    if (code == 848 && historyPage) --historyPage;
    if (code == 849 && (historyPage + 1) * 48 < historyFiles.size()) ++historyPage;
    dirty = true; return;
  }
  if (code >= 850 && code <= 898) {
    historical = std::make_unique<State>();
    if (code == 850) *historical = state;
    else {
      size_t index = code - 851 + historyPage * 48;
      if (index >= historyFiles.size()) return;
      board::lockIO(); bool ok = inspectArchive(path("album/" + historyFiles[index]), *historical); board::unlockIO();
      if (!ok || historical->profile != settings.profile) { notice = "História indisponível."; dirty = true; return; }
    }
    setPage("life"); return;
  }
  if (code >= 830 && code <= 839) {
    int direction = code % 2 ? 1 : -1;
    if (code <= 837) {
      uint8_t *values[] = {&settings.master, &settings.music, &settings.sfx,
                           &settings.brightness};
      int index = (code - 830) / 2;
      *values[index] = std::clamp(int(*values[index]) + direction * 5,
                                 index == 3 ? 10 : 0, 100);
      board::volume(settings.master, settings.music, settings.sfx);
      applyBrightness();
    } else
      settings.screenTimeoutSeconds = std::clamp(
          int(settings.screenTimeoutSeconds) + direction * 30, 0, 1800);
    persistSettings();
    return;
  }
  if (code == 950) {
    if (buttons.empty())
      return;
    for (size_t guard = 0; guard < buttons.size(); guard++) {
      selected = (selected + 1) % buttons.size();
      if (buttonCodes[selected] < 950 &&
          !lv_obj_has_state(buttons[selected], LV_STATE_DISABLED)) {
        selectedCode = buttonCodes[selected];
        lv_group_focus_obj(buttons[selected]);
        break;
      }
    }
    return;
  }
  if (code == 951) {
    if (selected < 0 || selected >= int(buttonCodes.size()) ||
        buttonCodes[selected] != selectedCode)
      focusSelected();
    if (selected >= 0 && selected < int(buttonCodes.size()) &&
        buttonCodes[selected] < 950 &&
        !lv_obj_has_state(buttons[selected], LV_STATE_DISABLED))
      click(buttonCodes[selected]);
    return;
  }
  if (code == 952) {
    setPage("home");
    return;
  }
  if (code >= 100 && code < 200) {
    doAction(Action(code - 100));
    return;
  }
  if (code >= 200 && code < 212) {
    doAction(foods[code - 200].action, &foods[code - 200]);
    return;
  }
  if (code == 314 || code == 316 || code == 318) {
    memoryPairs = code - 310;
    code = 304;
    page = "memorydifficulty";
  }
  if (code == 304 && page != "memorydifficulty") {
    setPage("memorydifficulty");
    return;
  }
  if (code >= 300 && code < 308) {
    if (startGame(state, Game(code - 299), memoryPairs)) {
      for (auto &t : thumbs)
        t.key = -1;
      save();
      gameArea = nullptr;
      setPage("match");
      notice = instructions[code - 300];
    } else
      notice = "Seu amigo precisa descansar ou ainda está no ovo.";
    dirty = true;
    return;
  }
  if (code >= 700 && code <= 740) {
    if (code == 740) {
      auto game = state.match.kind;
      startGame(state, game, state.match.pairs);
      save();
      rebuildGame = true;
      return;
    }
    auto result = play(state, code - 700, rules);
    notice = result.message;
    if (result.changed) {
      save();
      sfx(state.match.finished ? (victory(state.match) ? "vitoria" : "derrota")
          : state.match.kind == Game::Sequence
              ? "nota" + std::to_string(code - 700)
              : "confirmar");
      rebuildGame = true;
    }
    return;
  }
  if (code >= 400 && code < 408) {
    int form = code - 400;
    notice = state.discovered & (1ULL << (albumFamily * 8 + form))
                 ? names[albumFamily][form]
                 : "Pista: descanso, exploração e brincadeiras abrem caminhos "
                   "diferentes.";
    dirty = true;
    return;
  }
  if (code >= 500 && code <= 502) {
    if (code == 502) {
      pending = "classic";
      notice = "Ativar o modo clássico? A falta prolongada de cuidados pode "
               "encerrar o ciclo. O álbum será mantido.";
      setPage("confirm");
    } else {
      state.mode = Mode(code - 500);
      save();
      notice =
          code == 500 ? "Modo tranquilo ativado." : "Modo contínuo ativado.";
      setPage("home");
    }
    return;
  }
#if CONFIG_GOTCHI_BREEDING
  if (code >= 520 && code <= 522) {
    pending = "breed" + std::to_string(code - 520);
    notice =
        "Criar um ovo com estes dois pais no perfil indicado? Ambos precisam "
        "ser adultos, e o destino precisa estar livre ou encerrado.";
    setPage("confirm");
    return;
  }
#endif
  if (code >= 530 && code <= 532) {
    pending = "delete" + std::to_string(code - 530);
    notice = "Apagar este perfil? O ciclo será arquivado antes de liberar o "
             "espaço para um novo ovo.";
    setPage("confirm");
    return;
  }
  if (code >= 510 && code <= 512) {
    save();
    unlink(path("saves/empty" + std::to_string(code - 510)).c_str());
    loadProfile(code - 510);
    saveSettings(path("saves/settings"), settings);
    dirty = true;
    return;
  }
  if (code >= 600 && code < 612) {
    if (state.decorations & (1 << (code - 600))) {
      state.decor = state.favorite = code - 600;
      save();
      notice = "Decoração escolhida: " + decorNames[code - 600];
    } else
      notice = "Conquiste este objeto com convivência e jogos.";
    dirty = true;
    return;
  }
  switch (code) {
  case 1:
    setPage("home");
    break;
  case 10:
    setPage("food");
    break;
  case 11:
    setPage("hygiene");
    break;
  case 12:
    setPage("sleep");
    break;
  case 13:
    setPage("health");
    break;
  case 14:
    setPage("games");
    break;
  case 15:
    setPage("education");
    break;
  case 16:
    setPage("info");
    break;
  case 17:
    setPage("album");
    break;
  case 18:
    setPage("settings");
    break;
  case 19:
    foodPage = (foodPage + 1) % 3;
    dirty = true;
    break;
  case 20:
    state.light = !state.light;
    save();
    dirty = true;
    break;
  case 21:
    state.sleepHour = (state.sleepHour + 1) % 24;
    save();
    dirty = true;
    break;
  case 22:
    state.wakeHour = (state.wakeHour + 1) % 24;
    save();
    dirty = true;
    break;
  case 80:
    setPage("audio");
    break;
  case 81:
    setPage("display");
    break;
  case 82:
    if (page == "name" && nameField && strlen(lv_textarea_get_text(nameField))) {
      snprintf(state.name, sizeof state.name, "%s",
               lv_textarea_get_text(nameField));
      if (save()) setPage("info");
    }
    break;
  case 23:
    nameDraft = state.name;
    nameCursor = LV_TEXTAREA_CURSOR_LAST;
    setPage("name");
    break;
  case 24: {
    char name[160];
    snprintf(name, sizeof name, "%s/album/portrait-%llu-%u.bmp", ROOT, state.id,
             unsigned(state.sequence));
    notice = portrait(name) ? "Retrato salvo no álbum do microSD."
                            : "Não foi possível salvar o retrato.";
    dirty = true;
    break;
  }
  case 25:
    setPage("profiles");
    break;
  case 26:
    pending = "retire";
    notice = "Encerrar este ciclo? O histórico será guardado no álbum, e um "
             "novo ovo chegará após sua confirmação.";
    setPage("confirm");
    break;
  case 27:
    notice = "Pistas: sono e carinho → tranquilidade; quintal → exploração; "
             "jogos variados → sociabilidade.";
    dirty = true;
    break;
  case 28:
    albumFamily = (albumFamily + 1) % rules.familyCount;
    dirty = true;
    break;
  case 29:
    setPage("decor");
    break;
  case 34:
    settings.retro = !settings.retro;
    lastForm = 255;
    break;
  case 35:
    settings.abc = !settings.abc;
    break;
  case 36:
    setPage("more");
    break;
  case 37:
    setPage("modes");
    break;
  case 38:
    board::lockIO();
    notice = store.backup(settings.profile,
                          path("album/backup" +
                               std::to_string(settings.profile) + ".sav"))
                 ? "Backup exportado para tamagotchi/album."
                 : "Falha no backup.";
    board::unlockIO();
    dirty = true;
    break;
  case 39:
    pending = "restore";
    notice = "Restaurar o backup deste perfil? O progresso atual será "
             "substituído pelo backup exportado.";
    setPage("confirm");
    break;
  case 40:
    settings.silentSleep = !settings.silentSleep;
    break;
  case 41:
    settings.tilt = !settings.tilt;
    notice = settings.tilt ? "Reação suave à inclinação ativada."
                           : "Reação à inclinação desligada.";
    break;
  case 42:
    setPage("diary");
    break;
  case 43:
    cancelGame(state);
    save();
    gameArea = nullptr;
    setPage("games");
    break;
  case 44:
  case 45:
    if (state.match.round % 2 == 0) {
      state.match.parity = code - 44;
      save();
      rebuildGame = true;
    }
    break;
  case 48:
    setPage("extras");
    break;
#if CONFIG_GOTCHI_LAN
  case 49:
    setPage("lan");
    break;
  case 60:
    lanField = 0;
    setPage("lanform");
    break;
  case 61:
    lanField = 2;
    setPage("lanform");
    break;
  case 62:
  case 63: {
    visit::Packet packet{};
    packet.kind = code == 62 ? 1 : 2;
    packet.code = remoteCode;
    packet.request = (uint64_t(esp_random()) << 32) | esp_random();
    packet.individual = state.id;
    packet.family = state.family;
    packet.stage = uint8_t(state.stage);
    packet.palette = state.palette;
    packet.decor = state.favorite;
    notice = state.stage != Stage::Egg && lan::send(peerIP.c_str(), packet)
                 ? "Pedido enviado. O outro aparelho precisa aceitar."
                 : "Confira conexão, IP, código e se o amigo já nasceu.";
    dirty = true;
    break;
  }
  case 64:
    acceptVisit();
    dirty = true;
    break;
  case 65:
    hasVisit = false;
    notice = "Visita recusada.";
    dirty = true;
    break;
  case 66:
    lan::disconnect();
    notice = "Desconectando a rede local.";
    dirty = true;
    break;
#endif
  case 46:
    setPage("history");
    break;
  case 47:
    setPage("breeding");
    break;
  case 900:
    if (pending.rfind("delete", 0) == 0) {
      unsigned target = atoi(pending.c_str() + 6);
      board::lockIO();
      bool ok = store.backup(
          target, path("album/deleted-" + std::to_string(target) + "-" +
                       std::to_string(esp_timer_get_time()) + ".sav"));
      if (ok) {
        FILE *mark =
            fopen(path("saves/empty" + std::to_string(target)).c_str(), "wb");
        ok = mark && fputs("empty", mark) >= 0;
        if (mark)
          fclose(mark);
      }
      if (ok)
        for (const char *suffix :
             {"a.sav", "b.sav", "a.sav.tmp", "b.sav.tmp"}) {
          std::string file =
              path("saves/profile" + std::to_string(target) + suffix);
          if (unlink(file.c_str()) && errno != ENOENT)
            ok = false;
        }
      board::unlockIO();
      if (!ok) {
        notice = "Não foi possível apagar; arquivos preservados.";
        setPage("profiles");
        break;
      }
      if (target == settings.profile) {
        profileVacant = true;
        state = {};
        state.profile = target;
        lastForm = 255;
      }
      setPage("profiles");
      break;
    }
#if CONFIG_GOTCHI_BREEDING
    if (pending.rfind("breed", 0) == 0) {
      int pair = atoi(pending.c_str() + 5);
      int first = pair == 2 ? 1 : 0, second = pair == 0 ? 1 : 2,
          target = 3 - first - second;
      auto parents = std::make_unique<State[]>(3);
      board::lockIO();
      auto la = store.load(first, parents[0]),
           lb = store.load(second, parents[1]),
           lt = store.load(target, parents[2]);
      board::unlockIO();
      if (la != Load::Ok || lb != Load::Ok || lt == Load::Corrupt ||
          (lt == Load::Ok && !parents[2].retired && !parents[2].dead) ||
          !compatibleParents(parents[0], parents[1])) {
        notice = "Escolha dois adultos e deixe o terceiro perfil livre ou "
                 "encerrado.";
        setPage("breeding");
        break;
      }
      auto child = std::make_unique<State>();
      offspring(*child, parents[0], parents[1], target, esp_random(),
                (uint64_t(esp_random()) << 32) | esp_random(), rules);
      child->discovered = lt == Load::Ok ? parents[2].discovered : 0;
      board::lockIO();
      bool ok = store.save(*child);
      if (ok) {
        std::string file =
            path("album/parents-" + std::to_string(child->id) + ".json");
        FILE *f = fopen(file.c_str(), "wb");
        if (f) {
          fprintf(f, "{\"child\":\"%llu\",\"parents\":[\"%llu\",\"%llu\"]}\n",
                  child->id, parents[0].id, parents[1].id);
          fflush(f);
          fsync(fileno(f));
          fclose(f);
        }
      }
      board::unlockIO();
      if (ok) {
        save();
        loadProfile(target);
        notice = "Um ovo da nova geração chegou.";
      } else
        notice = "Falha ao salvar o novo ovo; os pais foram preservados.";
      setPage("home");
      break;
    }
#endif
    if (pending == "classic") {
      state.mode = Mode::Classic;
      save();
      setPage("home");
    } else if (pending == "restore") {
      board::lockIO();
      bool ok = store.restore(
          settings.profile,
          path("album/backup" + std::to_string(settings.profile) + ".sav"));
      board::unlockIO();
      if (ok)
        loadProfile(settings.profile);
      notice =
          ok ? "Backup restaurado." : "Backup inválido. Progresso preservado.";
      setPage("home");
    } else if (pending == "retire") {
      state.retired = 1;
      log(state, "Despedida escolhida. Obrigado pela nossa história!");
      if (!save())
        break;
      board::lockIO();
      bool archived =
          store.backup(settings.profile,
                       path("album/life-" + std::to_string(state.id) + ".sav"));
      board::unlockIO();
      if (!archived) {
        fault = true;
        faultText = "Falha ao arquivar o ciclo. O indivíduo foi preservado.";
        dirty = true;
        break;
      }
      auto discovered = state.discovered;
      auto oldDecorations = state.decorations;
      state = newborn(settings.profile, esp_random(),
                      (uint64_t(esp_random()) << 32) | esp_random(), rules);
      state.discovered = discovered;
      state.decorations = oldDecorations;
      save();
      lastForm = 255;
      setPage("home");
    }
    break;
  default:
    break;
  }
  if ((code >= 34 && code <= 35) || code == 40 || code == 41)
    persistSettings();
}
static void diagnostics(bool automatic = false) {
  printf("METRICS fps=%.2f audio_blocks=%u audio_errors=%u save_bytes=%u "
         "internal_min=%u ui_stack_free=%u\n",
         double(measuredFps), board::audioBlocks(), board::audioErrors(),
         unsigned(sizeof(State)),
         unsigned(heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL)),
         unsigned(uxTaskGetStackHighWaterMark(nullptr)));
  auto *j = cJSON_CreateObject();
  if (!j)
    return;
  auto n = [&](const char *key, double value) {
    cJSON_AddNumberToObject(j, key, value);
  };
  cJSON_AddBoolToObject(j, "ready", ready);
  cJSON_AddBoolToObject(j, "fault", fault);
  n("keyboardConnected", keyboardOnline);
  n("keyboardEvents", keyboardEvents);
  cJSON_AddStringToObject(j, "name", state.name);
  if (nameField && page == "name") cJSON_AddStringToObject(j, "nameDraft", lv_textarea_get_text(nameField));
  n("profileVacant", profileVacant);
  n("stateFormat", state.format);
  n("currentPose", currentPose);
  n("checkpointSeconds", checkpointSeconds);
  n("batteryMv", battery);
  n("batteryPercent", power::batteryPercent(battery));
  n("screenOff", screenOff);
  n("screenTimeoutSeconds", settings.screenTimeoutSeconds);
  n("brightness", settings.brightness);
  n("appliedBrightness", appliedBrightness);
  n("profile", settings.profile);
  char id[32];
  snprintf(id, sizeof id, "%llu", state.id);
  cJSON_AddStringToObject(j, "id", id);
  n("family", state.family);
  n("stage", unsigned(state.stage));
  n("form", state.form);
  n("age", state.age);
  n("incubation", state.incubation);
  n("target", state.incubationTarget);
  n("saveSequence", state.sequence);
  cJSON_AddStringToObject(j, "page", page.c_str());
  n("mode", unsigned(state.mode));
  n("sleeping", state.sleeping);
  n("bathStep", state.bathStep);
  n("poop", state.poop);
  n("gameCount", state.gameCount);
  auto &m = state.match;
  n("game", unsigned(m.kind));
  n("gameActive", m.active);
  n("gameFinished", m.finished);
  n("gameRewarded", m.rewarded);
  n("gameWins", m.wins);
  n("gameLosses", m.losses);
  n("gameRound", m.round);
  n("gameBase", m.base);
  n("gameScore", m.score);
  n("memoryFirst", m.first);
  n("memorySecond", m.second);
  n("sequenceShowing", m.showing);
  n("sequenceLength", m.length);
  n("sequenceInput", m.input);
  n("sequenceLit", m.kind == Game::Sequence && m.showing &&
                           m.elapsed / 800 < m.length && m.elapsed % 800 < 500
                       ? m.sequence[m.elapsed / 800]
                       : -1);
  n("fruitX", m.fruitX);
  n("fruitY", m.fruitY);
  n("basket", m.basket);
  n("internalFree", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
  n("psramFree", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  n("largest", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
  n("audioBlocks", board::audioBlocks());
  n("audioErrors", board::audioErrors());
  n("audioResourceErrors", board::audioResourceErrors());
  n("fps", measuredFps);
  n("uiHz", measuredUiHz);
  auto *stats = cJSON_AddArrayToObject(j, "stats");
  for (float v : state.stats)
    cJSON_AddItemToArray(stats, cJSON_CreateNumber(v));
  if (m.kind == Game::Memory) {
    auto *cards = cJSON_AddArrayToObject(j, "memoryShown");
    auto *matched = cJSON_AddArrayToObject(j, "memoryMatched");
    for (int i = 0; i < m.pairs * 2; i++) {
      cJSON_AddItemToArray(
          cards,
          cJSON_CreateNumber(
              m.matched[i] || m.first == i || m.second == i ? m.cards[i] : -1));
      cJSON_AddItemToArray(matched, cJSON_CreateBool(m.matched[i]));
    }
  }
  char *body = cJSON_PrintUnformatted(j);
  if (body) {
    printf("%s %s\n", automatic ? "TELEMETRY" : "GOTCHI", body);
    free(body);
  }
  cJSON_Delete(j);
  fflush(stdout);
}
static void console() {
  static std::string line;
  uint8_t ch;
  while (usb_serial_jtag_read_bytes(&ch, 1, 0) > 0) {
    if (ch == '\n' || ch == '\r') {
      if (line == "gotchi capture") {
        if (board::screenshot(path("album/test-screen.bmp").c_str())) {
          FILE *f = fopen(path("album/test-screen.bmp").c_str(), "rb");
          if (f) {
            fseek(f, 0, SEEK_END);
            long n = ftell(f);
            rewind(f);
            printf("GOTCHI_BINARY %ld\n", n);
            fflush(stdout);
            uint8_t block[4096];
            size_t count;
            while ((count = fread(block, 1, sizeof block, f)) > 0)
              usb_serial_jtag_write_bytes(block, count, pdMS_TO_TICKS(10000));
            fclose(f);
          }
        } else {
          printf("GOTCHI {\"captureError\":true}\n");
        }
      } else if (line.rfind("gotchi tap ", 0) == 0) {
        int x = 0, y = 0;
        if (sscanf(line.c_str() + 11, "%d %d", &x, &y) == 2) {
          testPoint.x = std::clamp(x, 0, 1279);
          testPoint.y = std::clamp(y, 0, 719);
          testRelease = esp_timer_get_time() + 150000;
          printf("GOTCHI {\"tap\":true}\n");
        }
      } else if (line.rfind("gotchi key ", 0) == 0) {
        unsigned modifiers = 0, usage = 0;
        bool ok = sscanf(line.c_str() + 11, "%u %u", &modifiers, &usage) == 2 && modifiers <= 255 && usage <= 255;
        if (ok) nameKey(modifiers, usage);
        printf("GOTCHI {\"key\":%s}\n", ok ? "true" : "false");
      } else if (line == "gotchi status")
        diagnostics();
      else if (line == "gotchi screenshot") {
        bool ok = board::screenshot(path("album/test-screen.bmp").c_str());
        printf("GOTCHI {\"screenshot\":%s}\n", ok ? "true" : "false");
      } else if (line == "gotchi launcher") {
        save();
        board::launcher();
      } else if (line.rfind("gotchi click ", 0) == 0) {
        click(atoi(line.c_str() + 13));
        printf("GOTCHI {\"clicked\":true}\n");
      } else if (line == "gotchi save") {
        printf("GOTCHI {\"saved\":%s}\n", save() ? "true" : "false");
      }
      line.clear();
    } else if (line.size() < 200 && ch >= 32)
      line += char(ch);
  }
}
// A164 register protocol: EVENT_NUM is a queue count; reading HID_EVENT pops it.
// https://docs.m5stack.com/en/tab5/Tab5_Keyboard
static void nameKey(uint8_t modifiers, uint8_t usage) {
  auto key = keyboardDecoder.decode(modifiers, usage);
  if (key.kind == keyboard::Kind::Ignore) return;
  bool waking = screenOff;
  displayActivity();
  if (waking || !nameField || page != "name") return;
  switch (key.kind) {
  case keyboard::Kind::Save: click(82); break;
  case keyboard::Kind::Cancel: setPage("info"); break;
  case keyboard::Kind::Backspace: lv_textarea_delete_char(nameField); break;
  case keyboard::Kind::Delete: lv_textarea_delete_char_forward(nameField); break;
  case keyboard::Kind::Left: lv_textarea_cursor_left(nameField); break;
  case keyboard::Kind::Right: lv_textarea_cursor_right(nameField); break;
  case keyboard::Kind::Home: lv_textarea_set_cursor_pos(nameField, 0); break;
  case keyboard::Kind::End: lv_textarea_set_cursor_pos(nameField, LV_TEXTAREA_CURSOR_LAST); break;
  case keyboard::Kind::Clear: lv_textarea_set_text(nameField, ""); break;
  case keyboard::Kind::Text: {
    char text[] = {key.text, 0}; lv_textarea_add_text(nameField, text); break;
  }
  default: break;
  }
}
static void physicalKeyboard() {
  static i2c_master_bus_handle_t bus = nullptr;
  static i2c_master_dev_handle_t dev = nullptr;
  static int64_t retry = 0, pollAt = 0;
  int64_t now = esp_timer_get_time();
  if (now < pollAt || now < retry) return;
  pollAt = now + 20000;
  auto online = [](bool value) {
    keyboardOnline = value;
    if (keyboardLabel) lv_label_set_text(keyboardLabel, value ? "Teclado conectado" : "Procurando teclado encaixado…");
  };
  auto disconnect = [&] {
    if (dev) i2c_master_bus_rm_device(dev);
    dev = nullptr; retry = now + 1000000; keyboardDecoder.reset(); online(false);
  };
  if (!bus) {
    i2c_master_bus_config_t cfg{};
    cfg.i2c_port = BSP_I2C_NUM == 0 ? I2C_NUM_1 : I2C_NUM_0;
    cfg.sda_io_num = GPIO_NUM_0; cfg.scl_io_num = GPIO_NUM_1;
    cfg.clk_source = I2C_CLK_SRC_DEFAULT; cfg.glitch_ignore_cnt = 7;
    cfg.flags.enable_internal_pullup = true;
    if (i2c_new_master_bus(&cfg, &bus) != ESP_OK) { retry = now + 1000000; return; }
  }
  auto read = [&](uint8_t reg, uint8_t *data, size_t count) {
    return i2c_master_transmit_receive(dev, &reg, 1, data, count, 50);
  };
  auto write = [&](uint8_t reg, uint8_t value) {
    uint8_t data[] = {reg, value}; return i2c_master_transmit(dev, data, 2, 50);
  };
  if (!dev) {
    i2c_device_config_t cfg{};
    cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7; cfg.device_address = 0x6d; cfg.scl_speed_hz = 100000;
    if (i2c_master_bus_add_device(bus, &cfg, &dev) != ESP_OK) { retry = now + 1000000; return; }
    uint8_t address = 0;
    // An address-only probe can time out on this STM32. Use its real ID register.
    if (read(0xff, &address, 1) != ESP_OK || address != 0x6d ||
        write(0x10, 1) != ESP_OK || write(0x00, 2) != ESP_OK ||
        write(0x02, 0) != ESP_OK || write(0x01, 0) != ESP_OK) { disconnect(); return; }
    keyboardDecoder.reset(); retry = 0; online(true);
  }
  // Drain a bounded batch; IRQ acknowledgement does not clear queued events.
  for (unsigned i = 0; i < 8; ++i) {
    uint8_t count = 0, data[2]{};
    if (read(0x02, &count, 1) != ESP_OK) { disconnect(); return; }
    if (!count) break;
    if (count > 32 || read(0x30, data, 2) != ESP_OK) { disconnect(); return; }
    if (!data[1] || (data[0] == 0xff && data[1] == 0xff)) continue;
    ++keyboardEvents;
    nameKey(data[0], data[1]);
  }
  if (write(0x01, 0) != ESP_OK) disconnect();
}
static void step(lv_timer_t *) {
  uiTicks++;
  int64_t now = esp_timer_get_time();
  if (now - fpsStart >= 10000000) {
    measuredFps = renderedFrames * 1000000.f / (now - fpsStart);
    measuredUiHz = uiTicks * 1000000.f / (now - fpsStart);
    uiTicks = 0;
    renderedFrames = 0;
    fpsStart = now;
  }
  uint32_t dt = last ? std::min<int64_t>((now - last) / 1000, 3600000) : 0;
  last = now;
  console();
  physicalKeyboard();
  static unsigned lastAudioFault = 0;
  unsigned audioFault = board::audioResourceErrors();
  if (audioFault != lastAudioFault) {
    lastAudioFault = audioFault;
    notice = "Recurso de áudio indisponível ou corrompido. Confira o pacote; detalhes na USB.";
    dirty = true;
  }
  if (now - lastBattery >= 15000000) {
    battery = board::batteryMv();
    if (!screenOff)
      refreshBatteryLabels();
    lastBattery = now;
  }
  // Blank only the backlight: the BSP's display sleep also disables touch.
  // Keep simulation/checkpoints running, but avoid rendering invisible frames.
  bool playing = ready && !fault && !profileVacant && state.match.active;
  if (playing)
    lv_display_trigger_activity(board::display);
  if (!screenOff && power::shouldBlank(
                        lv_display_get_inactive_time(board::display),
                        settings.screenTimeoutSeconds, playing)) {
    if (bsp_display_backlight_off() == ESP_OK) {
      screenOff = true;
      appliedBrightness = 0;
      // A full-screen target also wakes above disabled buttons or empty areas.
      wakeShield = lv_obj_create(lv_layer_top());
      lv_obj_remove_style_all(wakeShield);
      lv_obj_set_pos(wakeShield, 0, 0);
      lv_obj_set_size(wakeShield, LV_PCT(100), LV_PCT(100));
      lv_obj_remove_flag(wakeShield, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_add_flag(wakeShield, LV_OBJ_FLAG_CLICKABLE);
    }
  }
  if (now - metrics > 30000000) {
    diagnostics(true);
    metrics = now;
  }
  if (profileVacant) {
    if (dirty)
      render();
    return;
  }
  if (!ready || fault) {
    if (dirty)
      render();
    return;
  }
#if CONFIG_GOTCHI_LAN
  static std::string lastNetworkState;
  auto networkState = lan::status();
  if (page == "lan" && networkState != lastNetworkState) {
    lastNetworkState = networkState;
    dirty = true;
  }
  visit::Packet arriving{};
  if (lan::poll(arriving) && arriving.family < rules.familyCount &&
      arriving.individual != state.id) {
    pendingVisit = arriving;
    hasVisit = true;
    pendingUntil = now + 60000000;
    notice = "Pedido de visita local. Abra Ajustes > Extensões > Visitas LAN "
             "para aceitar.";
    dirty = true;
  }
  if (hasVisit && now > pendingUntil)
    hasVisit = false;
  if (visitorVisible && now > visitUntil) {
    visitorVisible = false;
    dirty = true;
  }
#endif
  if (now - sdProbeAt > 2000000) {
    sdProbeAt = now;
    board::lockIO();
    FILE *probe = fopen(path("manifest.json").c_str(), "rb");
    bool good = probe && fgetc(probe) == '{';
    if (probe)
      fclose(probe);
    board::unlockIO();
    if (!good) {
      fault = true;
      faultText = "O microSD não está respondendo. A simulação foi pausada. "
                  "Confira o cartão e retorne pelo launcher.";
      board::stopAudio();
      dirty = true;
      render();
      return;
    }
  }
  uint8_t oldStage = uint8_t(state.stage), oldForm = state.form,
          oldReward = state.match.rewarded;
  bool oldDead = state.dead;
  bool oldSleeping = state.sleeping;
  auto oldAction = state.action;
  tick(state, dt, rules);
  if (oldSleeping != bool(state.sleeping)) {
    save();
    notice = "Soneca concluída. Pronto para brincar!";
    dirty = true;
  }
  if (!oldDead && state.dead) {
    save();
    notice = "O ciclo chegou ao fim. A história está preservada no álbum.";
    dirty = true;
  }
  if (oldAction == Action::Explore && state.action != oldAction &&
      page == "home")
    dirty = true;
  static int lastHour = -1;
  time_t wallNow = board::wall();
  tm local{};
  localtime_r(&wallNow, &local);
  if (wallNow > 1704067200) {
    if (lastHour >= 0 && lastHour != local.tm_hour &&
        state.stage != Stage::Egg && !state.match.active) {
      if (local.tm_hour == state.sleepHour && !state.sleeping)
        doAction(Action::Sleep);
      else if (local.tm_hour == state.wakeHour && state.sleeping)
        doAction(Action::Wake);
    }
    lastHour = local.tm_hour;
  }
  if (state.match.active)
    tickGame(state, dt, rules);
  if (uint8_t(state.stage) != oldStage || state.form != oldForm) {
    save();
    lastForm = 255;
    notice = "Um momento especial: nascimento ou evolução!";
    nowSim = now;
    sfx(oldStage == uint8_t(Stage::Egg) ? "eclosao" : "evolucao");
    if (oldStage == uint8_t(Stage::Egg))
      voiceAt = now + 1000000;
    dirty = true;
  }
  if (state.match.rewarded != oldReward) {
    save();
    sfx(victory(state.match) ? "vitoria" : "derrota");
    rebuildGame = true;
  }
  if (now - checkpoint > int64_t(checkpointSeconds) * 1000000) {
    save();
    checkpoint = now;
  }
  if (voiceAt && now >= voiceAt) {
    board::voice(path(std::string("audio/sfx/") + families[state.family] +
                      "_chamado.wav"));
    voiceAt = 0;
  }
  if (screenOff)
    return;
  for (int i = 0; i < 4; i++)
    if (homeBars[i]) {
      lv_bar_set_value(homeBars[i], int(state.stats[i]), LV_ANIM_OFF);
      char line[64];
      snprintf(line, sizeof line, "%s  %d%%",
               (const char *[]){"Saciedade", "Humor", "Energia", "Higiene"}[i],
               int(state.stats[i]));
      if (strcmp(lv_label_get_text(homeLabels[i]), line))
        lv_label_set_text(homeLabels[i], line);
    }
  if (clockLabel) {
    char t[16];
    if (wallNow > 1704067200)
      strftime(t, sizeof t, "%H:%M", &local);
    else
      snprintf(t, sizeof t, "--:--");
    if (strcmp(lv_label_get_text(clockLabel), t)) lv_label_set_text(clockLabel, t);
  }
  if (night) {
    int minutes = local.tm_hour * 60 + local.tm_min;
    int target = !state.light || state.sleeping ? 100 : wallNow < 1704067200 ? 0 :
                 minutes < 330 || minutes >= 1110 ? 32 :
                 minutes < 390 ? (390 - minutes) * 32 / 60 :
                 minutes >= 1050 ? (minutes - 1050) * 32 / 60 : 0;
    if (dayOpacity != target) {
      dayOpacity = dayOpacity < 0 ? target : dayOpacity + (target > dayOpacity ? 1 : -1);
      lv_obj_set_style_bg_opa(night, dayOpacity, 0);
    }
  }
  if (dirty) {
    gameArea = nullptr;
    render();
  }
  frame = now / 160000;
  if (petImage && !bank.empty()) {
    unsigned pose = animationFrame();
    if (pose != currentPose) {
      currentPose = pose;
      imageDescriptor(petDsc, activeAsset().width, activeAsset().height,
                      bank.data() + pose * activeAsset().width * activeAsset().height * 3, true);
      lv_image_set_src(petImage, &petDsc);
    }
    if (page == "match")
      lv_obj_set_pos(petImage, 752, 85);
    else
      lv_obj_set_pos(petImage,
                     360 + (48 - activeAsset().anchorX) * 3 + (settings.tilt ? board::tiltOffset() : 0) +
                         (state.action == Action::Explore
                              ? int((now / 80000) % 70) - 35
                              : 0),
                     (page == "home" ? 210 : 258) + (81 - activeAsset().anchorY) * 3);
  }
  if (state.pendingEvolution && nowSim + 2000000 < uint64_t(now)) {
    state.pendingEvolution--;
    lastForm = 255;
    dirty = true;
    nowSim = now;
    save();
  }
  if (page == "match" && gameArea) {
    auto &m = state.match;
    int show =
        m.showing
            ? (m.elapsed / 800 < m.length
                   ? int(m.elapsed / 800) * 2 + (m.elapsed % 800 < 500 ? 0 : 1)
                   : -2)
            : -1;
    if (rebuildGame || lastPhase != int(m.phase > 0) || lastFirst != m.first ||
        lastShow != show) { // Rebuild only a game layer, keeping the scene and
                            // pet buffers stable.
      renderGame();
    }
    if (m.kind == Game::Fruit && fruitObj) {
      lv_obj_set_pos(fruitObj, m.fruitX * 830 / 640 + 10 - 16, m.fruitY + 135);
      lv_obj_set_pos(basketObj, m.basket * 830 / 640 + 10 - 48, 415);
      char txt[96];
      snprintf(txt, sizeof txt, "Frutas: %d • %u segundos", int(m.score),
               unsigned(m.elapsed >= 30000 ? 0 : 30 - m.elapsed / 1000));
      if (strcmp(lv_label_get_text(gameStatus), txt)) lv_label_set_text(gameStatus, txt);
    }
  }
  if (state.stage == Stage::Egg && status) {
    char t[100];
    auto remain = state.incubationTarget - state.incubation;
    snprintf(t, sizeof t, "Ovo surpresa • eclosão em %u:%02u",
             unsigned(remain / 60000), unsigned(remain / 1000 % 60));
    lv_label_set_text(status, t);
  }
}
void start() {
  bsp_display_lock(0);
  screen = lv_screen_active();
  lv_obj_set_style_bg_color(screen, color(0xf3eddf), 0);
  lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_text_font(screen, LV_FONT_DEFAULT, 0);
  group = lv_group_create();
  lv_group_set_default(group);
  faultText = board::sdReady ? "Pacote não carregado."
                             : "MicroSD ausente ou ilegível. Insira o cartão "
                               "com a pasta tamagotchi.";
  ready = board::sdReady && content();
  if (ready) {
    board::assetValidator(validateAudioAsset);
    loadSettings(path("saves/settings"), settings);
    lv_obj_set_style_text_font(screen, &font, 0);
    loadProfile(settings.profile);
  }
  battery = board::batteryMv();
  lastBattery = esp_timer_get_time();
  applyBrightness();
  render();
  last = checkpoint = nowSim = esp_timer_get_time();
  usb_serial_jtag_driver_config_t c{};
  c.rx_buffer_size = 512;
  c.tx_buffer_size = 4096;
  usb_serial_jtag_driver_install(&c);
  testPointer = lv_indev_create();
  lv_indev_set_type(testPointer, LV_INDEV_TYPE_POINTER);
  lv_indev_set_display(testPointer, board::display);
  lv_indev_set_read_cb(testPointer, [](lv_indev_t *, lv_indev_data_t *d) {
    d->point.x = testPoint.y;
    d->point.y = 1279 - testPoint.x;
    d->state = esp_timer_get_time() < testRelease ? LV_INDEV_STATE_PRESSED
                                                  : LV_INDEV_STATE_RELEASED;
  });
  for (auto *input = lv_indev_get_next(nullptr); input;
       input = lv_indev_get_next(input)) {
    if (lv_indev_get_display(input) != board::display)
      continue;
    lv_indev_add_event_cb(input, [](lv_event_t *e) {
      if (!screenOff)
        return;
      displayActivity();
      auto *input = static_cast<lv_indev_t *>(lv_event_get_target(e));
      // Consume the entire first gesture, including a long press/release.
      lv_indev_stop_processing(input);
      lv_indev_wait_release(input);
    }, LV_EVENT_PRESSED, nullptr);
  }
  lv_display_trigger_activity(board::display);
  fpsStart = esp_timer_get_time();
  lv_display_add_event_cb(
      board::display, [](lv_event_t *) { renderedFrames++; },
      LV_EVENT_REFR_READY, nullptr);
  // Headroom above 30 fps: LVGL schedules the next period after rendering work.
  lv_timer_set_period(lv_display_get_refr_timer(board::display), 16);
  lv_timer_create(step, 16, nullptr);
  bsp_display_unlock();
}
} // namespace ui
