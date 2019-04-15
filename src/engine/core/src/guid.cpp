#include <core/guid.h>
// Windows specific stuff
#include <combaseapi.h>
#include <core/log.h>

Guid Guid::make_new() {
  Guid guid;
  HRESULT hr = CoCreateGuid((GUID *)&guid);
  if (hr != S_OK)
    DEBUG_LOG(System, Error, "Couldn't create GUID.");
  return guid;
}

const unsigned int &Guid::operator[](int index) const {
  if (index > 3 || index < 0) {
    DEBUG_LOG(System, Error, "Indvalid guid index: %d", index);
    return a;
  }

  switch (index) {
  case 0:
  default:
    return a;
  case 1:
    return b;
  case 2:
    return c;
  case 3:
    return d;
  }
}

size_t Guid::get_hash() const {
  const GUID &guid = *(GUID *)this;
  const std::uint64_t *p = reinterpret_cast<const std::uint64_t *>(&guid);
  std::hash<std::uint64_t> hash;
  return hash(p[0]) ^ hash(p[1]);
}

Archive &operator<<(Archive &archive, Guid &guid) {
  return archive << guid.a << guid.b << guid.c << guid.d;
}
