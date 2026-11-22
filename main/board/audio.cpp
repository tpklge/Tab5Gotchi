#include "board.hpp"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
namespace board {
static SemaphoreHandle_t guard;
static std::string wantedMusic, wantedSfx, wantedVoice;
static int masterVol = 55, musicVol = 30, sfxVol = 65;
static std::atomic<bool> muted{false}, cutAudio{false};
static std::atomic<unsigned> blocks{0}, errors{0};
unsigned audioBlocks() { return blocks.load(); }
unsigned audioErrors() { return errors.load(); }
void music(const std::string &p) {
  if (!guard)
    return;
  xSemaphoreTake(guard, portMAX_DELAY);
  wantedMusic = p;
  xSemaphoreGive(guard);
}
void sound(const std::string &p) {
  if (!guard)
    return;
  xSemaphoreTake(guard, portMAX_DELAY);
  wantedSfx = p;
  xSemaphoreGive(guard);
}
void voice(const std::string &p) {
  if (!guard)
    return;
  xSemaphoreTake(guard, portMAX_DELAY);
  wantedVoice = p;
  xSemaphoreGive(guard);
}
void stopAudio() {
  if (!guard)
    return;
  xSemaphoreTake(guard, portMAX_DELAY);
  wantedMusic.clear();
  wantedSfx.clear();
  wantedVoice.clear();
  cutAudio = true;
  xSemaphoreGive(guard);
}
void volume(int m, int b, int s) {
  if (!guard)
    return;
  xSemaphoreTake(guard, portMAX_DELAY);
  masterVol = m;
  musicVol = b;
  sfxVol = s;
  xSemaphoreGive(guard);
}
void mute(bool b) { muted = b; }
static FILE *openWav(const std::string &p) {
  if (p.empty())
    return nullptr;
  FILE *f = fopen(p.c_str(), "rb");
  if (!f)
    return nullptr;
  uint8_t h[44];
  if (fread(h, 1, 44, f) != 44 || memcmp(h, "RIFF", 4) ||
      memcmp(h + 8, "WAVE", 4) || h[20] != 1 || h[22] != 1 || h[34] != 16 ||
      h[24] != 0x22 || h[25] != 0x56 || h[26] != 0 || h[27] != 0) {
    fclose(f);
    return nullptr;
  }
  return f;
}
static void task(void *) {
  auto codec = bsp_audio_codec_speaker_init();
  if (!codec) {
    vTaskDelete(nullptr);
    return;
  }
  esp_codec_dev_sample_info_t fmt{};
  fmt.sample_rate = 22050;
  fmt.channel = 2;
  fmt.bits_per_sample = 16;
  if (esp_codec_dev_open(codec, &fmt) != ESP_CODEC_DEV_OK) {
    vTaskDelete(nullptr);
    return;
  }
  esp_codec_dev_set_out_vol(codec, 75);
  FILE *bg = nullptr, *fx = nullptr, *vc = nullptr;
  std::string current;
  static int16_t a[512], b[512], c[512], out[1024];
  float level = 0;
  while (true) {
    xSemaphoreTake(guard, portMAX_DELAY);
    std::string desired = wantedMusic, effect = wantedSfx, vocal = wantedVoice;
    wantedSfx.clear();
    wantedVoice.clear();
    float mv = masterVol / 100.f, bv = musicVol / 100.f, sv = sfxVol / 100.f;
    xSemaphoreGive(guard);
    bool switching = desired != current;
    level = std::clamp(level + (switching || muted ? -.12f : .12f), 0.f, 1.f);
    lockIO();
    if (cutAudio.exchange(false)) {
      if (bg)
        fclose(bg);
      if (fx)
        fclose(fx);
      if (vc)
        fclose(vc);
      bg = fx = vc = nullptr;
      current.clear();
      level = 0;
    }
    if (switching && level == 0) {
      if (bg)
        fclose(bg);
      bg = openWav(desired);
      current = desired;
    }
    if (!effect.empty()) {
      if (fx)
        fclose(fx);
      fx = openWav(effect);
    }
    if (!vocal.empty()) {
      if (vc)
        fclose(vc);
      vc = openWav(vocal);
    }
    memset(c, 0, sizeof c);
    memset(a, 0, sizeof a);
    memset(b, 0, sizeof b);
    if (bg) {
      size_t n = fread(a, 2, 512, bg);
      if (n < 512) {
        if (ferror(bg)) {
          fclose(bg);
          bg = nullptr;
        } else {
          fseek(bg, 44, SEEK_SET);
          fread(a + n, 2, 512 - n, bg);
        }
      }
    }
    if (fx && fread(b, 2, 512, fx) < 512) {
      fclose(fx);
      fx = nullptr;
    }
    if (vc && fread(c, 2, 512, vc) < 512) {
      fclose(vc);
      vc = nullptr;
    }
    unlockIO();
    for (int i = 0; i < 512; i++) {
      int sample = int((a[i] * bv * level + b[i] * sv + c[i] * sv * .8f) * mv);
      out[i * 2] = out[i * 2 + 1] = std::clamp(sample, -30000, 30000);
    }
    if (esp_codec_dev_write(codec, out, sizeof out) == ESP_CODEC_DEV_OK)
      blocks++;
    else
      errors++;
  }
}
void audioInit() {
  guard = xSemaphoreCreateMutex();
  xTaskCreate(task, "gotchi_audio", 12288, nullptr, 4, nullptr);
}
} // namespace board
