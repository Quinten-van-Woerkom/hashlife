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

    SECTION("Initialises empty") {
        REQUIRE(set.find(3) == set.capacity());
    }

    SECTION("Emplacing should guarantuee find") {
        set.emplace(3);
        auto index = set.find(3);

        REQUIRE(index != set.capacity());
        REQUIRE(set[index] == 3);
    }

    SECTION("Emplacing more than five elements should fail after the fifth") {
        auto success1 = set.emplace(1);
        auto success2 = set.emplace(2);
        auto success3 = set.emplace(3);
        auto success4 = set.emplace(4);
        auto success5 = set.emplace(5);
        auto fail6 = set.emplace(6);

        REQUIRE(success1 != set.capacity());
        REQUIRE(success2 != set.capacity());
        REQUIRE(success3 != set.capacity());
        REQUIRE(success4 != set.capacity());
        REQUIRE(success5 != set.capacity());
        REQUIRE(fail6 == set.capacity());

        REQUIRE(set.find(1) != set.capacity());
        REQUIRE(set.find(2) != set.capacity());
        REQUIRE(set.find(3) != set.capacity());
        REQUIRE(set.find(4) != set.capacity());
        REQUIRE(set.find(5) != set.capacity());
        REQUIRE(set.find(6) == set.capacity());
    }
}