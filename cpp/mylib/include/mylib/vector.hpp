#ifndef MYLIB_VECTOR_HPP_
#define MYLIB_VECTOR_HPP_

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>

namespace mylib
{
template <typename T, typename Alloc = std::allocator<T>>
class vector
{
public:
  using traits = std::allocator_traits<Alloc>;
  using value_type = T;
  using size_type = std::size_t;
  using pointer = typename traits::pointer;
  using const_pointer = typename traits::const_pointer;
  using reference = T &;
  using const_reference = const T &;
  // allocator
  using allocator_type = Alloc;
  // iterator
  using iterator = T *;
  using const_iterator = const T *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  pointer arr_;
  size_type length_ = 0, capacity_ = 1;
  allocator_type alloc_;

  // === Validation ===
  static_assert(
    std::is_same<T, typename Alloc::value_type>::value,
    "The allocator value must be matched the vector value type.");
  static_assert(!std::is_const<T>::value, "Const elements are forbidden.");

  // === Helper functions ===
  pointer allocate(size_type n) { return traits::allocate(alloc_, n); }
  void deallocate() { traits::deallocate(alloc_, arr_, capacity()); }
  void construct(pointer ptr) { traits::construct(alloc_, ptr); }
  void construct(pointer ptr, const_reference value) { traits::construct(alloc_, ptr, value); }
  void construct(pointer ptr, value_type && value)
  {
    traits::construct(alloc_, ptr, std::move(value));
  }
  void destroy(pointer ptr) { traits::destroy(alloc_, ptr); }
  void destroy_until(reverse_iterator rend)
  {
    for (auto riter = rbegin(); riter != rend; ++riter) {
      destroy(&*riter);
    }
  }

public:
  // === Constructor ===
  explicit vector(const allocator_type & alloc) noexcept : alloc_(alloc) {}

  vector() : vector(allocator_type()) {}

  explicit vector(size_type n, const allocator_type & alloc = allocator_type()) : vector(alloc)
  {
    resize(n);
  }

  explicit vector(
    size_type n, const_reference value, const allocator_type & alloc = allocator_type())
  : vector(alloc)
  {
    resize(n, value);
  }

  // === Destructor ===
  ~vector()
  {
    clear();
    deallocate();
  }

  // === Copy-constructor ==
  vector & operator=(const vector & v)
  {
    clear();
    deallocate();
    resize(v.size());
    arr_ = allocate(capacity());
    for (size_type i = 0; i < size(); ++i) {
      construct(begin() + i, *(v.begin() + i));
    }
  }

  vector & operator=(vector && v)
  {
    clear();
    deallocate();
    *this = std::move(v);
    return *this;
  }

  // === Iterator ===
  iterator begin() noexcept { return arr_; }
  const_iterator begin() const noexcept { return arr_; }
  const_iterator cbegin() const noexcept { return arr_; }
  reverse_iterator rbegin() noexcept { return reverse_iterator(arr_ + size()); }
  const_iterator crbegin() const noexcept { return const_reverse_iterator(arr_ + size()); }
  iterator end() noexcept { return arr_ + size(); }
  const_iterator end() const noexcept { return arr_ + size(); }
  const_iterator cend() const noexcept { return arr_ + size(); }
  reverse_iterator rend() noexcept { return reverse_iterator(arr_); }
  const_reverse_iterator rend() const noexcept { return const_reverse_iterator(arr_); }
  const_reverse_iterator crend() const noexcept { return const_reverse_iterator(arr_); }

  // === Size ===
  size_type capacity() const noexcept { return capacity_; }
  size_type size() const { return length_; }
  bool empty() const { return begin() == end(); }

  void resize(size_type sz)
  {
    reserve(sz);
    if (sz < size()) {
      auto diff = size() - sz;
      destroy_until(rbegin() + diff);
      length_ = sz;
    } else if (sz > size()) {
      for (auto iter = begin(); iter != end(); ++iter) {
        construct(iter);
      }
    }
    length_ = sz;
  }

  void resize(size_type sz, const_reference value)
  {
    reserve(sz);
    if (sz < size()) {
      auto diff = size() - sz;
      destroy_until(rbegin() + diff);
    } else if (sz > size()) {
      for (auto iter = begin(); iter != end(); ++iter) {
        construct(iter, value);
      }
    }
    length_ = sz;
  }

  void reserve(size_type sz)
  {
    if (capacity() < sz) {
      while (capacity_ < sz) {
        capacity_ *= 2;
      }
      arr_ = allocate(sz);
    }
  }

  // === Access to the elements ===
  reference operator[](const size_type i) const { return arr_[i]; }
  reference at(const size_type i) const
  {
    if (i >= size()) {
      throw std::out_of_range("Index %d is out of range.");
    }
    return arr_[i];
  }

  reference front() { return arr_; }
  reference back() { return arr_ + length_; }
  pointer data() { return arr_; }

  // === Update container ===
  template <class... Args>
  void emplace_back(Args &&... args)
  {
    if (capacity() < size() + 1) {
      reserve(size() + 1);
    }
    construct(arr_ + size(), std::forward<Args>(args)...);
    ++length_;
  }

  void push_back(const value_type value) { emplace_back(value); }

  void clear() { destroy_until(rend()); }

};  // class vector
}  // namespace mylib

#endif  // MYLIB_VECTOR_HPP_
