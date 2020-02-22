/**
 * Hashlife
 * Tests for basic Game of Life cells.
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

#include "catch2/catch.hpp"

#include "cells.hpp"

#include <iostream>

using namespace life;

TEST_CASE("Cell construction", "[cells-constructor]") {
  auto empty = cells{"$$$$$$$$"};
  auto block = cells{"$$$...**...$...**...$$$$"};

  REQUIRE(empty == cells{});
  REQUIRE(empty == cells{0});
  REQUIRE(block == cells{0x0000001818000000ull});
}

TEST_CASE("Life rules", "[life-rules]") {
  SECTION("Block of 2x2 living cells shall not change over generations") {
    auto block = cells{"$$$...**...$...**...$$$$"};
    REQUIRE(block.step() == block);
    REQUIRE(block.next() == block);
  }

  SECTION("Empty cell block shall remain empty") {
    auto empty = cells{"$$$$$$$$"};
    REQUIRE(empty.step() == empty);
    REQUIRE(empty.next() == empty);
  }

  SECTION("Blinker shall rotate 90 degrees every step") {
    auto blinker = cells{"$$.***$$$$$$"};
    auto rotated = cells{"$..*$..*$..*$$$$$"};
    REQUIRE(blinker.step() == rotated);
    REQUIRE(rotated.step() == blinker);
  }

  SECTION("Glider shall move one block each four generations") {
    auto glider = cells{"$$...*$..*$..***$$$$"};
    auto moved_glider = cells{"$$$..*$.*$.***$$$"};
    REQUIRE(glider.step().step().step().step() == moved_glider);
  }
}