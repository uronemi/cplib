#pragma once
#include "BTreeBase.hpp"

template <
  typename Key,
  typename Compare = std::less<Key>,
  std::size_t B = 16,
  typename Alloc = std::allocator<Key>
>
class BTreeSet : private BTreeBase<Key, void, Compare, B, Alloc> {
  private:
  using Base = BTreeBase<Key, void, Compare, B, Alloc>;

  public:
  using key_type               = Key;
  using value_type             = Key;
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  using key_compare            = Compare;
  using value_compare          = Compare;
  using reference              = const Key&;
  using const_reference        = const Key&;
  using pointer                = const Key*;
  using const_pointer          = const Key*;
  using iterator               = typename Base::iterator;
  using const_iterator         = iterator;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using allocator_type         = Alloc;

  constexpr BTreeSet() noexcept = default;

  constexpr explicit BTreeSet(const Alloc& alloc) noexcept : Base(alloc) {}

  constexpr BTreeSet(const BTreeSet& other) = default;

  constexpr BTreeSet(BTreeSet&& other) noexcept = default;

  constexpr BTreeSet& operator=(const BTreeSet& other) = default;

  constexpr BTreeSet& operator=(BTreeSet&& other) noexcept = default;

  template <std::input_iterator It>
  constexpr BTreeSet(It first, It last) { insert(first, last); }

  constexpr BTreeSet(std::initializer_list<Key> init) { insert(init.begin(), init.end()); }

  template <std::ranges::input_range R>
  constexpr BTreeSet(std::from_range_t, R&& r) {
    for (auto&& e : r) insert(std::forward<decltype(e)>(e));
  }

  [[nodiscard]] constexpr allocator_type get_allocator() const noexcept { return this->alloc_; }

  using Base::begin;
  using Base::end;

  [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }

  [[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  }

  [[nodiscard]] constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  }

  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(begin());
  }

  [[nodiscard]] constexpr bool empty() const noexcept { return Base::is_empty(); }

  [[nodiscard]] constexpr size_type size() const noexcept { return Base::get_size(); }

  [[nodiscard]] constexpr size_type max_size() const noexcept {
    return std::allocator_traits<allocator_type>::max_size(this->alloc_);
  }

  [[nodiscard]] constexpr key_compare key_comp() const { return key_compare(); }

  [[nodiscard]] constexpr value_compare value_comp() const { return value_compare(); }

  using Base::find;
  using Base::lower_bound;
  using Base::upper_bound;

  [[nodiscard]] constexpr bool contains(const Key& key) const noexcept {
    return find(key) != end();
  }

  [[nodiscard]] constexpr size_type count(const Key& key) const noexcept {
    return contains(key) ? 1 : 0;
  }

  [[nodiscard]] constexpr std::pair<const_iterator, const_iterator> equal_range(const Key& key) const noexcept {
    return {lower_bound(key), upper_bound(key)};
  }

  [[nodiscard]] constexpr iterator find_by_order(size_type idx) noexcept {
    return Base::at(idx);
  }

  [[nodiscard]] constexpr const_iterator find_by_order(size_type idx) const noexcept {
    return Base::at(idx);
  }

  [[nodiscard]] constexpr const_reference operator[](size_type idx) const noexcept {
    return *Base::at(idx);
  }

  [[nodiscard]] constexpr size_type order_of_key(const Key& key) const noexcept {
    return Base::order_of_key(key);
  }

  [[nodiscard]] constexpr const_iterator pred(const Key& key) const noexcept {
    return const_reverse_iterator(lower_bound(key));
  }

  [[nodiscard]] constexpr const_reverse_iterator pred_or_equal(const Key& key) const noexcept {
    return const_reverse_iterator(upper_bound(key));
  }

  [[nodiscard]] constexpr const_iterator succ(const Key& key) const noexcept {
    return upper_bound(key);
  }

  [[nodiscard]] constexpr const_iterator succ_or_equal(const Key& key) const noexcept {
    return lower_bound(key);
  }

  using Base::insert;

  template <std::input_iterator It>
  constexpr void insert(It first, It last) {
    for (; first != last; ++first) insert(*first);
  }

  constexpr void insert(std::initializer_list<Key> init) { insert(init.begin(), init.end()); }

  template <class... Args>
  constexpr std::pair<iterator, bool> emplace(Args&&... args) {
    return insert(Key(std::forward<Args>(args)...));
  }

  constexpr size_type erase(const Key& key) noexcept { return Base::erase(key) ? 1 : 0; }

  constexpr iterator erase(const_iterator pos) noexcept {
    if (pos == end()) return end();
    Key k = *pos;
    Base::erase(k);
    return upper_bound(k);
  }

  constexpr iterator erase(const_iterator first, const_iterator last) noexcept {
    if (first == end() || first == last) return iterator(first);
    if (last == end()) {
      while (first != end()) first = erase(first);
      return end();
    }
    Key last_key = *last;
    auto comp = key_comp();
    while (first != end() && comp(*first, last_key)) first = erase(first);
    return iterator(first);
  }

  constexpr void clear() noexcept {
    Base::clear_tree(this->root_);
    this->root_ = nullptr;
    this->tree_size_ = 0;
  }

  constexpr void swap(BTreeSet& other) noexcept { Base::swap(other); }
};

template <typename Key, typename Compare, std::size_t B, typename Alloc>
constexpr void swap(BTreeSet<Key, Compare, B, Alloc>& lhs, BTreeSet<Key, Compare, B, Alloc>& rhs) noexcept {
  lhs.swap(rhs);
}