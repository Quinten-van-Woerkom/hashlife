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

#include "bitwise.hpp"

TEST_CASE("Obtaining bits", "[bit]") {
  REQUIRE(bit(0b100u, 2) == true);
  REQUIRE(bit(0b100u, 3) == false);
  REQUIRE(bit(0b100u, 1) == false);
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