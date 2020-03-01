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
  auto empty = cells::empty_square();
  auto block = cells{"$$$...**...$...**...$$$$"};

  REQUIRE(empty == cells{});
  REQUIRE(empty == cells{0});
  REQUIRE(block == cells{0x0000001818000000ull});
}

TEST_CASE("Life rules", "[life-rules]") {
  SECTION("Still-live patterns shall not change over generations") {
    auto empty = cells::empty_square();
    auto block = cells::block();
    auto beehive = cells::beehive();
    auto loaf = cells::loaf();
    auto boat = cells::boat();
    auto tub = cells::tub();

    REQUIRE(empty.step() == empty);
    REQUIRE(empty.next() == empty);
    REQUIRE(block.step() == block);
    REQUIRE(block.next() == block);
    REQUIRE(beehive.step() == beehive);
    REQUIRE(beehive.next() == beehive);
    REQUIRE(loaf.step() == loaf);
    REQUIRE(loaf.next() == loaf);
    REQUIRE(boat.step() == boat);
    REQUIRE(boat.next() == boat);
    REQUIRE(tub.step() == tub);
    REQUIRE(tub.next() == tub);
  }

  SECTION("Oscillators shall be periodic") {
    auto blinker = cells::blinker();
    auto toad = cells::toad();
    auto beacon = cells::beacon();

    REQUIRE(blinker.step() != blinker);
    REQUIRE(blinker.step().step() == blinker);
    REQUIRE(toad.step() != toad);
    REQUIRE(toad.step().step() == toad);
    REQUIRE(beacon.step() != beacon);
    REQUIRE(beacon.step().step() == beacon);
  }

  SECTION("Glider shall move one block each four generations") {
    auto glider = cells{"$$...*$..*$..***$$$$"};
    auto moved_glider = cells{"$$$..*$.*$.***$$$"};
    REQUIRE(glider.step().step().step().step() == moved_glider);
  }
}

TEST_CASE("Population count", "[popcount]") {
  REQUIRE(cells::empty_square().population_count() == 0);
  REQUIRE(cells::blinker().population_count() == 3);
  REQUIRE(cells::block().population_count() == 4);
  REQUIRE(cells::glider().population_count() == 5);
}

TEST_CASE("Cell square combinations", "[cell-combine]") {
  auto filled = cells::filled();
  REQUIRE(filled == cells::center(filled, filled, filled, filled));
  REQUIRE(filled == cells::horizontal(filled, filled));
  REQUIRE(filled == cells::vertical(filled, filled));
}