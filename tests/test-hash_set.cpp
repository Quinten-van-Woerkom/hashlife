/**
 * Hashlife
 * Tests for hash table implementation.
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

#include "hash_set.hpp"

#include <iostream>
#include <vector>

using namespace life;

TEST_CASE("Arena allocation works as expected", "[arena-allocation]") {
  auto arena = memory_arena<int>{10};
  REQUIRE(arena.size() == 10);

  SECTION("Arena shall be non-full upon construction") {
    REQUIRE(!arena.full());
  }

  SECTION("Fitting allocation shall not fail") {
    auto array = arena.allocate(9);
    REQUIRE(array != nullptr);
  }

  SECTION("Fitting allocation shall result in non-full arena") {
    arena.allocate(9);
    REQUIRE(!arena.full());
  }

  SECTION("Exactly-fitting allocation should not fail") {
    auto array = arena.allocate(10);
    REQUIRE(array != nullptr);
  }

  SECTION("Arena shall be full after exactly-fitting allocation") {
    arena.allocate(10);
    REQUIRE(arena.full());
  }

  SECTION("Allocations bigger than the available arena memory shall fail") {
    auto fails = arena.allocate(11);
    REQUIRE(fails == nullptr);
  }
}