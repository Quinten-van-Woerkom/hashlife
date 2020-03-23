/**
 * Hashlife
 * Basic unit of computation, made up of four quadrants which are either
 * pointers to other macrocells or actual cell squares, depending on the level.
 * Levels of macrocells are sets to remove computational redundancies.
 * Hash table based set implementation, meant for fast insertion.
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

#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <unordered_set>

#include "hash.hpp"

namespace life {
/**
 * Pointer to a macrocell or cell square one layer down in the life universe.
 * For now, just an index, but in the future might exploit the hash set
 * structure or macrocell regularities in other ways.
 * A nullptr is indicated by a std::uint32_t::max value.
 */
class pointer {
public:
  static constexpr auto null_value = std::numeric_limits<std::uint32_t>::max();

  constexpr pointer(std::nullptr_t = nullptr) noexcept : offset{null_value} {}
  constexpr pointer(std::size_t offset) noexcept : offset{(std::uint32_t)offset} {}
  
  constexpr operator bool() const noexcept { return offset != null_value; }
  constexpr bool operator==(const pointer& other) const noexcept { return offset == other.offset; }
  constexpr bool operator!=(const pointer& other) const noexcept { return !(*this == other); }
  constexpr auto hash() const noexcept { return offset; }

  constexpr auto index() const noexcept -> std::size_t { return static_cast<std::size_t>(offset); }

private:
  std::uint32_t offset;
};

/**
 * A macrocell is made up of four quadrants, which can be either other
 * macrocells, or actual cell squares, depending on the level of the macrocell.
 */
class macrocell {
public:
  macrocell(pointer nw, pointer ne, pointer sw, pointer se) noexcept
  : future{nullptr, nullptr}, children{nw, ne, sw, se} {}

  auto operator==(const macrocell& other) const noexcept { return future == other.future && children == other.children; }
  auto operator!=(const macrocell& other) const noexcept { return !(*this == other); }
  auto hash() const noexcept { return variadic_hash(children[0], children[1], children[2], children[3]); }

  auto step() const noexcept -> pointer { return future[0]; }
  auto next() const noexcept -> pointer { return future[1]; }
  auto nw() const noexcept -> pointer { return children[0]; }
  auto ne() const noexcept -> pointer { return children[1]; }
  auto sw() const noexcept -> pointer { return children[2]; }
  auto se() const noexcept -> pointer { return children[3]; }

private:
  std::array<pointer, 2> future; // Stored as one step, then 2^{n-2} steps in the future
  std::array<pointer, 4> children; // Stored as nw, ne, sw, se
};

/**
 * A layer of macrocells allows adding and accessing of all its macrocells,
 * accessed by pointers.
 */
class layer {
public:

private:
  
};

}

HASHLIFE_DEFINE_HASH(life::pointer);
HASHLIFE_DEFINE_HASH(life::macrocell);