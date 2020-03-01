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

TEST_CASE("Allocation works as expected for an array", "[array-allocation]") {
  auto arena = memory_arena<int>{10};
  REQUIRE(arena.size() == 10);

  auto allocator = arena_allocator{arena};
  int* array = allocator.allocate(9);
  REQUIRE(array != nullptr);
  int* fails = allocator.allocate(11);
  REQUIRE(fails == nullptr);
}