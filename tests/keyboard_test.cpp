#include "ui/keyboard.hpp"
#include <cassert>
#include <iostream>
using namespace ui::keyboard;
int main() {
 Decoder d;
 assert(d.decode(0,4).text=='a'); assert(d.decode(2,4).text=='A');
 d.decode(0,0x39); assert(d.decode(0,4).text=='A'); assert(d.decode(2,4).text=='a');
 assert(d.decode(0,0x27).text=='0'); assert(d.decode(2,0x27).text==')');
 assert(d.decode(0,0x2d).text=='-'); assert(d.decode(2,0x2d).text=='_');
 assert(d.decode(0,0x34).text=='\''); assert(d.decode(2,0x34).text=='"');
 assert(d.decode(0,0x38).text=='/'); assert(d.decode(2,0x38).text=='?');
 assert(d.decode(1,4).kind==Kind::Clear); assert(d.decode(1,6).kind==Kind::Ignore);
 assert(d.decode(4,4).kind==Kind::Ignore); assert(d.decode(0xff,0xff).kind==Kind::Ignore);
 assert(d.decode(0,0).kind==Kind::Ignore);
 assert(d.decode(0,0x28).kind==Kind::Save); assert(d.decode(0,0x58).kind==Kind::Save);
 assert(d.decode(0,0x29).kind==Kind::Cancel); assert(d.decode(0,0x4c).kind==Kind::Delete);
 assert(d.decode(0,0x2a).kind==Kind::Backspace); assert(d.decode(0,0x50).kind==Kind::Left);
 assert(d.decode(0,0x4a).kind==Kind::Home); assert(d.decode(0,0x4d).kind==Kind::End);
 d.reset(); assert(d.decode(0,4).text=='a');
 std::cout << "PASS: HID, maiúsculas, símbolos, edição e atalhos de nome\n";
}
