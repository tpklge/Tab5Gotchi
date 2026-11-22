#pragma once
#include "visit_protocol.hpp"
#include <string>
namespace lan {
void connect(const char *ssid, const char *password, uint32_t ownCode);
bool send(const char *ip, const visit::Packet &);
bool poll(visit::Packet &);
std::string status();
bool online();
void disconnect();
} // namespace lan
