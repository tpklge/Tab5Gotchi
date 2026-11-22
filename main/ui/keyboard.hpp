#pragma once
#include <cstdint>
namespace ui::keyboard {
enum class Kind { Ignore, Text, Save, Cancel, Backspace, Delete, Left, Right, Home, End, Clear };
struct Key { Kind kind = Kind::Ignore; char text = 0; };
// USB HID usage table used by the A164 in HID mode. No host/device dependencies.
class Decoder {
  bool caps = false;
public:
  void reset() { caps = false; }
  Key decode(uint8_t modifiers, uint8_t usage) {
    if (!usage || usage == 0xff || usage <= 3) return {};
    if (usage == 0x39) { caps = !caps; return {}; }
    if (modifiers & 0x11) return usage == 0x04 ? Key{Kind::Clear} : Key{};
    if (modifiers & 0xcc) return {}; // Alt/GUI shortcuts are not text.
    bool shift = modifiers & 0x22;
    if (usage >= 4 && usage <= 29)
      return {Kind::Text, char(((shift != caps) ? 'A' : 'a') + usage - 4)};
    if (usage >= 30 && usage <= 39)
      return {Kind::Text, (shift ? "!@#$%^&*()" : "1234567890")[usage - 30]};
    if (usage >= 0x2c && usage <= 0x38) {
      constexpr char plain[] = " -=[]\\#;'`,./";
      constexpr char shifted[] = " _+{}|~:\"~<>?";
      return {Kind::Text, (shift ? shifted : plain)[usage - 0x2c]};
    }
    switch (usage) {
    case 0x28: case 0x58: return {Kind::Save};
    case 0x29: return {Kind::Cancel};
    case 0x2a: return {Kind::Backspace};
    case 0x4c: return {Kind::Delete};
    case 0x50: return {Kind::Left};
    case 0x4f: return {Kind::Right};
    case 0x4a: return {Kind::Home};
    case 0x4d: return {Kind::End};
    default: return {};
    }
  }
};
} // namespace ui::keyboard
