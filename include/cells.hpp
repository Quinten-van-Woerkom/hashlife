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

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string_view>

namespace life {
/**
 * Collection of 8x8 life cells.
 */
class cells {
public:
  static constexpr std::size_t columns = 8;
  static constexpr std::size_t rows = 8;

  cells(std::string_view format) noexcept;
  cells(std::uint64_t bitmap) noexcept : bitmap{bitmap} {}

  cells() noexcept = default;
  cells(const cells&) = default;
  cells(cells&&) = default;

  auto operator==(const cells&) const noexcept -> bool;
  auto operator!=(const cells&) const noexcept -> bool;
  auto hash() const noexcept -> std::size_t;

  auto operator()(std::size_t x, std::size_t y) const noexcept -> bool;
  auto next() const noexcept -> cells;
  auto step() const noexcept -> cells;
  auto population_count() const noexcept -> std::size_t;
  auto empty() const noexcept -> bool;

  static auto empty_square() noexcept { return cells{"$$$$$$$$"}; }
  static auto block() noexcept { return cells{"$$$...**...$...**...$$$$"}; }
  static auto beehive() noexcept { return cells{"$$$...**$..*..*$...**$$$"}; }
  static auto loaf() noexcept { return cells{"$$...**$..*..*$...*.*$....*$$$"}; }
  static auto boat() noexcept { return cells{"$$$..**$..*.*$...*$$$"}; }
  static auto tub() noexcept { return cells{"$$$...*$..*.*$...*$$$"}; }

  static auto blinker() noexcept { return cells{"$$.***$$$$$$"}; }
  static auto toad() noexcept { return cells{"$$$...***$..***$$$$"}; }
  static auto beacon() noexcept { return cells{"$$..**$..**$....**$....**$$$"}; }

  static auto glider() noexcept { return cells{"$$...*$..*$..***$$$$"}; }

private:
  auto neighbours() const noexcept -> std::array<std::uint64_t, 3>;

  std::uint64_t bitmap = 0;
};
}

auto operator<<(std::ostream& os, const life::cells& other) -> std::ostream&;

namespace std {
  template<>
  struct hash<life::cells> {
    constexpr auto operator()(const life::cells& cells) const noexcept {
      return cells.hash();
    }
  };
}