#ifndef DEBUG_INFO_H_
#define DEBUG_INFO_H_

#include "jast/source-locator.h"
#include "jast/token.h"

#include <iosfwd>

namespace jast {

class DebugInfo {
public:
  DebugInfo() = default;
  DebugInfo(const Position &start, const Position &end)
    : start_{ start }, end_{ end }
  { }

  Position &start();
  Position &end();

  void print(std::ostream &os) const;
  void dump() const;

private:
  Position start_;
  Position end_;
};

}

#endif
