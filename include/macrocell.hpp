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

#include <cstdint>
#include <limits>

namespace life {
/**
 * Pointer to a macrocell or cell square one layer down in the life universe.
 * For now, just an index, but in the future might exploit the hash set
 * structure or anything in other ways.
 * A nullptr is indicated by a std::uint32_t::max value.
 */
class pointer {
public:
  constexpr pointer(std::nullptr_t = nullptr) noexcept : offset{std::numeric_limits<std::uint32_t>::max()} {}
  constexpr pointer(std::size_t offset) noexcept : offset{(std::uint32_t)offset} {}

  constexpr auto index() const noexcept -> std::size_t { return static_cast<std::size_t>(offset); }
  constexpr operator bool() const noexcept { return offset != std::numeric_limits<std::uint32_t>::max(); }

  constexpr bool operator==(const pointer& other) const noexcept { return offset == other.offset; }
  constexpr bool operator!=(const pointer& other) const noexcept { return !(*this == other); }

private:
  std::uint32_t offset;
};

/**
 * A macrocell is made up of four quadrants, which can be either other
 * macrocells, or actual cell squares, depending on the level of the macrocell.
 */
class macrocell {
public:

private:
  pointer nw, ne, sw, se;
};
}