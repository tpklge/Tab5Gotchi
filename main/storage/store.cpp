#include "store.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
namespace gotchi {
struct Header {
  uint32_t magic, version, bytes, crc;
};
uint32_t crc32(const void *p, size_t n) {
  uint32_t c = 0xffffffff;
  auto *b = (const uint8_t *)p;
  for (size_t i = 0; i < n; i++) {
    c ^= b[i];
    for (int j = 0; j < 8; j++)
      c = (c >> 1) ^ (0xedb88320u & uint32_t(-int(c & 1)));
  }
  return ~c;
}
static bool read(const std::string &p, void *data, size_t n) {
  FILE *f = fopen(p.c_str(), "rb");
  if (!f)
    return false;
  Header h{};
  bool ok = fread(&h, 1, sizeof h, f) == sizeof h && h.magic == 0x31475442 &&
            h.version == 1 && h.bytes == n && fread(data, 1, n, f) == n &&
            fgetc(f) == EOF && crc32(data, n) == h.crc;
  fclose(f);
  return ok;
}
static bool write(const std::string &p, const void *data, size_t n) {
  std::string temp = p + ".tmp";
  FILE *f = fopen(temp.c_str(), "wb");
  if (!f)
    return false;
  Header h{0x31475442, 1, uint32_t(n), crc32(data, n)};
  bool ok = fwrite(&h, 1, sizeof h, f) == sizeof h &&
            fwrite(data, 1, n, f) == n && fflush(f) == 0;
  if (ok)
    ok = fsync(fileno(f)) == 0;
  if (fclose(f))
    ok = false;
  std::vector<uint8_t> check(n);
  if (!ok || !read(temp, check.data(), n))
    return false; // Only the older slot is ever replaced.
  // FatFs rename does not replace an existing destination. The other A/B
  // slot remains valid while this older slot is removed and replaced.
  struct stat destination{};
  if (stat(p.c_str(), &destination) == 0 && remove(p.c_str()) != 0)
    return false;
  if (rename(temp.c_str(), p.c_str()) != 0)
    return false;
  return read(p, check.data(), n);
}
static bool exists(const std::string &p) {
  struct stat st{};
  return stat(p.c_str(), &st) == 0;
}
Load Store::load(uint32_t p, State &s) {
  auto scratch = std::make_unique<State[]>(2);
  State &a = scratch[0];
  State &b = scratch[1];
  std::string base = root + "/profile" + std::to_string(p);
  bool va = read(base + "a.sav", &a, sizeof a) && valid(a, p),
       vb = read(base + "b.sav", &b, sizeof b) && valid(b, p);
  if (!va && !vb)
    return exists(base + "a.sav") || exists(base + "b.sav") ||
                   exists(base + "a.sav.tmp") || exists(base + "b.sav.tmp")
               ? Load::Corrupt
               : Load::Missing;
  s = va && (!vb || a.sequence >= b.sequence) ? a : b;
  return Load::Ok;
}
bool Store::save(State &s) {
  if (!valid(s, s.profile))
    return false;
  auto scratch = std::make_unique<State[]>(2);
  State &previous = scratch[0];
  auto loaded = load(s.profile, previous);
  if (loaded == Load::Corrupt)
    return false;
  if (loaded == Load::Ok && previous.id != s.id && !previous.retired &&
      !previous.dead)
    return false;
  State &copy = scratch[1];
  copy = s;
  copy.sequence = (loaded == Load::Ok ? previous.sequence : s.sequence) + 1;
  std::string path = root + "/profile" + std::to_string(s.profile) +
                     (copy.sequence % 2 ? "a.sav" : "b.sav");
  if (!write(path, &copy, sizeof copy))
    return false;
  s.sequence = copy.sequence;
  return true;
}
bool inspectArchive(const std::string &path, State &s) {
  return read(path, &s, sizeof s) && valid(s, s.profile);
}
bool Store::backup(uint32_t p, const std::string &dst) {
  auto scratch = std::make_unique<State>();
  State &s = *scratch;
  return load(p, s) == Load::Ok && write(dst, &s, sizeof s);
}
static bool preserveBeforeRestore(const std::string &root, uint32_t profile) {
  std::string dir = root + "/recovery";
  if (mkdir(dir.c_str(), 0775) != 0 && errno != EEXIST)
    return false;
  std::string destination;
  bool made = false;
  for (unsigned i = 0; i < 10000; i++) {
    destination = dir + "/profile" + std::to_string(profile) + "-restore" +
                  std::to_string(i);
    if (mkdir(destination.c_str(), 0775) == 0) {
      made = true;
      break;
    }
    if (errno != EEXIST)
      return false;
  }
  if (!made)
    return false;
  std::vector<uint8_t> block(4096);
  for (const char *suffix : {"a.sav", "b.sav", "a.sav.tmp", "b.sav.tmp"}) {
    std::string file = "profile" + std::to_string(profile) + suffix,
                source = root + "/" + file;
    if (!exists(source))
      continue;
    FILE *in = fopen(source.c_str(), "rb");
    FILE *out = fopen((destination + "/" + file).c_str(), "wb");
    if (!in || !out) {
      if (in)
        fclose(in);
      if (out)
        fclose(out);
      return false;
    }
    bool ok = true;
    size_t n;
    while ((n = fread(block.data(), 1, block.size(), in)) > 0)
      if (fwrite(block.data(), 1, n, out) != n) {
        ok = false;
        break;
      }
    if (ferror(in) || fflush(out) != 0 || fsync(fileno(out)) != 0)
      ok = false;
    fclose(in);
    if (fclose(out) != 0)
      ok = false;
    if (!ok)
      return false;
  }
  return true;
}
bool Store::restore(uint32_t p, const std::string &src) {
  auto scratch = std::make_unique<State[]>(2);
  State &incoming = scratch[0];
  if (!read(src, &incoming, sizeof incoming) || !valid(incoming, p))
    return false;
  State &old = scratch[1];
  auto loaded = load(p, old);
  if (!preserveBeforeRestore(root, p))
    return false;
  incoming.sequence = loaded == Load::Ok ? old.sequence + 1 : 1;
  return write(root + "/profile" + std::to_string(p) +
                   (incoming.sequence % 2 ? "a.sav" : "b.sav"),
               &incoming, sizeof incoming);
}
bool loadSettings(const std::string &p, Settings &s) {
  Settings a{};
  bool ok = read(p + "a.cfg", &a, sizeof a);
  if (!ok)
    ok = read(p + "b.cfg", &a, sizeof a);
  if (!ok || a.version != 1 || a.profile > 2 || a.master > 100 ||
      a.music > 100 || a.sfx > 100 || a.brightness > 100)
    return false;
  s = a;
  return true;
}
bool saveSettings(const std::string &p,
                  const Settings &s) { // Keep previous known-good copy first.
  Settings old{};
  if (loadSettings(p, old) && !write(p + "b.cfg", &old, sizeof old))
    return false;
  return write(p + "a.cfg", &s, sizeof s);
}
} // namespace gotchi
