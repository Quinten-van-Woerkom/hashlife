/**
 * Hashlife
 * A block of 8x8 cells, the basic unit of computation of the life rules.
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

#include "cells.hpp"

#include <bitset>
#include <functional>
#include <iostream>

#include "bitwise.hpp"

using namespace life;

/**
 * Cells can be initialised according to Tomas Rokicki's hashlife cell format.
 * Each block of cells is a single line, consisting of three characters:
 *  '.':  Dead cell
 *  '*':  Living cell
 *  '$':  Start of next row, non-mentioned cells are dead
 */
cells::cells(std::string_view format) noexcept {
  auto row = 0u, column = 0u;
  for (const auto& character : format) {
    if (character == '*') set(bitmap, column++ + row*columns);
    if (character == '.') ++column;
    if (character == '$') column = 0, row += 1;
  }
}

/**
 * Equality is based on comparison of the underlying bitmap.
 */
auto cells::operator==(const cells& other) const noexcept -> bool {
  return bitmap == other.bitmap;
}

/**
 * Inequality is based on comparison of the underlying bitmap.
 */
auto cells::operator!=(const cells& other) const noexcept -> bool {
  return bitmap != other.bitmap;
}

/**
 * Uses std::hash instead of identity to allow for non-64-bit systems.
 * It is likely that this results in identity on 64-bit systems anyway.
 */
auto cells::hash() const noexcept -> std::size_t {
  return std::hash<std::uint64_t>()(bitmap);
}

/**
 * Determines whether or not the cell located at the given position is alive or
 * dead. The position (0, 0) is located at the top-left, with locations
 * positive down and to the right.
 */
auto cells::operator()(std::size_t x, std::size_t y) const noexcept -> bool {
  return (bitmap >> (x + y*columns)) & 1ull;
}

/**
 * Returns the next generation of cells, i.e. its state 2 steps into the
 * future. Implemented as repeated calls to step(), since that is highly
 * optimised already. All but the middle 4x4 square is disregarded, since only
 * those cells are known for sure.
 */
auto cells::next() const noexcept -> cells {
  return cells{step().step().bitmap & 0x00003c3c3c3c0000ull};
}

/**
 * Returns the state of the cells one step into the future.
 * Applies the normal Game of Life rules, i.e. a cell lives if:
 *  1. It is alive and has 3 living neighbours.
 *  2. Exactly three cells in its neighbourhood (including itself) are alive.
 * Implemented in terms of 64-bit register operations to be as fast as
 * possible. See Tony Finch's "Life in a Register" for more information.
 */
auto cells::step() const noexcept -> cells {
  const auto [sum1, sum2, sum4] = this->neighbours();
  const auto case1 = bitmap & (~sum1 & ~sum2 & sum4); // Alive and 3 neighbours
  const auto case2 = sum1 & sum2 & ~sum4; // Total of 3 cells in neighbourhood
  constexpr auto  mask = 0x007e7e7e7e7e7e00ull; // Edge cells are unknown.
  return cells{(case1 | case2) & mask};
}

/**
 * Determines the number of living cells contained in this square.
 * Implemented as bitset::count(), as this is likely implemented in terms of
 * built-in popcount() in most compilers.
 * MSVC does not seem to use built-ins; for now, we ignore this, as we don't
 * use popcount in any performance-critical code.
 */
auto cells::population_count() const noexcept -> std::size_t {
  return std::bitset<64>(bitmap).count();
}

/**
 * Computes the number of neighbours a cell has, returning it as a 3-bit
 * value, encoded in three bitmaps. Each location in a bitmap represents that
 * bit in the number of neighbours of the cell located there.
 * We neglect the counts of 8 and 9 neighbouring cells, since they behave
 * identically to 0 and 1 neighbours, respectively. As such, we just allow
 * overflow.
 */
auto cells::neighbours() const noexcept -> std::array<std::uint64_t, 3> {
  const auto left = bitmap << 1;
  const auto right = bitmap >> 1;
  const auto [mid1, mid2] = full_add(left, bitmap, right);

  const auto up1 = mid1 << columns;
  const auto up2 = mid2 << columns;
  const auto down1 = mid1 >> columns;
  const auto down2 = mid2 >> columns;

  const auto [sum1, sum2a] = full_add(up1, mid1, down1);
  const auto [sum2b, sum4a] = full_add(up2, mid2, down2);
  const auto [sum2, sum4b] = half_add(sum2a, sum2b);
  const auto sum4 = sum4a ^ sum4b;

  return {sum1, sum2, sum4};
}

/**
 * Prints the cell block to the output stream.
 * Living cells are printed as '*', dead as '.'.
 */
auto operator<<(std::ostream& os, const cells& other) -> std::ostream& {
  for (auto i = 0u; i < cells::rows; ++i) {
    for (auto j = 0u; j < cells::columns; ++j) {
      if (other(j, i)) os << '*';
      else os << '.';
    }
    os << '\n';
  }
  return os;
}