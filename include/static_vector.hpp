/**
 * Hashlife
 * Static vector, that dynamically allocates a fixed size that is determined
 * upon construction.
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

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>

/**
 * Dynamically-allocated vector of constant size.
 * Exploits the constant-size condition to minimise allocations.
 */
template<typename T>
class static_vector {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr static_vector() noexcept : _storage{nullptr}, _size{0} {}
    static_vector(std::size_t count) : _storage{new T[count]}, _size{count} {}
    static_vector(static_vector&&) noexcept(std::is_nothrow_swappable_v<T>) = default;

    template<typename... Args>
    static_vector(std::size_t count, Args&&... args) : _storage{new T[count]}, _size{count} {
        fill(T{args...});
    }

    static_vector(const static_vector& other) : _storage{new T[other.size()]}, _size{other.size()} {
        std::copy(other.begin(), other.end(), begin());
    }

    constexpr auto operator=(static_vector&&) noexcept -> static_vector& = default;
    auto operator=(const static_vector&) -> static_vector&;

    constexpr auto empty() const noexcept -> bool { return _size == 0; }
    constexpr auto size() const noexcept -> std::size_t { return _size; }
    constexpr auto max_size() const noexcept -> std::size_t { return _size; }

    constexpr void fill(const T& value) { std::fill(begin(), end(), value); };
    constexpr void swap(static_vector& other) noexcept(std::is_nothrow_swappable_v<T>) { std::swap(*this, other); }

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