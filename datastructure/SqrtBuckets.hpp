#pragma once
#include "../utility/Algebra.hpp"

template <
  typename Key,
  bool Multi = false,
  typename Compare = std::less<Key>
>
class SqrtBucketsSet {
  public:
  using key_type = Key;
  using value_type = Key;
  using key_compare = Compare;
  using value_compare = Compare;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = typename std::allocator_traits<std::allocator<Key>>::pointer;
  using const_pointer = typename std::allocator_traits<std::allocator<Key>>::const_pointer;
  using reference = Key&;
  using const_reference = const Key&;

  private:
  struct Bucket {
    private:
    std::vector<key_type> data;
    size_type head, sz, cap, mask;

    inline size_type idx(size_type i) const { return (head + i) & mask; }

    public:
    Bucket(std::size_t l) {
      head = sz = 0;
      cap = std::bit_ceil(l + 1);
      mask = cap - 1;
      data.resize(cap);
    }

    size_type size() const { return sz; }

    const key_type& operator[](size_type k) const { return data[idx(k)]; }

    key_type& operator[](size_type k) { return data[idx(k)]; }

    key_type back() const { return data[idx(sz - 1)]; }

    void pop_front() {
      if (sz > 0) {
        head = (head + 1) & mask;
        sz--;
      }
    }

    void pop_back() { if (sz > 0) sz--; }

    void push_front(const key_type& x) {
      head = (head - 1) & mask;
      data[head] = x;
      sz++;
    }

    void push_back(const key_type& x) {
      data[idx(sz)] = x;
      sz++;
    }

    void insert(size_type i, const key_type& x) {
      if (!sz) {
        push_front(x);
        return;
      }
      if (i == sz) {
        push_back(x);
        return;
      }

      size_type index = idx(i), tail = idx(sz);
      if (index < tail) std::copy_backward(data.begin() + index, data.begin() + tail, data.begin() + tail + 1);
      else {
        if (tail > 0) std::copy_backward(data.begin(), data.begin() + tail, data.begin() + tail + 1);
        data[0] = data.back();
        if (index < cap - 1) std::copy_backward(data.begin() + index, data.end() - 1, data.end());
      }

      data[index] = x;
      sz++;
    }

    void erase(size_type i) {
      if (!i) {
        pop_front();
        return;
      }
      if (i == sz - 1) {
        pop_back();
        return;
      }

      size_type index = idx(i), tail = idx(sz - 1);
      if (index < tail) std::copy(data.begin() + index + 1, data.begin() + tail + 1, data.begin() + index);
      else {
        if (index < cap - 1) std::copy(data.begin() + index + 1, data.end(), data.begin() + index);
        data[cap - 1] = data[0];
        if (tail > 0) std::copy(data.begin() + 1, data.begin() + tail + 1, data.begin());
      }
      sz--;
    }
  };

  std::vector<Bucket> buckets;
  std::vector<key_type> m;
  size_type b, sz;
  Compare comp;

  size_type find_bucket(const key_type& x) const {
    if (m.empty()) return 0;

    auto it = std::upper_bound(m.begin(), m.end(), x, [this](const key_type& a, const key_type& b) {
      return comp(a, b);
    });
    return it == m.begin() ? 0 : it - m.begin() - 1;
  }

  public:
  explicit SqrtBucketsSet(size_type B = 1024) : b(B), sz(0), comp(key_compare()), buckets(), m() {}

  SqrtBucketsSet(const SqrtBucketsSet& other)
    : b(other.b), sz(other.sz), comp(other.comp), buckets(other.buckets), m(other.m) {}

  SqrtBucketsSet(size_type B, const key_compare& c) : b(B), sz(0), comp(c), buckets(), m() {}

  SqrtBucketsSet(SqrtBucketsSet&& other) noexcept
    : b(other.b),
      sz(other.sz),
      comp(std::move(other.comp)),
      buckets(std::move(other.buckets)),
      m(std::move(other.m)) {
    other.sz = 0;
  }

  template <class Inputit>
  SqrtBucketsSet(size_type B, Inputit first, Inputit last)
    : b(B), sz(0), comp(key_compare()), buckets(), m() {
    this->insert(first, last);
  }

  template <class Inputit>
  SqrtBucketsSet(size_type B, Inputit first, Inputit last, const key_compare& c)
    : b(B), sz(0), comp(c), buckets(), m() {
    this->insert(first, last);
  }

  SqrtBucketsSet(std::initializer_list<key_type> init, size_type B = 1024, const key_compare& c = key_compare())
    : b(B), sz(0), comp(c), buckets(), m() {
    this->insert(init.begin(), init.end());
  }

  ~SqrtBucketsSet() = default;

  size_type size() const noexcept { return sz; }

  bool empty() const noexcept { return !sz; }

  void clear() noexcept {
    buckets.clear();
    m.clear();
    sz = 0;
  }

  void swap(SqrtBucketsSet& other) noexcept {
    std::swap(buckets, other.buckets);
    std::swap(m, other.m);
    std::swap(b, other.b);
    std::swap(sz, other.sz);
    std::swap(comp, other.comp);
  }

  SqrtBucketsSet& operator=(const SqrtBucketsSet& other) {
    if (this != &other) {
      SqrtBucketsSet tmp(other);
      this->swap(tmp);
    }
    return *this;
  }

  SqrtBucketsSet& operator=(SqrtBucketsSet&& other) noexcept {
    this->swap(other);
    return *this;
  }

  SqrtBucketsSet& operator=(std::initializer_list<key_type> init) {
    this->clear();
    this->insert(init.begin(), init.end());
    return *this;
  }

  class const_iterator {
    public:
    friend class SqrtBucketsSet;

    private:
    const SqrtBucketsSet* container;
    size_type idx;

    const_iterator(const SqrtBucketsSet* c, size_type i) : container(c), idx(i) {}

    public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using pointer = const Key*;
    using reference = const Key&;

    const_iterator() : container(nullptr), idx(0) {}

    reference operator*() const { return (*container)[idx]; }

    pointer operator->() const { return &((*container)[idx]); }

    const_iterator& operator++() { idx++; return *this; }

    const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }

    const_iterator& operator--() { idx--; return *this; }

    const_iterator operator--(int) { const_iterator tmp = *this; --(*this); return tmp; }

    const_iterator operator+=(difference_type n) { idx += n; return *this; }

    const_iterator operator-=(difference_type n) { idx -= n; return *this; }

    const_iterator operator+(difference_type n) const { return const_iterator(container, idx + n); }

    const_iterator operator-(difference_type n) const { return const_iterator(container, idx - n); }

    friend const_iterator operator+(difference_type n, const const_iterator& it) { return it + n; }

    difference_type operator-(const const_iterator& other) const {
      return static_cast<difference_type>(idx) - static_cast<difference_type>(other.idx);
    }

    reference operator[](difference_type n) const { return *(*this + n); }

    bool operator==(const const_iterator& other) const {
      return container == other.container && idx == other.idx;
    }

    bool operator!=(const const_iterator& other) const { return !(*this == other); }

    bool operator<(const const_iterator& other) const { return idx < other.idx; }

    bool operator<=(const const_iterator& other) const { return idx <= other.idx; }

    bool operator>(const const_iterator& other) const { return idx > other.idx; }

    bool operator>=(const const_iterator& other) const { return idx >= other.idx; }
  };

  using iterator = const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() const noexcept { return iterator(this, 0); }

  iterator end() const noexcept { return iterator(this, sz); }

  reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }

  reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

  const_iterator cbegin() const noexcept { return begin(); }

  const_iterator cend() const noexcept { return end(); }

  const_reverse_iterator crbegin() const noexcept { return rbegin(); }

  const_reverse_iterator crend() const noexcept { return rend(); }

  void insert(const key_type& x) {
    if (buckets.empty()) {
      buckets.emplace_back(b);
      buckets.back().push_front(x);
      m.push_back(x);
      sz++;
      return;
    }

    size_type idx = find_bucket(x), ok = 0, ng = buckets[idx].size();
    while (ok < ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(buckets[idx][mid], x)) ok = mid + 1;
      else ng = mid;
    }
    if constexpr (!Multi) {
      if (ok < buckets[idx].size() && !comp(x, buckets[idx][ok]) && !comp(buckets[idx][ok], x)) return;
    }
    buckets[idx].insert(ok, x);
    sz++;
    if (!ok) m[idx] = x;

    size_type cur = idx;
    while (cur < buckets.size() && buckets[cur].size() > b) {
      key_type backv = buckets[cur].back();
      buckets[cur].pop_back();
      if (cur + 1 == buckets.size()) {
        buckets.emplace_back(b);
        buckets.back().push_front(backv);
        m.push_back(backv);
        break;
      }
      else {
        cur++;
        buckets[cur].push_front(backv);
        m[cur] = backv;
      }
    }
  }

  size_type erase(const key_type& x) {
    if (buckets.empty()) return 0;

    size_type idx = find_bucket(x), ok = 0, ng = buckets[idx].size();
    while (ok < ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(buckets[idx][mid], x)) ok = mid + 1;
      else ng = mid;
    }
    if (ok == buckets[idx].size() || comp(x, buckets[idx][ok])) return 0;
    buckets[idx].erase(ok);
    sz--;
    if (buckets[idx].size() > 0 && !ok) m[idx] = buckets[idx][0];

    size_type cur = idx;
    while (cur + 1 < buckets.size()) {
      key_type frontv = buckets[cur + 1][0];
      buckets[cur + 1].pop_front();
      buckets[cur].push_back(frontv);
      if (buckets[cur + 1].size() > 0) m[cur + 1] = buckets[cur + 1][0];
      cur++;
    }
    if (buckets.size() && !buckets.back().size()) buckets.pop_back(), m.pop_back();

    return 1;
  }

  template <class Inputit>
  void insert(Inputit first, Inputit last) {
    for (auto it = first; it != last; ++it) insert(*it);
  }

  iterator erase(iterator pos) {
    if (pos == end() || pos.container != this) return end();
    
    size_type b_idx = pos.idx / b, e_idx = pos.idx % b;
    buckets[b_idx].erase(e_idx);
    sz--;
    if (buckets[b_idx].size() > 0 && !e_idx) m[b_idx] = buckets[b_idx][0];

    size_type cur = b_idx;
    while (cur + 1 < buckets.size()) {
      key_type frontv = buckets[cur + 1][0];
      buckets[cur + 1].pop_front();
      buckets[cur].push_back(frontv);
      if (buckets[cur + 1].size() > 0) m[cur + 1] = buckets[cur + 1][0];
      cur++;
    }

    if (buckets.size() && !buckets.back().size()) buckets.pop_back(), m.pop_back();
    if (buckets.empty()) return end();
    return iterator(this, pos.idx);
  }

  iterator lower_bound(const key_type& x) const {
    if (buckets.empty()) return end();

    size_type b_idx = find_bucket(x), ok = buckets[b_idx].size(), ng = 0;
    while (ok > ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(buckets[b_idx][mid], x)) ng = mid + 1;
      else ok = mid;
    }

    return iterator(this, b_idx * b + ok);
  }

  iterator upper_bound(const key_type& x) const {
    if (buckets.empty()) return end();

    size_type b_idx = find_bucket(x), ok = buckets[b_idx].size(), ng = 0;
    while (ok > ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(x, buckets[b_idx][mid])) ok = mid;
      else ng = mid + 1;
    }

    return iterator(this, b_idx * b + ok);
  }

  iterator find(const key_type& x) const {
    iterator it = lower_bound(x);
    if (it != end() && !comp(x, *it) && !comp(*it, x)) return it;
    return end();
  }

  const key_type& operator[](size_type k) const { return buckets[k / b][k % b]; }

  const key_type& at(size_type k) const {
    if (k >= sz) throw std::out_of_range("SqrtBucketsSet::at: index out of range");
    return (*this)[k];
  }

  bool contains(const key_type& x) const { return find(x) != end(); }

  size_type count(const key_type& x) const {
    if constexpr (!Multi) return contains(x) ? 1 : 0;
    return upper_bound(x) - lower_bound(x);
  }

  size_type count_geq(const key_type& l) const { return end() - lower_bound(l); }

  size_type count_lt(const key_type& r) const { return lower_bound(r) - begin(); }

  size_type count_range(const key_type& l, const key_type& r) const {
    return comp(l, r) ? lower_bound(r) - lower_bound(l) : 0;
  }
};

template <
  typename Key,
  IsGroup G,
  bool Multi = false,
  typename Compare = std::less<Key>
>
class SqrtBucketsMap {
  public:
  using key_type = Key;
  using mapped_type = G::value_type;
  using value_type = std::pair<Key, mapped_type>;
  using key_compare = Compare;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = typename std::allocator_traits<std::allocator<value_type>>::pointer;
  using const_pointer = typename std::allocator_traits<std::allocator<value_type>>::const_pointer;
  using reference = value_type&;
  using const_reference = const value_type&;

  class value_compare {
    protected:
    key_compare comp;
    value_compare(key_compare c) : comp(c) {}

    public:
    using result_type = bool;
    using first_argument_type = value_type;
    using second_argument_type = value_type;

    bool operator()(const value_type& x, const value_type& y) const { return comp(x.first, y.first); }
  };

  private:
  struct Bucket {
    private:
    std::vector<value_type> data;
    size_type head, sz, cap, mask;
    G::value_type fold_;

    inline size_type idx(size_type i) const { return (head + i) & mask; }

    public:
    Bucket(std::size_t l) {
      head = sz = 0;
      cap = std::bit_ceil(l + 1);
      mask = cap - 1;
      data.resize(cap);
      fold_ = G::e();
    }

    size_type size() const { return sz; }

    G::value_type fold() const { return fold_; }

    const value_type& operator[](size_type k) const { return data[idx(k)]; }

    value_type& operator[](size_type k) { return data[idx(k)]; }

    value_type back() const { return data[idx(sz - 1)]; }

    void pop_front() {
      if (sz > 0) {
        fold_ = G::op(G::inv(data[idx(0)].second), fold_);
        head = (head + 1) & mask;
        sz--;
      }
    }

    void pop_back() {
      if (sz > 0) {
        sz--;
        fold_ = G::op(fold_, G::inv(data[idx(sz)].second));
      }
    }

    void push_front(const value_type& x) {
      head = (head - 1) & mask;
      data[head] = x;
      sz++;
      fold_ = G::op(x.second, fold_);
    }

    void push_back(const value_type& x) {
      data[idx(sz)] = x;
      sz++;
      fold_ = G::op(fold_, x.second);
    }

    void insert(size_type i, const value_type& x) {
      if (!sz) {
        push_front(x);
        return;
      }
      if (i == sz) {
        push_back(x);
        return;
      }

      size_type index = idx(i), tail = idx(sz);
      if (index < tail) std::copy_backward(data.begin() + index, data.begin() + tail, data.begin() + tail + 1);
      else {
        if (tail > 0) std::copy_backward(data.begin(), data.begin() + tail, data.begin() + tail + 1);
        data[0] = data.back();
        if (index < cap - 1) std::copy_backward(data.begin() + index, data.end() - 1, data.end());
      }

      data[index] = x;
      sz++;
      update();
    }

    void erase(size_type i) {
      if (!i) {
        pop_front();
        return;
      }
      if (i == sz - 1) {
        pop_back();
        return;
      }

      size_type index = idx(i), tail = idx(sz - 1);
      if (index < tail) std::copy(data.begin() + index + 1, data.begin() + tail + 1, data.begin() + index);
      else {
        if (index < cap - 1) std::copy(data.begin() + index + 1, data.end(), data.begin() + index);
        data[cap - 1] = data[0];
        if (tail > 0) std::copy(data.begin() + 1, data.begin() + tail + 1, data.begin());
      }
      sz--;
      update();
    }

    void update() {
      fold_ = G::e();
      for (size_type i = 0; i < sz; ++i) fold_ = G::op(fold_, data[idx(i)].second);
    }
  };

  std::vector<Bucket> buckets;
  std::vector<key_type> m;
  size_type b, sz;
  Compare comp;

  size_type find_bucket(const key_type& x) const {
    if (m.empty()) return 0;

    auto it = std::upper_bound(m.begin(), m.end(), x, [this](const key_type& a, const key_type& b) {
      return comp(a, b);
    });
    return it == m.begin() ? 0 : it - m.begin() - 1;
  }

  public:
  explicit SqrtBucketsMap(size_type B = 1024) : b(B), sz(0), comp(key_compare()), buckets(), m() {}

  SqrtBucketsMap(const SqrtBucketsMap& other)
    : b(other.b), sz(other.sz), comp(other.comp), buckets(other.buckets), m(other.m) {}

  SqrtBucketsMap(size_type B, const key_compare& c) : b(B), sz(0), comp(c), buckets(), m() {}

  SqrtBucketsMap(SqrtBucketsMap&& other) noexcept
    : b(other.b),
      sz(other.sz),
      comp(std::move(other.comp)),
      buckets(std::move(other.buckets)),
      m(std::move(other.m)) {
    other.sz = 0;
  }

  template <class Inputit>
  SqrtBucketsMap(size_type B, Inputit first, Inputit last)
    : b(B), sz(0), comp(key_compare()), buckets(), m() {
    this->insert(first, last);
  }

  template <class Inputit>
  SqrtBucketsMap(size_type B, Inputit first, Inputit last, const key_compare& c)
    : b(B), sz(0), comp(c), buckets(), m() {
    this->insert(first, last);
  }

  SqrtBucketsMap(std::initializer_list<key_type> init, size_type B = 1024, const key_compare& c = key_compare())
    : b(B), sz(0), comp(c), buckets(), m() {
    this->insert(init.begin(), init.end());
  }

  ~SqrtBucketsMap() = default;

  size_type size() const noexcept { return sz; }

  bool empty() const noexcept { return !sz; }

  void clear() noexcept {
    buckets.clear();
    m.clear();
    sz = 0;
  }

  void swap(SqrtBucketsMap& other) noexcept {
    std::swap(buckets, other.buckets);
    std::swap(m, other.m);
    std::swap(b, other.b);
    std::swap(sz, other.sz);
    std::swap(comp, other.comp);
  }

  SqrtBucketsMap& operator=(const SqrtBucketsMap& other) {
    if (this != &other) {
      SqrtBucketsMap tmp(other);
      this->swap(tmp);
    }
    return *this;
  }

  SqrtBucketsMap& operator=(SqrtBucketsMap&& other) noexcept {
    this->swap(other);
    return *this;
  }

  SqrtBucketsMap& operator=(std::initializer_list<key_type> init) {
    this->clear();
    this->insert(init.begin(), init.end());
    return *this;
  }

  class const_iterator {
    public:
    friend class SqrtBucketsMap;

    private:
    const SqrtBucketsMap* container;
    size_type idx;

    const_iterator(const SqrtBucketsMap* c, size_type i) : container(c), idx(i) {}

    public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::pair<Key, mapped_type>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    const_iterator() : container(nullptr), idx(0) {}

    reference operator*() const { return container->buckets[idx / container->b][idx % container->b]; }

    pointer operator->() const { return &(operator*()); }

    const_iterator& operator++() { idx++; return *this; }

    const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }

    const_iterator& operator--() { idx--; return *this; }

    const_iterator operator--(int) { const_iterator tmp = *this; --(*this); return tmp; }

    const_iterator operator+=(difference_type n) { idx += n; return *this; }

    const_iterator operator-=(difference_type n) { idx -= n; return *this; }

    const_iterator operator+(difference_type n) const { return const_iterator(container, idx + n); }

    const_iterator operator-(difference_type n) const { return const_iterator(container, idx - n); }

    friend const_iterator operator+(difference_type n, const const_iterator& it) { return it + n; }

    difference_type operator-(const const_iterator& other) const {
      return static_cast<difference_type>(idx) - static_cast<difference_type>(other.idx);
    }

    reference operator[](difference_type n) const { return *(*this + n); }

    bool operator==(const const_iterator& other) const {
      return container == other.container && idx == other.idx;
    }

    bool operator!=(const const_iterator& other) const { return !(*this == other); }

    bool operator<(const const_iterator& other) const { return idx < other.idx; }

    bool operator<=(const const_iterator& other) const { return idx <= other.idx; }

    bool operator>(const const_iterator& other) const { return idx > other.idx; }

    bool operator>=(const const_iterator& other) const { return idx >= other.idx; }
  };

  using iterator = const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() const noexcept { return iterator(this, 0); }

  iterator end() const noexcept { return iterator(this, sz); }

  reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }

  reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

  const_iterator cbegin() const noexcept { return begin(); }

  const_iterator cend() const noexcept { return end(); }

  const_reverse_iterator crbegin() const noexcept { return rbegin(); }

  const_reverse_iterator crend() const noexcept { return rend(); }

  void insert(const value_type& x) {
    if (buckets.empty()) {
      buckets.emplace_back(b);
      buckets.back().push_front(x);
      m.push_back(x.first);
      sz++;
      return;
    }

    size_type idx = find_bucket(x.first), ok = 0, ng = buckets[idx].size();
    while (ok < ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(buckets[idx][mid].first, x.first)) ok = mid + 1;
      else ng = mid;
    }
    if constexpr (!Multi) {
      if (ok < buckets[idx].size() && !comp(x.first, buckets[idx][ok].first) && !comp(buckets[idx][ok].first, x.first)) return;
    }
    buckets[idx].insert(ok, x);
    sz++;
    if (!ok) m[idx] = x.first;

    size_type cur = idx;
    while (cur < buckets.size() && buckets[cur].size() > b) {
      value_type backv = buckets[cur].back();
      buckets[cur].pop_back();
      if (cur + 1 == buckets.size()) {
        buckets.emplace_back(b);
        buckets.back().push_front(backv);
        m.push_back(backv.first);
        break;
      }
      else {
        cur++;
        buckets[cur].push_front(backv);
        m[cur] = backv.first;
      }
    }
  }

  void set(const key_type& key, const G::value_type& val) {
    auto it = lower_bound(key);
    if (it != end() && !comp(key, it->first) && !comp(it->first, key)) {
      size_type b_idx = it.idx / b, e_idx = it.idx % b;
      buckets[b_idx][e_idx].second = val;
      buckets[b_idx].update();
    }
    else insert({key, val});
  }

  size_type erase(const key_type& x) {
    if (buckets.empty()) return 0;

    size_type idx = find_bucket(x), ok = 0, ng = buckets[idx].size();
    while (ok < ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(buckets[idx][mid].first, x)) ok = mid + 1;
      else ng = mid;
    }
    if (ok == buckets[idx].size() || comp(x, buckets[idx][ok].first)) return 0;
    buckets[idx].erase(ok);
    sz--;
    if (buckets[idx].size() > 0 && !ok) m[idx] = buckets[idx][0].first;

    size_type cur = idx;
    while (cur + 1 < buckets.size()) {
      value_type frontv = buckets[cur + 1][0];
      buckets[cur + 1].pop_front();
      buckets[cur].push_back(frontv);
      if (buckets[cur + 1].size() > 0) m[cur + 1] = buckets[cur + 1][0].first;
      cur++;
    }
    if (buckets.size() && !buckets.back().size()) buckets.pop_back(), m.pop_back();

    return 1;
  }

  template <class Inputit>
  void insert(Inputit first, Inputit last) {
    for (auto it = first; it != last; ++it) insert(*it);
  }

  iterator erase(iterator pos) {
    if (pos == end() || pos.container != this) return end();
    
    size_type b_idx = pos.idx / b, e_idx = pos.idx % b;
    buckets[b_idx].erase(e_idx);
    sz--;
    if (buckets[b_idx].size() > 0 && !e_idx) m[b_idx] = buckets[b_idx][0].first;

    size_type cur = b_idx;
    while (cur + 1 < buckets.size()) {
      value_type frontv = buckets[cur + 1][0];
      buckets[cur + 1].pop_front();
      buckets[cur].push_back(frontv);
      if (buckets[cur + 1].size() > 0) m[cur + 1] = buckets[cur + 1][0].first;
      cur++;
    }

    if (buckets.size() && !buckets.back().size()) buckets.pop_back(), m.pop_back();
    if (buckets.empty()) return end();
    return iterator(this, pos.idx);
  }

  G::value_type fold(const key_type& l, const key_type& r) const {
    mapped_type res = G::e();
    if (comp(r, l) || (!comp(l, r) && !comp(r, l))) return res;

    auto it_l = lower_bound(l), it_r = lower_bound(r);
    if (it_l == it_r) return res;

    size_type idx_l = it_l.idx, idx_r = it_r.idx, b_l = idx_l / b, b_r = idx_r / b;
    if (b_l == b_r) {
      for (size_type i = idx_l % b; i < idx_r % b; ++i) res = G::op(res, buckets[b_l][i].second);
    }
    else {
      for (size_type i = idx_l % b; i < buckets[b_l].size(); ++i) res = G::op(res, buckets[b_l][i].second);
      for (size_type i = b_l + 1; i < b_r; ++i) res = G::op(res, buckets[i].fold());
      if (b_r < buckets.size()) {
        for (size_type i = 0; i < idx_r % b; ++i) res = G::op(res, buckets[b_r][i].second);
      }
    }

    return res;
  }

  iterator lower_bound(const key_type& x) const {
    if (buckets.empty()) return end();

    size_type b_idx = find_bucket(x), ok = buckets[b_idx].size(), ng = 0;
    while (ok > ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(buckets[b_idx][mid].first, x)) ng = mid + 1;
      else ok = mid;
    }

    return iterator(this, b_idx * b + ok);
  }

  iterator upper_bound(const key_type& x) const {
    if (buckets.empty()) return end();

    size_type b_idx = find_bucket(x), ok = buckets[b_idx].size(), ng = 0;
    while (ok > ng) {
      size_type mid = (ok + ng) / 2;
      if (comp(x, buckets[b_idx][mid].first)) ok = mid;
      else ng = mid + 1;
    }

    return iterator(this, b_idx * b + ok);
  }

  iterator find(const key_type& x) const {
    iterator it = lower_bound(x);
    if (it != end() && !comp(x, (*it).first) && !comp((*it).first, x)) return it;
    return end();
  }

  G::value_type operator[](const key_type& x) const {
    iterator it = find(x);
    return it != end() ? (*it).second : G::e();
  }

  G::value_type at(const key_type& k) const { return (*this)[k]; }

  bool contains(const key_type& x) const { return find(x) != end(); }

  size_type count(const key_type& x) const {
    if constexpr (!Multi) return contains(x) ? 1 : 0;
    return upper_bound(x) - lower_bound(x);
  }

  size_type count_geq(const key_type& l) const { return end() - lower_bound(l); }

  size_type count_lt(const key_type& r) const { return lower_bound(r) - begin(); }

  size_type count_range(const key_type& l, const key_type& r) const {
    return comp(l, r) ? lower_bound(r) - lower_bound(l) : 0;
  }
};