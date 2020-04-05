/**
 * Hashlife
 * Hash table based set implementation, meant for fast insertion.
 * Based on the premise that insertion is allowed to fail, and that no
 * deletions occur. Keyhis allows us to neglect tombstones, and mean we have no
 * need for robin-hood-like reordering techniques.
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
 * WIKeyHOUKey WARRANKeyIES OR CONDIKeyIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <algorithm>
#include <tuple>

#include "static_vector.hpp"

/**
 * Hashlife requires a rather specialized hash table, requiring open addressing
 * and stability of reference. In addition, the knowledge that no deletions
 * occur (in principle garbage cleaning is necessary, but better implemented
 * as a full hash table reset) can be exploited since no tombstones are
 * necessary.
 */
template<typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
class dense_set {
public:
    using key_type = Key;
    using value_type = Key;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using hash_type = std::invoke_result_t<hasher, key_type>;
    using key_equal = KeyEqual;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    static constexpr bool is_nothrow_swappable = std::is_nothrow_swappable_v<hasher> && std::is_nothrow_swappable_v<key_equal>;

    /**************************************************************************
     * Iterator into the hashmap.
     * Basically an index, considering the hashmap supports open addressing.
     */
    struct iterator {
        constexpr iterator(const dense_set& owner, size_type index) noexcept
         : owner{&owner}, index{index} {}
        
        constexpr iterator(const iterator&) = default;
        constexpr auto operator=(const iterator&) -> iterator& = default;

        /**
         * Increments the index until a valid object is found, i.e. where the
         * sentinel indicates an object exists.
         */
        constexpr auto operator++() noexcept -> iterator& {
            ++index;
            while (owner->_sentinels[index].empty())
                ++index;
            return *this;
        }

        constexpr auto operator++(int) noexcept -> iterator {
            auto return_value = *this;
            ++index;
            while (owner->_sentinels[index].empty())
                ++index;
            return return_value;
        }

        /**
         * Equality comparators.
         */
        constexpr auto operator==(iterator other) const noexcept {
            return owner == other.owner && index == other.index;
        }

        constexpr auto operator!=(iterator other) const noexcept {
            return !(*this == other);
        }

        /**
         * Dereferences the iterator, returning the object at the current index.
         * Assumes that an object exists at this index.
         */
        constexpr auto operator*() const noexcept -> const_reference {
            return (*owner)[index];
        }

        const dense_set* owner;
        size_type index;
    };

    using const_iterator = iterator;


    /**************************************************************************
     * Constructors
     */
    dense_set(size_type count);
    dense_set(const dense_set&) = default;
    constexpr dense_set(dense_set&&) noexcept(is_nothrow_swappable) = default;


    /**************************************************************************
     * Assignment operators
     */
    auto operator=(const dense_set&) -> dense_set& = default;
    constexpr auto operator=(dense_set&&) noexcept(is_nothrow_swappable) -> dense_set& = default;


    /**************************************************************************
     * Iterators
     */
    constexpr auto begin() noexcept { return iterator{*this, 0}; }
    constexpr auto begin() const noexcept { return const_iterator{*this, 0}; }
    constexpr auto end() noexcept { return iterator{*this, max_size()}; }
    constexpr auto end() const noexcept { return const_iterator{*this, max_size()}; }
    constexpr auto cbegin() const noexcept { return const_iterator{*this, 0}; }
    constexpr auto cend() const noexcept { return const_iterator{*this, max_size()}; }


    /**************************************************************************
     * Capacity
     */
    auto empty() const noexcept { return std::all_of(_sentinels.begin(), _sentinels.end(), [](auto a) { return a.empty(); }); }
    auto size() const noexcept { return std::count(_sentinels.begin(), _sentinels.end(), [](auto a) { return a.filled(); }); }
    auto max_size() const noexcept { return _elements.max_size(); }


    /**************************************************************************
     * Modifiers
     */
    void clear() noexcept;
    auto insert(const value_type& value) -> std::pair<iterator, bool>;
    auto insert(value_type&& value) -> std::pair<iterator, bool>;
    void swap(dense_set&& other) noexcept(is_nothrow_swappable) { std::swap(*this, other); }

    template<typename... Args>
    auto emplace(Args&&... args) noexcept -> std::pair<iterator, bool> {
        auto object = Key{args...};
        auto location = find(object);
        if (location != end()) return {location, false};

        auto hash = hasher()(object);
        location = iterator{*this, hash % max_size()};
        location = probe(location);

        if (location == end()) return {location, false};

        auto reduced_hash = (std::uint8_t) (hash >> (8*sizeof(hash) - 7)) & 0xef;
        _sentinels[location.index].colonize(reduced_hash);
        _elements[location.index] = std::move(object);
        return {location, true};
    }


    /**************************************************************************
     * Lookup
     */
    auto operator[](size_type index) noexcept -> Key&;
    auto operator[](size_type index) const noexcept -> const Key&;
    auto count(const Key& key) const noexcept -> size_type;
    auto find(const Key& key) const noexcept -> iterator;
    auto contains(const Key& key) const noexcept -> bool;
    auto probe(iterator location) const noexcept -> iterator;

private:
    /**
     * Piece of metadata that stores whether or not an element is present at a
     * location, and the 7 high bits of the hash, if this is the case.
     * This allows for faster comparison by also allowing hash-comparison
     * without actually entering the table.
     */
    class sentinel {
    public:
        constexpr sentinel() noexcept : _filled{false}, _reduced_hash{0x00} {}
        void colonize(hash_type reduced_hash) noexcept;
        bool filled() const noexcept { return _filled; }
        bool empty() const noexcept { return !filled(); }
        bool matches(hash_type reduced_hash) const noexcept;

    private:
        bool _filled : 1;
        hash_type _reduced_hash : 7;
    };

    static_vector<Key> _elements;
    static_vector<sentinel> _sentinels;
};


/******************************************************************************
 * Constructors
 */
/**
 * Constructs an empty hash table of size <count>.
 * Note that all sentinels must be value-initialized.
 */
template<typename Key, typename Hash, typename KeyEqual>
dense_set<Key, Hash, KeyEqual>::dense_set(std::size_t count)
 : _elements{count}, _sentinels{count, sentinel{}} {}


/******************************************************************************
 * Lookup
 */
/**
 * Indexing is checked in debug mode to ensure that accessed elements actually
 * exist.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::operator[](std::size_t index) noexcept -> Key& {
    assert(index < max_size() && "dense_set: Index access out of bound");
    assert(_sentinels[index].filled() && "dense_set: Trying to access non-existent element");
    return _elements[index];
}

/**
 * Indexing is checked in debug mode to ensure that accessed elements actually
 * exist.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::operator[](std::size_t index) const noexcept -> const Key& {
    assert(index < max_size() && "dense_set: Index access out of bound");
    assert(_sentinels[index].filled() && "dense_set: Trying to access non-existent element");
    return _elements[index];
}

/**
 * Returns the number of elements matching the given key.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::count(const Key& key) const noexcept -> size_type {
    if (find(key) != end()) return 1;
    else return 0;
}

/**
 * Checks if the set already contains a given object.
 * Returns the location of the match, or the index one-past-the-end if nothing
 * is found.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::find(const Key& key) const noexcept -> iterator {
    auto hash = hasher()(key);
    auto reduced_hash = (std::uint8_t) (hash >> (8*sizeof(hash) - 7)) & 0xef;
    auto first = hash % max_size();

    if (_sentinels[first].matches(reduced_hash))
        if (_elements[first] == key)
            return iterator{*this, first};

    for (auto current = first + 1;; ++current) {
        if (current == max_size()) current = 0;
        if (current == first) break;

        if (_sentinels[current].matches(reduced_hash))
            if (_elements[current] == key)
                return iterator{*this, current};
    }
    return end();
}

/**
 * Checks if the container contains an element with the given key.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::contains(const Key& key) const noexcept -> bool {
    return count(key) != 0;
}

/**
 * Finds the first free location after a given index.
 * If none can be found within 10 (or max_size()) spots, fails and returns the end iterator.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::probe(iterator location) const noexcept -> iterator {
    auto spots_visited = 0;
    auto first = location.index;

    if (_sentinels[first].empty()) return iterator{*this, first};

    for (auto current = first + 1;; ++current, ++spots_visited) {
        if (current == max_size()) current = 0;
        if (_sentinels[current].empty()) return iterator{*this, current};
        if (current == first || spots_visited == 10) return end();
    }
}

/**
 * Clears all elements by resetting the sentinels.
 * Allows for fast resetting of the hash table.
 */
template<typename Key, typename Hash, typename KeyEqual>
void dense_set<Key, Hash, KeyEqual>::clear() noexcept {
    std::fill(_sentinels.begin(), _sentinels.end(), sentinel{});
}


 /**
 * Colonizes the spot guarded by this metadata by raising the occupancy
 * flag and storing the 7 high bits of the given hash.
 */
template<typename Key, typename Hash, typename KeyEqual>
void dense_set<Key, Hash, KeyEqual>::sentinel::colonize(hash_type reduced_hash) noexcept {
    _filled = true;
    _reduced_hash = reduced_hash;
}

/**
 * Returns true if the spot is occupied and contains an object with a
 * similar (i.e. same 7 high bits) hash.
 */
template<typename Key, typename Hash, typename KeyEqual>
bool dense_set<Key, Hash, KeyEqual>::sentinel::matches(hash_type reduced_hash) const noexcept {
    return filled() && _reduced_hash == reduced_hash;
}