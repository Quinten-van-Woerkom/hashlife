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

#include "static_vector.hpp"

#include <algorithm>

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
    a[0] = 1;
    a[1] = 3;
    a[2] = 4;

    std::copy(a.begin(), a.end(), test.begin());
    REQUIRE(std::equal(a.begin(), a.end(), test.begin()));
  }

  SECTION("Value-initialized") {
    static_vector<int> a{10, 1};
    for (auto element : a)
      REQUIRE(element == 1);
  }

  SECTION("Swappable") {
    static_vector<int> a{10, 2};
    static_vector<int> b{20, 1};
    auto c = b;
    auto *d = b.data();
    a.swap(b);
    REQUIRE(std::equal(a.begin(), a.end(), c.begin()));
    REQUIRE(a.data() == d);
  }
}