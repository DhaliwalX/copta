#ifndef INTERP_NATIVE_FUNCTIONS_H_
#define INTERP_NATIVE_FUNCTIONS_H_

#include "jast/handle.h"
#include <vector>
namespace jast {
namespace internal {

class Object;
Ref<Object> print(std::vector<Ref<Object>> &args);

}
}

#endif
