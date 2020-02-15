/**
 * Hashlife - bitwise.hpp
 * Implementation of useful bitwise operations.
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

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <tuple>

/**
 * Short-hand for SFINAE on determining if a set of types is unsigned.
 * Useful to make sure we are actually working on unsigned types, since the
 * bitwise behaviour of signed types can be surprising, and we shouldn't need
 * it for our Hashlife implementation.
 */
template<typename... T>
using requires_unsigned = std::enable_if_t<std::conjunction_v<std::is_unsigned<T>...>>;

/**
 * Returns the bit located at <index> in <value>.
 * Any bits outside of the range of the size of value are given to be false.
 */
template<typename Unsigned, typename = requires_unsigned<Unsigned>>
constexpr auto bit(Unsigned value, std::size_t index) noexcept -> bool {
  return (value >> index) & 1u;
}

/**
 * Sets the bit at <index> in <value>.
 * If <index> is bigger than the number of bits in Unsigned, the operation is a
 * no-op.
 */
template<typename Unsigned, typename = requires_unsigned<Unsigned>>
constexpr void set(Unsigned& value, std::size_t index) noexcept {
  value |= (Unsigned(1) << index);
}

/**
 * Bitmask-based half-adder, allowing for simultaneous adding of all bits in
 * two unsigned values. Returns a pair representing the sum and carry bits.
 */
template<typename Unsigned1, typename Unsigned2,
  typename = requires_unsigned<Unsigned1, Unsigned2>>
constexpr auto half_add(Unsigned1 left, Unsigned2 right) noexcept {
  return std::pair{left ^ right, left & right};
}

/**
 * Bitmask-based full-adder, allowing for simultaneous adding of all bits in
 * two unsigned values, supporting carries. Returns a pair representing the sum
 * and carry bits in the result.
 */
template<typename Unsigned1, typename Unsigned2, typename Unsigned3,
  typename = requires_unsigned<Unsigned1, Unsigned2, Unsigned3>>
constexpr auto full_add(Unsigned1 left, Unsigned2 right, Unsigned3 carry) noexcept {
  const auto sum = left ^ right ^ carry;
  const auto result_carry = (left & right) | (left & carry) | (right & carry);
  return std::pair{sum, result_carry};
}