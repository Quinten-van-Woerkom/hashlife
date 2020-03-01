/**
 * Hashlife
 * Hash table based set implementation, meant for fast insertion and removal.
 * Uses an arena-based memory allocation strategy that is allowed to fail once
 * full.
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

#include <memory>

namespace life {
/**
 * Since the hash table only deallocates upon destruction, a single memory
 * arena that is allowed to fail when full is used.
 */
template<typename T>
class memory_arena {
public:
  using value_type = T;

  memory_arena(std::size_t elements)
  : elements{elements}, head{0}, memory{new T[elements]} {};

  memory_arena(memory_arena&&) = default;
  auto operator=(memory_arena&&) -> memory_arena& = default;

  auto size() const noexcept -> std::size_t { return elements; }
  auto full() const noexcept -> bool { return head == elements; }
  auto allocate(std::size_t n) noexcept -> T*;
  void deallocate(T* p, std::size_t bytes) const noexcept { /* no-op */ }

private:
  std::size_t elements;
  std::size_t head;
  std::unique_ptr<T[]> memory;
};

/**
 * Allocation is straightforward moving of the head of the allocated array.
 * If the array is full, fails by returning a nullptr;
 */
template<typename T>
auto memory_arena<T>::allocate(std::size_t n) noexcept -> T* {
  if (auto new_head = head + n; new_head <= elements) {
    auto old_head = head;
    head = new_head;
    return &memory[old_head];
  } else return nullptr;
}

/**
 * Allocator corresponding with the memory_arena type.
 */
template<typename T>
class arena_allocator {
public:
  using value_type = T;

  arena_allocator(memory_arena<T>& arena) : arena{arena} {}
  auto allocate(std::size_t n) noexcept -> T* { return arena.allocate(n); }
  void deallocate(T* p, std::size_t bytes) const noexcept { /* no-op */ }

private:
  memory_arena<T>& arena;
};
}