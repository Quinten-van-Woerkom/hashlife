/**
 * Hashlife - bitwise.hpp
 * Implementation of simple hash functionality.
 *
 * Copyright 2020 Quinten van Woerkom
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <functional>

/**
 * Hash-combination function that allows computing of the hashes of different
 * objects.
 */
namespace internal {
template <typename T, typename... Rest>
void hash_combine(std::size_t &seed, const T &v,
                  const Rest &... rest) noexcept {
  seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  (hash_combine(seed, rest), ...);
}
} // namespace internal

template <typename... T> auto variadic_hash(const T &... arguments) noexcept {
  std::size_t seed = 42;
  internal::hash_combine(seed, arguments...);
  return seed;
}

/**
 * Macro that defines std::hash<type> for type <type>, calling a member function
 * type::hash() to obtain the value of this hash.
 */
#define HASHLIFE_DEFINE_HASH(type)                                             \
  namespace std {                                                              \
  template <> struct hash<type> {                                              \
    auto operator()(const type &object) const noexcept {                       \
      return object.hash();                                                    \
    }                                                                          \
  };                                                                           \
  }
