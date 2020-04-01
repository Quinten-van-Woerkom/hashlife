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

#include "hash_set.hpp"

#include <array>
#include <iostream>

TEST_CASE("Static vector works as expected") {
    static_vector<int> test{3};

    SECTION("Size sticks") {
        REQUIRE(test.size() == 3);
        REQUIRE(!test.empty());
    }

    SECTION("Element assignment sticks") {
        test[0] = 42;
        REQUIRE(test[0] == 42);
    }

    SECTION("Vector assignment sticks") {
        static_vector<int> assign{5};
        assign[3] = 65;
        test = assign;

        REQUIRE(test.size() == 5);
        REQUIRE(test[3] == 65);
        REQUIRE(std::equal(test.begin(), test.end(), assign.begin()));
    }

    SECTION("Allow iteration") {
        static_vector<int> a{3};
        a[0] = 1; a[1] = 3; a[2] = 4;

        std::copy(a.begin(), a.end(), test.begin());
        REQUIRE(std::equal(a.begin(), a.end(), test.begin()));
    }

    SECTION("Value-initialized") {
        static_vector<int> a{1000, 1};
        for (auto element : a)
            REQUIRE(element == 1);
    }
}

TEST_CASE("Hash-set works as expected") {
    auto set = dense_hash_set<int>{5};

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