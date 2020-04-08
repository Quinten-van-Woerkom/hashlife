/**
 * Hashlife
 * Tests for hash set implementation.
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

#include "dense_set.hpp"

#include <array>
#include <iostream>

TEST_CASE("Hash-set works as expected") {
  auto set = dense_set<int>{5};

  SECTION("A set shall never have capacity 0") {
    REQUIRE_THROWS(dense_set<int>{0});
  }

  SECTION("Size initialises to 0") { REQUIRE(set.size() == 0); }

  SECTION("Initialises with empty slots") { REQUIRE(set.find(3) == set.end()); }

  SECTION("Emplacing should guarantuee find") {
    set.emplace(3);
    auto location = set.find(3);

    REQUIRE(location != set.end());
    REQUIRE(*location == 3);
  }

  SECTION("Emplacing more than five elements should fail after the fifth") {
    auto success1 = set.emplace(1);
    auto success2 = set.emplace(2);
    auto success3 = set.emplace(3);
    auto success4 = set.emplace(4);
    auto success5 = set.emplace(5);

    REQUIRE(set.size() == 5);
    auto fail6 = set.emplace(6);
    REQUIRE(set.size() == 5);

    REQUIRE(success1.second);
    REQUIRE(success2.second);
    REQUIRE(success3.second);
    REQUIRE(success4.second);
    REQUIRE(success5.second);
    REQUIRE(!fail6.second);

    REQUIRE(set.find(1) != set.end());
    REQUIRE(set.find(2) != set.end());
    REQUIRE(set.find(3) != set.end());
    REQUIRE(set.find(4) != set.end());
    REQUIRE(set.find(5) != set.end());
    REQUIRE(set.find(6) == set.end());
  }

  SECTION("Container shall allow testing after copying") {
    set.emplace(1);
    const auto test = set;
    auto success = test.find(1);
    auto fail = test.find(2);

    REQUIRE(success != test.end());
    REQUIRE(fail == test.end());
    REQUIRE(set.size() == test.size());
  }

  SECTION("Iterator difference shall represent pointer distance") {
    REQUIRE(set.end() - set.begin() == set.capacity());
  }
}