/**
 * Hashlife
 * Tests for bitwise operations.
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

#include <cstddef>
#include <cstdint>

#include "bitwise.hpp"

TEST_CASE("Obtaining bits", "[bit]") {
  auto value = 0b0100u;

  SECTION("Bits should match value") {
    REQUIRE(bit(value, 0) == false);
    REQUIRE(bit(value, 1) == false);
    REQUIRE(bit(value, 2) == true);
    REQUIRE(bit(value, 3) == false);
  }

  SECTION("Bits past end of integer are 0") {
    REQUIRE(bit(value, sizeof(value)) == false);
  }
}

TEST_CASE("Setting bits", "[set-bit]") {
  auto value = 0b1000u;
  
  SECTION("Should set a bit to true") {
    set(value, 1);
    REQUIRE(bit(value, 1) == true);
  }

  SECTION("Setting a bit should not affect others") {
    set(value, 1);
    REQUIRE(bit(value, 2) == false);
    REQUIRE(bit(value, 3) == true);
  }
}

TEST_CASE("Half-adding", "[half-add]") {
  REQUIRE(half_add(0u, 0u) == std::pair{0u, 0u});
  REQUIRE(half_add(1u, 0u) == std::pair{1u, 0u});
  REQUIRE(half_add(0u, 1u) == std::pair{1u, 0u});
  REQUIRE(half_add(1u, 1u) == std::pair{0u, 1u});
}

TEST_CASE("Full-adding", "[full-add]") {
  REQUIRE(full_add(0u, 0u, 0u) == std::pair{0u, 0u});
  REQUIRE(full_add(0u, 0u, 1u) == std::pair{1u, 0u});
  REQUIRE(full_add(0u, 1u, 0u) == std::pair{1u, 0u});
  REQUIRE(full_add(0u, 1u, 1u) == std::pair{0u, 1u});
  REQUIRE(full_add(1u, 0u, 0u) == std::pair{1u, 0u});
  REQUIRE(full_add(1u, 0u, 1u) == std::pair{0u, 1u});
  REQUIRE(full_add(1u, 1u, 0u) == std::pair{0u, 1u});
  REQUIRE(full_add(1u, 1u, 1u) == std::pair{1u, 1u});
}