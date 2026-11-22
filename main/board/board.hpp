#pragma once
#include "bsp/m5stack_tab5.h"
#include <string>
namespace board {
extern bool sdReady;
extern lv_display_t *display;
bool init();
void lockIO();
void unlockIO();
bool screenshot(const char *, lv_obj_t *target = nullptr);
void sensorInit();
int tiltOffset();
int64_t wall();
int batteryMv();
void launcher();
void audioInit();
void voice(const std::string &);
void stopAudio();
unsigned audioBlocks();
unsigned audioErrors();
void music(const std::string &);
void sound(const std::string &);
void volume(int master, int music, int effects);
void mute(bool);
} // namespace board
