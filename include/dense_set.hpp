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
#include <stdexcept>
#include <tuple>
#include <type_traits>

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

private:
    class sentinel;

    /**************************************************************************
     * Inner iterator into the hashmap, covering all elements, empty or not.
     * Basically an index, considering the hashmap supports open addressing.
     * Wraps around upon reaching the end of the element range.
     */
    template<bool is_constant>
    struct inner_iterator {
        using owner_reference = std::conditional_t<is_constant, const dense_set&, dense_set&>;
        using owner_pointer = std::conditional_t<is_constant, const dense_set*, dense_set*>;

        constexpr inner_iterator(owner_reference owner, size_type index) noexcept
        : owner{&owner}, index{index} {}

        /**
         * Only enabled if this iterator is constant and the other is non-constant, to prevent the
         * implicit default constructors from being deleted.
         */
        template<bool other_is_constant, typename = std::enable_if_t<is_constant && !other_is_constant>>
        constexpr inner_iterator(inner_iterator<other_is_constant> other) noexcept
        : owner{other.owner}, index{other.index} {}

        constexpr auto operator++() noexcept -> inner_iterator& {
            index = (index + 1) % owner->capacity();
            return *this;
        }

        constexpr auto operator++(int) noexcept -> inner_iterator {
            auto return_value = *this;
            ++(*this);
            return return_value;
        }

        constexpr auto operator==(const inner_iterator& other) const noexcept {
            return owner == other.owner && index == other.index;
        }

        constexpr auto operator!=(const inner_iterator& other) const noexcept {
            return !(*this == other);
        }

        constexpr auto& operator*() const noexcept {
            return (*owner)[index];
        }

        /**
         * Lookup, wrapper functions that allow for quick determination of the
         * (non-)existence of the currently pointed-to object.
         */
        constexpr auto empty() const noexcept {
            return owner->_sentinels[index].empty();
        }

        constexpr auto matches(size_type reduced_hash) const noexcept {
            return owner->_sentinels[index].matches(reduced_hash);
        }

        constexpr void colonize(hash_type reduced_hash) const noexcept {
            owner->_sentinels[index].colonize(reduced_hash);
        }

        constexpr auto contains(const Key& key) const noexcept {
            return key_equal()((*owner)[index], key);
        }

        owner_pointer owner;
        size_type index;
    };


public:
    /**************************************************************************
     * Iterator into the hashmap.
     * Basically an index, considering the hashmap supports open addressing.
     */
    template<bool is_constant>
    struct iterator_implementation {
        using owner_reference = typename inner_iterator<is_constant>::owner_reference;

        constexpr iterator_implementation(owner_reference owner, size_type index) noexcept
         : inner{owner, index} {}

        constexpr iterator_implementation(inner_iterator<is_constant> inner) noexcept
        : inner{inner} {}

         /**
         * Only enabled if this iterator is constant and the other is non-constant, to prevent the
         * implicit default constructors from being deleted.
         */
        template<bool other_is_constant, typename = std::enable_if_t<is_constant && !other_is_constant>>
        constexpr iterator_implementation(iterator_implementation<other_is_constant> other) noexcept
        : inner{other.inner} {}

        constexpr operator inner_iterator<is_constant>() const noexcept { return inner; }

        /**
         * Increments the index until a valid object is found, i.e. where the
         * sentinel indicates an object exists.
         * Precondition: not end() iterator
         */
        constexpr auto operator++() noexcept -> iterator_implementation& {
            ++inner;
            while (inner.empty()) ++inner;
            return *this;
        }

        constexpr auto operator++(int) noexcept -> iterator_implementation {
            auto return_value = *this;
            ++(*this);
            return return_value;
        }

        /**
         * Equality comparators.
         */
        constexpr auto operator==(const iterator_implementation& other) const noexcept {
            return inner == other.inner;
        }

        constexpr auto operator!=(const iterator_implementation& other) const noexcept {
            return !(*this == other);
        }

        /**
         * Dereferences the iterator, returning the object at the current index.
         * Assumes that an object exists at this index.
         */
        constexpr auto operator*() const noexcept {
            return *inner;
        }

        inner_iterator<is_constant> inner;
    };

    using iterator = iterator_implementation<false>;
    using const_iterator = iterator_implementation<true>;


    /**************************************************************************
     * Constructors
     */
    dense_set(size_type count);
    dense_set(const dense_set&) = default;
    constexpr dense_set(dense_set&&) = default;


    /**************************************************************************
     * Assignment operators
     */
    auto operator=(const dense_set&) -> dense_set& = default;
    constexpr auto operator=(dense_set&&) -> dense_set& = default;


    /**************************************************************************
     * Iterators
     */
    constexpr auto begin() noexcept { return iterator{*this, 0}; }
    constexpr auto begin() const noexcept { return const_iterator{*this, 0}; }
    constexpr auto end() noexcept { return iterator{*this, capacity()}; }
    constexpr auto end() const noexcept { return const_iterator{*this, capacity()}; }
    constexpr auto cbegin() const noexcept { return const_iterator{*this, 0}; }
    constexpr auto cend() const noexcept { return const_iterator{*this, capacity()}; }


    /**************************************************************************
     * Capacity
     */
    constexpr auto empty() const noexcept { return _size == 0; }
    constexpr auto size() const noexcept { return _size; }
    constexpr auto capacity() const noexcept { return _elements.capacity(); }


    /**************************************************************************
     * Modifiers
     */
    void clear() noexcept;
    auto insert(const value_type& value) noexcept -> std::pair<iterator, bool> { return emplace(value); };
    auto insert(value_type&& value) noexcept -> std::pair<iterator, bool> { return emplace(std::move(value)); };
    void swap(dense_set&& other) noexcept(is_nothrow_swappable) { std::swap(*this, other); }

    template<typename... Args>
    auto emplace(Args&&... args) noexcept -> std::pair<iterator, bool> {
        auto object = Key{args...};
        auto hash = hasher()(object);
        auto reduced_hash = (std::uint8_t) (hash >> (8*sizeof(hash) - 7)) & 0xef;

        auto location = find(object, hash, reduced_hash);
        if (location != end()) return {location, false};

        location = iterator{*this, hash % capacity()};
        auto free_location = probe(location);

        if (free_location == end()) return {location, false};
        
        free_location.colonize(reduced_hash);
        *free_location = std::move(object);
        ++_size;
        return {location, true};
    }


    /**************************************************************************
     * Lookup
     */
    auto operator[](size_type index) noexcept -> Key&;
    auto operator[](size_type index) const noexcept -> const Key&;
    auto count(const Key& key) const noexcept -> size_type;
    auto find(const Key& key) noexcept -> iterator;
    auto find(const Key& key) const noexcept -> const_iterator;
    auto contains(const Key& key) const noexcept -> bool;

private:
    auto find(const Key& key, hash_type hash, hash_type reduced_hash) noexcept -> iterator;
    auto find(const Key& key, hash_type hash, hash_type reduced_hash) const noexcept -> const_iterator;
    auto probe(inner_iterator<false> start) noexcept -> inner_iterator<false>;
    auto probe(inner_iterator<false> start) const noexcept -> inner_iterator<true>;

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
    size_type _size = 0;
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
 : _elements{count}, _sentinels{count, sentinel{}} {
     if (count <= 0) throw std::domain_error{"dense_set: element counts equal to or smaller than 0 are not supported."};
 }


/******************************************************************************
 * Lookup
 */
/**
 * Indexing is checked in debug mode to ensure that accessed elements actually
 * exist.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::operator[](std::size_t index) noexcept -> Key& {
    assert(index < capacity() && "dense_set: Index access out of bound");
    assert(_sentinels[index].filled() && "dense_set: Trying to access non-existent element");
    return _elements[index];
}

/**
 * Indexing is checked in debug mode to ensure that accessed elements actually
 * exist.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::operator[](std::size_t index) const noexcept -> const Key& {
    assert(index < capacity() && "dense_set: Index access out of bound");
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
auto dense_set<Key, Hash, KeyEqual>::find(const Key& key) noexcept -> iterator {
    auto hash = hasher()(key);
    auto reduced_hash = (std::uint8_t) (hash >> (8*sizeof(hash) - 7)) & 0xef; 
    return find(key, hash, reduced_hash);
}

template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::find(const Key& key) const noexcept -> const_iterator {
    return const_cast<dense_set*>(this)->find(key);
}

/**
 * Checks if the set already contains a given object.
 * Returns the location of the match, or the index one-past-the-end if nothing
 * is found.
 * Used when the hash and reduced_hash are already computed elsewhere, to save
 * the time required for recomputation.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::find(const Key& key, hash_type hash, hash_type reduced_hash) noexcept -> iterator {
    auto start = inner_iterator<false>{*this, hash % capacity()};
    auto current = start;

    do {
        if (current.empty()) return end();
        if (current.matches(reduced_hash))
            if (current.contains(key))
                return current;

        ++current;
    } while (current != start);
        
    return end();
}

template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::find(const Key& key, hash_type hash, hash_type reduced_hash) const noexcept -> const_iterator {
    return const_cast<dense_set*>(this)->find(key, hash, hash_type);
}

/**
 * Checks if the container contains an element with the given key.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::contains(const Key& key) const noexcept -> bool {
    return count(key) != 0;
}

/**
 * Finds the first free location at or after a given index.
 * If none can be found within 10 (or capacity()) spots, fails and returns the end iterator.
 */
template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::probe(inner_iterator<false> start) noexcept -> inner_iterator<false> {
    auto spots_visited = 0;
    auto current = start;

    do {
        if (current.empty()) return current;
        ++current, ++spots_visited;
    } while (current != start && spots_visited != 10);

    return end();
}

template<typename Key, typename Hash, typename KeyEqual>
auto dense_set<Key, Hash, KeyEqual>::probe(inner_iterator<false> start) const noexcept -> inner_iterator<true> {
    return const_cast<dense_set*>(this)->probe(start);
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