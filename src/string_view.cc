#include "string_view.h"

std::ostream &operator<<(std::ostream &os, const string_view &s) {
  os << s.string();
  return os;
}
