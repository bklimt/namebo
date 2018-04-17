#include "string_view.h"

std::ostream &operator<<(std::ostream &os, const string_view &s) {
  os << s.ToString();
  return os;
}
