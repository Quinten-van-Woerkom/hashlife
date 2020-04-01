/**
 * Hashlife
 * Hash table based set implementation, meant for fast insertion.
 * Based on the premise that insertion is allowed to fail, and that no
 * deletions occur. This allows us to neglect tombstones, and mean we have no
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <vector>

#include <iostream>

/**
 * Dynamically-allocated vector of constant size.
 * Exploits the constant-size condition to minimise allocations.
 */
template<typename T>
class static_vector {
public:
    static_vector() noexcept : _storage{nullptr}, _size{0} {}
    static_vector(std::size_t count) : _storage{new T[count]}, _size{count} {}
    static_vector(const static_vector&) = default;
    static_vector(static_vector&&) noexcept = default;

    template<typename... Args>
    static_vector(std::size_t count, Args&&... args) : _storage{new T[count]}, _size{count} {
        std::fill(begin(), end(), T{args...});
    }

    constexpr auto operator=(static_vector&&) noexcept -> static_vector& = default;
    auto operator=(const static_vector&) -> static_vector&;

    constexpr auto size() const noexcept -> std::size_t { return _size; }
    constexpr auto empty() const noexcept -> bool { return _size == 0; }

    constexpr auto operator[](std::size_t index) -> T&;
    constexpr auto operator[](std::size_t index) const -> const T&;

    constexpr auto begin() noexcept { return _storage.get(); }
    constexpr auto end() noexcept { return _storage.get() + _size; }
    constexpr auto begin() const noexcept { return _storage.get(); }
    constexpr auto end() const noexcept { return _storage.get() + _size; }
    constexpr auto cbegin() const noexcept { return _storage.get(); }
    constexpr auto cend() const noexcept { return _storage.get() + _size; }
    constexpr auto rbegin() noexcept { return std::rbegin(_storage.get() + _size); }
    constexpr auto rend() noexcept { return std::rend(_storage.get()); }
    constexpr auto rbegin() const noexcept { return std::rbegin(_storage.get() + _size); }
    constexpr auto rend() const noexcept { return std::rend(_storage.get()); }
    constexpr auto crbegin() const noexcept { return std::crbegin(_storage.get() + _size); }
    constexpr auto crend() const noexcept { return std::crend(_storage.get()); }

private:
    std::unique_ptr<T[]> _storage;
    std::size_t _size;
};

/**
 * Assignment reallocates only if a different size is encountered.
 * It does currently reallocate on shrinking, this could be improved on.
 */
template<typename T>
auto static_vector<T>::operator=(const static_vector& other) -> static_vector& {
    if (size() != other.size()) {
        _storage.reset(new T[other.size()]);
        _size = other.size();
    }
    std::copy(other.begin(), other.end(), begin());

    return *this;
}

/**
 * Array indexing does bounds-checking in debug mode.
 */
template<typename T>
constexpr auto static_vector<T>::operator[](std::size_t index) -> T& {
    assert(index < size() && "static_vector: Index out of bounds");
    return _storage[index];
}

/**
 * Array indexing does bounds-checking in debug mode.
 */
template<typename T>
constexpr auto static_vector<T>::operator[](std::size_t index) const -> const T& {
    assert(index < size() && "static_vector: Index out of bounds");
    return _storage[index];
}


/**
 * Hashlife requires a rather specialized hash table, requiring open addressing
 * and stability of reference. In addition, the knowledge that no deletions
 * occur (in principle garbage cleaning is necessary, but better implemented
 * as a full hash table reset) can be exploited since no tombstones are
 * necessary.
 */
template<typename T, typename Hash = std::hash<T>>
class dense_hash_set {
public:
    using index_type = std::size_t;
    using hash_type = std::size_t;

    dense_hash_set(std::size_t count);

    auto operator[](std::size_t index) const noexcept -> const T&;
    auto operator[](std::size_t index) noexcept -> T&;

    auto capacity() const noexcept { return _sentinels.size(); }
    auto find(const T& object) const noexcept -> index_type;
    auto free(index_type location) const noexcept -> index_type;
    void clear() noexcept;

    template<typename... Args>
    auto emplace(Args&&... args) noexcept -> index_type {
        auto object = T{args...};
        auto location = find(T{args...});
        if (location != capacity()) return location;

        auto hash = Hash()(object);
        location = hash % capacity();
        location = free(location);

        if (location == capacity()) return capacity();
        else {
            auto reduced_hash = (std::uint8_t) (hash >> (8*sizeof(hash) - 7)) & 0xef;
            _sentinels[location].colonize(reduced_hash);
            _elements[location] = std::move(object);
            return location;
        }
    }

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

    static_vector<T> _elements;
    static_vector<sentinel> _sentinels;
};

/**
 * Constructs an empty hash table of size <count>.
 * Note that all sentinels must be value-initialized.
 */
template<typename T, typename Hash>
dense_hash_set<T, Hash>::dense_hash_set(std::size_t count)
 : _elements{count}, _sentinels{count, sentinel{}} {}

/**
 * Indexing is checked in debug mode to ensure that accessed elements actually
 * exist.
 */
template<typename T, typename Hash>
auto dense_hash_set<T, Hash>::operator[](std::size_t index) const noexcept -> const T& {
    assert(index < capacity() && "dense_hash_set: Index access out of bound");
    assert(_sentinels[index].filled() && "dense_hash_set: Trying to access non-existent element");
    return _elements[index];
}

/**
 * Indexing is checked in debug mode to ensure that accessed elements actually
 * exist.
 */
template<typename T, typename Hash>
auto dense_hash_set<T, Hash>::operator[](std::size_t index) noexcept -> T& {
    assert(index < capacity() && "dense_hash_set: Index access out of bound");
    assert(_sentinels[index].filled() && "dense_hash_set: Trying to access non-existent element");
    return _elements[index];
}

/**
 * Checks if the set already contains a given object.
 * Returns the location of the match, or the index one-past-the-end if nothing
 * is found.
 */
template<typename T, typename Hash>
auto dense_hash_set<T, Hash>::find(const T& object) const noexcept -> index_type {
    auto hash = Hash()(object);
    auto reduced_hash = (std::uint8_t) (hash >> (8*sizeof(hash) - 7)) & 0xef;

    auto index = hash % capacity();

    if (_sentinels[index].matches(reduced_hash) && _elements[index] == object)
        return index;

    for (auto i = index + 1; i != index; ++i) {
        if (i == capacity()) i = 0;
        if (_sentinels[i].empty()) break;

        if (_sentinels[i].matches(reduced_hash))
            if (_elements[i] == object)
                return i;
    }

    return capacity();
}

/**
 * Finds the first free location after a given index.
 * If none can be found within <limit> spots, fails and returns the index
 * one-past-the-end of the array.
 */
template<typename T, typename Hash>
auto dense_hash_set<T, Hash>::free(index_type location) const noexcept -> index_type {
    auto n = 0;
    for (auto i = location; n < 10; ++i, ++n) {
        if (i == capacity()) i = 0;
        if (_sentinels[i].empty()) return i;
    }
    return capacity();
}

/**
 * Clears all elements by resetting the sentinels.
 * Allows for fast resetting of the hash table.
 */
template<typename T, typename Hash>
void dense_hash_set<T, Hash>::clear() noexcept {
    std::fill(_sentinels.begin(), _sentinels.end(), sentinel{});
}


 /**
 * Colonizes the spot guarded by this metadata by raising the occupancy
 * flag and storing the 7 high bits of the given hash.
 */
template<typename T, typename Hash>
void dense_hash_set<T, Hash>::sentinel::colonize(std::size_t reduced_hash) noexcept {
    _filled = true;
    _reduced_hash = reduced_hash;
}

/**
 * Returns true if the spot is occupied and contains an object with a
 * similar (i.e. same 7 high bits) hash.
 */
template<typename T, typename Hash>
bool dense_hash_set<T, Hash>::sentinel::matches(std::size_t reduced_hash) const noexcept {
    return filled() && _reduced_hash == reduced_hash;
}