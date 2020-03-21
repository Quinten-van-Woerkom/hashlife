/**
 * Hashlife
 * Tests for macrocell implementation.
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

#include "macrocell.hpp"

#include <iostream>
#include <vector>

using namespace life;

TEST_CASE("Pointers type as expected", "[pointer]") {
  SECTION("Pointer is null only when initialised to nullptr") {
    pointer null = nullptr;
    pointer fill = 3;
    pointer zero = (std::size_t)0;

    REQUIRE(!null);
    REQUIRE(fill);
    REQUIRE(zero);
  }

  SECTION("Pointer retains value it is initialised to") {
    pointer ptr = 3;
    REQUIRE(ptr.index() == 3);
  }

  SECTION("Pointers with same indices compare equal") {
    pointer a = 5;
    pointer b = 5;
    pointer c = 6;
    REQUIRE(a == b);
    REQUIRE(a != c);
  }
}