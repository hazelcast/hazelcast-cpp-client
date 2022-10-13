#include <type_traits>

#pragma once

namespace hazelcast {
namespace util {
template<typename CheckedT, typename EnabledT = void>
using enable_if_rvalue_ref_trait =
  typename std::enable_if<std::is_rvalue_reference<CheckedT>::value,
                          EnabledT>::type;
}
} // namespace hazelcast