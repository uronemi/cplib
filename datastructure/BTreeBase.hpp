#pragma once

template <typename Key, typename Value, std::size_t B>
struct BTreeNode {
  bool is_leaf_;
  std::uint16_t item_count_;
  std::uint32_t subtree_size_;

  Key key_[2 * B - 1];
  Value val_[2 * B - 1];
  BTreeNode *child_[2 * B];
  std::uint32_t child_size_[2 * B];

  constexpr BTreeNode(bool is_leaf) noexcept
    : is_leaf_(is_leaf), item_count_(0), subtree_size_(0), child_{}, child_size_{} {}

  template <typename Compare>
  constexpr std::uint16_t find_lower_bound(const Key& target, const Compare& comp) const noexcept {
    [[assume(item_count_ < 2 * B)]];
    if constexpr (B <= 8) {
      std::uint16_t i = 0;
      while (i < item_count_ && comp(key_[i], target)) ++i;
      return i;
    }
    else {
      std::uint16_t base = 0, len = item_count_;
      while (len > 0) {
        std::uint16_t half = len >> 1, mid = base + half;
        if (comp(key_[mid], target)) {
          base = mid + 1;
          len = len - half - 1;
        }
        else len = half;
      }
      return base;
    }
  }

  template <typename Compare>
  constexpr std::uint16_t find_upper_bound(const Key& target, const Compare& comp) const noexcept {
    [[assume(item_count_ < 2 * B)]];
    if constexpr (B <= 8) {
      std::uint16_t i = 0;
      while (i < item_count_ && !comp(target, key_[i])) ++i;
      return i;
    }
    else {
      std::uint16_t base = 0, len = item_count_;
      while (len > 0) {
        std::uint16_t half = len >> 1, mid = base + half;
        if (comp(target, key_[mid])) len = half;
        else {
          base = mid + 1;
          len = len - half - 1;
        }
      }
      return base;
    }
  }
};

template <typename Key, std::size_t B>
struct BTreeNode<Key, void, B> {
  bool is_leaf_;
  std::uint16_t item_count_;
  std::uint32_t subtree_size_;

  Key key_[2 * B - 1];
  BTreeNode *child_[2 * B];
  std::uint32_t child_size_[2 * B];

  constexpr BTreeNode(bool is_leaf) noexcept
    : is_leaf_(is_leaf), item_count_(0), subtree_size_(0) {
    std::fill(std::begin(child_), std::end(child_), nullptr);
    std::fill(std::begin(child_size_), std::end(child_size_), 0);
  }

  template <typename Compare>
  constexpr std::uint16_t find_lower_bound(const Key& target, const Compare& comp) const noexcept {
    [[assume(item_count_ < 2 * B)]];
    if constexpr (B <= 8) {
      std::uint16_t i = 0;
      while (i < item_count_ && comp(key_[i], target)) ++i;
      return i;
    }
    else {
      std::uint16_t base = 0, len = item_count_;
      while (len > 0) {
        std::uint16_t half = len >> 1, mid = base + half;
        if (comp(key_[mid], target)) {
          base = mid + 1;
          len = len - half - 1;
        }
        else len = half;
      }
      return base;
    }
  }

  template <typename Compare>
  constexpr std::uint16_t find_upper_bound(const Key& target, const Compare& comp) const noexcept {
    [[assume(item_count_ < 2 * B)]];
    if constexpr (B <= 8) {
      std::uint16_t i = 0;
      while (i < item_count_ && !comp(target, key_[i])) ++i;
      return i;
    }
    else {
      std::uint16_t base = 0, len = item_count_;
      while (len > 0) {
        std::uint16_t half = len >> 1, mid = base + half;
        if (comp(target, key_[mid])) len = half;
        else {
          base = mid + 1;
          len = len - half - 1;
        }
      }
      return base;
    }
  }
};

template <typename Key, typename Value, std::size_t B, std::size_t MaxHeight = 16>
struct TraversalPath {
  struct Step {
    BTreeNode<Key, Value, B> *node;
    std::uint16_t idx;
  };
  std::array<Step, MaxHeight> step_;
  std::size_t size_ = 0;

  constexpr void push(BTreeNode<Key, Value, B> *node, std::uint16_t idx) noexcept {
    [[assume(size_ < MaxHeight)]];
    step_[size_++] = {node, idx};
  }

  constexpr Step pop() noexcept {
    [[assume(size_ > 0)]];
    return step_[--size_];
  }

  constexpr void clear() noexcept { size_ = 0; }

  [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
};

template <typename T>
struct ArrowProxy {
  T value_;
  constexpr const T* operator->() const noexcept { return &value_; }
  constexpr T* operator->() noexcept { return &value_; }
};

template <typename K, typename V>
struct BTreeTraits {
  using value_type = std::pair<const K, V>;
  using reference = std::pair<const K&, V&>;
};

template <typename K>
struct BTreeTraits<K, void> {
  using value_type = K;
  using reference = const K&;
};

template <typename Key, typename Value, std::size_t B>
class BTreeIterator {
  private:
  using Node = BTreeNode<Key, Value, B>;
  Node *node_ = nullptr;
  std::uint16_t idx_ = 0;
  Node *root_ = nullptr;
  TraversalPath<Key, Value, B> path_;

  template <typename K, typename V, class C, std::size_t N, typename A>
  friend class BTreeBase;

  static constexpr bool is_set_ = std::is_void_v<Value>;

  public:
  using value_type = typename BTreeTraits<Key, Value>::value_type;
  using reference = typename BTreeTraits<Key, Value>::reference;
  using pointer = std::conditional_t<is_set_, const Key*, ArrowProxy<reference>>;
  using iterator_category = std::bidirectional_iterator_tag;
  using difference_type = std::ptrdiff_t;

  constexpr BTreeIterator() noexcept = default;
  constexpr BTreeIterator(Node *node, std::uint16_t idx, Node *root,
                          TraversalPath<Key, Value, B> Path) noexcept
    : node_(node), idx_(idx), root_(root), path_(Path) {}

  template <typename V = Value>
  [[nodiscard]] constexpr reference operator*(this auto&& self) noexcept {
    if constexpr (is_set_) return self.node_->key_[self.idx_];
    else {
      return std::pair<const Key&, V&>(
        self.node_->key_[self.idx_],
        self.node_->val_[self.idx_]
      );
    }
  }

  template <typename V = Value>
  [[nodiscard]] constexpr pointer operator->(this auto&& self) noexcept {
    if constexpr (is_set_) return &(self.node_->key_[self.idx_]);
    else {
      return ArrowProxy<reference>{
        std::pair<const Key&, V&>(
          self.node_->key_[self.idx_],
          self.node_->val_[self.idx_]
        )
      };
    }
  }

  constexpr auto& operator++(this auto& self) noexcept {
    if (!self.node_) return self;

    if (!self.node_->is_leaf_) {
      self.path_.push(self.node_, self.idx_ + 1);
      self.node_ = self.node_->child_[self.idx_ + 1];
      while (!self.node_->is_leaf_) {
        self.path_.push(self.node_, 0);
        self.node_ = self.node_->child_[0];
      }
      self.idx_ = 0;
    }
    else {
      self.idx_++;
      if (self.idx_ >= self.node_->item_count_) {
        while (self.path_.size_ > 0) {
          auto parent = self.path_.pop();
          if (parent.idx < parent.node->item_count_) {
            self.node_ = parent.node;
            self.idx_ = parent.idx;
            return self;
          }
        }
        self.node_ = nullptr;
        self.idx_ = 0;
      }
    }
    
    return self;
  }

  constexpr auto& operator--(this auto& self) noexcept {
    if (!self.node_) {
      if (!self.root_) return self;
      self.node_ = self.root_;
      self.path_.clear();
      while (!self.node_->is_leaf_) {
        std::uint16_t last = self.node_->item_count_;
        self.path_.push(self.node_, last);
        self.node_ = self.node_->child_[last];
      }
      self.idx_ = self.node_->item_count_ - 1;
      return self;
    }

    if (!self.node_->is_leaf_) {
      self.path_.push(self.node_, self.idx_);
      self.node_ = self.node_->child_[self.idx_];
      while (!self.node_->is_leaf_) {
        std::uint16_t last = self.node_->item_count_;
        self.path_.push(self.node_, last);
        self.node_ = self.node_->child_[last];
      }
      self.idx_ = self.node_->item_count_ - 1;
    }
    else {
      if (self.idx_ > 0) self.idx_--;
      else {
        bool found = false;
        while (self.path_.size_ > 0) {
          auto parent = self.path_.pop();
          if (parent.idx > 0) {
            self.node_ = parent.node;
            self.idx_ = parent.idx - 1;
            found = true;
            break;
          }
        }
        if (!found) {
          self.node_ = nullptr;
          self.idx_ = 0;
        }
      }
    }
    
    return self;
  }

  [[nodiscard]] constexpr bool operator==(const BTreeIterator& other) const noexcept {
    return node_ == other.node_ && idx_ == other.idx_;
  }
};

template <
  typename Key,
  typename Value,
  class Compare = std::less<Key>,
  std::size_t B = 16,
  typename Alloc = std::allocator<Key>
>
class BTreeBase {
  public:
  using iterator = BTreeIterator<Key, Value, B>;

  protected:
  using Node = BTreeNode<Key, Value, B>;
  using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  using AllocTraits = std::allocator_traits<NodeAlloc>;

  [[no_unique_address]] NodeAlloc alloc_;
  [[no_unique_address]] Compare comp_;
  Node *root_ = nullptr;
  std::size_t tree_size_ = 0;

  constexpr Node* allocate_node(bool is_leaf) {
    Node *node = AllocTraits::allocate(alloc_, 1);
    AllocTraits::construct(alloc_, node, is_leaf);
    return node;
  }

  constexpr void deallocate_node(Node *node) noexcept {
    if (!node) return;
    AllocTraits::destroy(alloc_, node);
    AllocTraits::deallocate(alloc_, node, 1);
  }

  constexpr void clear_tree(Node *node) noexcept {
    if (!node) return;
    if (!node->is_leaf_) {
      for (std::uint16_t i = 0; i <= node->item_count_; ++i) clear_tree(node->child_[i]);
    }
    deallocate_node(node);
  }

  constexpr void calc_size(Node *node) noexcept {
    node->subtree_size_ = node->item_count_;
    if (!node->is_leaf_) {
      for (std::uint16_t i = 0; i <= node->item_count_; ++i) {
        node->subtree_size_ += node->child_size_[i];
      }
    }
  }

  constexpr void split_child(Node *parent, std::uint16_t idx) {
    Node *child = parent->child_[idx], *new_node = allocate_node(child->is_leaf_);

    constexpr std::uint16_t mid = B - 1;
    new_node->item_count_ = mid;

    // 1. Move the second half to the new node
    for (std::uint16_t i = 0; i < mid; ++i) {
      new_node->key_[i] = std::move(child->key_[i + B]);
      child->key_[i + B] = Key();
      if constexpr (!std::is_void_v<Value>) {
        new_node->val_[i] = std::move(child->val_[i + B]);
        child->val_[i + B] = Value();
      }
    }
    if (!child->is_leaf_) {
      for (std::uint16_t i = 0; i <= mid; ++i) {
        new_node->child_[i] = child->child_[i + B];
        new_node->child_size_[i] = child->child_size_[i + B];
      }
    }
    child->item_count_ = mid;

    calc_size(new_node);
    calc_size(child);

    // 2. Shift the array of parent
    for (std::uint16_t i = parent->item_count_; i > idx; --i) {
      parent->key_[i] = std::move(parent->key_[i - 1]);
      if constexpr (!std::is_void_v<Value>) parent->val_[i] = std::move(parent->val_[i - 1]);
      parent->child_[i + 1] = parent->child_[i];
      parent->child_size_[i + 1] = parent->child_size_[i];
    }

    // 3. Put the middle key in the parent
    parent->key_[idx] = std::move(child->key_[mid]);
    child->key_[mid] = Key();
    if constexpr (!std::is_void_v<Value>) {
      parent->val_[idx] = std::move(child->val_[mid]);
      child->val_[mid] = Value();
    }
    parent->child_[idx + 1] = new_node;
    parent->child_size_[idx] = child->subtree_size_;
    parent->child_size_[idx + 1] = new_node->subtree_size_;
    parent->item_count_++;
  }

  constexpr Node* clone_tree(const Node* node) {
    if (!node) return nullptr;
    Node* new_node = allocate_node(node->is_leaf_);
    new_node->item_count_ = node->item_count_;
    new_node->subtree_size_ = node->subtree_size_;
    for (std::uint16_t i = 0; i < node->item_count_; ++i) {
      new_node->key_[i] = node->key_[i];
      if constexpr (!std::is_void_v<Value>) new_node->val_[i] = node->val_[i];
    }
    if (!node->is_leaf_) {
      for (std::uint16_t i = 0; i <= node->item_count_; ++i) {
        new_node->child_[i] = clone_tree(node->child_[i]);
        new_node->child_size_[i] = node->child_size_[i];
      }
    }
    return new_node;
  }
  
  public:
  constexpr BTreeBase() noexcept = default;
  constexpr BTreeBase(const Alloc& alloc) noexcept
    : alloc_(alloc), comp_(Compare{}) {}
  constexpr explicit BTreeBase(const Compare& comp, const Alloc& alloc = Alloc()) noexcept
    : alloc_(alloc), comp_(comp) {}

  constexpr BTreeBase(const BTreeBase& other)
    : alloc_(other.alloc_), comp_(other.comp_), root_(nullptr), tree_size_(other.tree_size_) {
    root_ = clone_tree(other.root_);
  }
  
  constexpr BTreeBase(BTreeBase&& other) noexcept
    : alloc_(std::move(other.alloc_)), comp_(std::move(other.comp_)),
      root_(std::exchange(other.root_, nullptr)), tree_size_(std::exchange(other.tree_size_, 0)) {}
  
  constexpr BTreeBase& operator=(const BTreeBase& other) {
    if (this != &other) {
      clear_tree(root_);
      alloc_ = other.alloc_;
      comp_ = other.comp_;
      root_ = clone_tree(other.root_);
      tree_size_ = other.tree_size_;
    }
    return *this;
  }
  
  constexpr BTreeBase& operator=(BTreeBase&& other) noexcept {
    if (this != &other) {
      clear_tree(root_);
      alloc_ = std::move(other.alloc_);
      comp_ = std::move(other.comp_);
      root_ = std::exchange(other.root_, nullptr);
      tree_size_ = std::exchange(other.tree_size_, 0);
    }
    return *this;
  }
  
  constexpr virtual ~BTreeBase() { clear_tree(root_); }

  [[nodiscard]] constexpr std::size_t get_size() const noexcept { return tree_size_; }
  [[nodiscard]] constexpr bool is_empty() const noexcept { return tree_size_ == 0; }

  [[nodiscard]] constexpr iterator begin() const noexcept {
    if (!root_) return end();
    Node *cur = root_;
    TraversalPath<Key, Value, B> path;
    while (!cur->is_leaf_) {
      path.push(cur, 0);
      cur = cur->child_[0];
    }
    return iterator(cur, 0, root_, path);
  }

  [[nodiscard]] constexpr iterator end() const noexcept {
    return iterator(nullptr, 0, root_, TraversalPath<Key, Value, B>());
  }

  [[nodiscard]] constexpr iterator at(std::size_t idx) const noexcept {
    if (idx >= tree_size_) return end();

    Node *cur = root_;
    std::size_t rem = idx;
    TraversalPath<Key, Value, B> path;
    while (cur) {
      if (cur->is_leaf_) return iterator(cur, static_cast<std::uint16_t>(rem), root_, path);

      bool is_descended = false;
      std::uint16_t cnt = cur->item_count_;
      [[assume(cnt + 1 <= 2 * B)]];

      for (std::uint16_t i = 0; i <= cnt; ++i) {
        std::uint32_t left = cur->child_size_[i];
        if (rem < left) {
          path.push(cur, i);
          cur = cur->child_[i];
          is_descended = true;
          break;
        }
        rem -= left;
        if (i < cnt) {
          if (rem == 0) return iterator(cur, i, root_, path);
          rem--;
        }
      }
      if (!is_descended) break;
    }

    return end();
  }

  constexpr void swap(BTreeBase& other) noexcept {
    std::swap(alloc_, other.alloc_);
    std::swap(comp_, other.comp_);
    std::swap(root_, other.root_);
    std::swap(tree_size_, other.tree_size_);
  }

  protected:
  constexpr void borrow_from_left(Node *parent, std::uint16_t p_idx, Node *left, Node *cur) {
    // 1. Shift all elements of cur to the right
    for (std::uint16_t i = cur->item_count_; i > 0; --i) {
      cur->key_[i] = std::move(cur->key_[i - 1]);
      if constexpr (!std::is_void_v<Value>) cur->val_[i] = std::move(cur->val_[i - 1]);
    }
    if (!cur->is_leaf_) {
      for (std::uint16_t i = cur->item_count_ + 1; i > 0; --i) {
        cur->child_[i] = cur->child_[i - 1];
        cur->child_size_[i] = cur->child_size_[i - 1];
      }
    }

    // 2. Sift down the element of parent to the front of cur
    cur->key_[0] = std::move(parent->key_[p_idx - 1]);
    if constexpr (!std::is_void_v<Value>) cur->val_[0] = std::move(parent->val_[p_idx - 1]);

    // 3. Sift up the back of left to parent
    std::uint16_t l_back = left->item_count_ - 1;
    parent->key_[p_idx - 1] = std::move(left->key_[l_back]);
    left->key_[l_back] = Key();
    if constexpr (!std::is_void_v<Value>) {
      parent->val_[p_idx - 1] = std::move(left->val_[l_back]);
      left->val_[l_back] = Value();
    }

    // 4. Move the back child of left to the front child of cur
    if (!cur->is_leaf_) {
      cur->child_[0] = left->child_[l_back + 1];
      cur->child_size_[0] = left->child_size_[l_back + 1];
    }

    left->item_count_--;
    cur->item_count_++;
    calc_size(left);
    calc_size(cur);
  }

  constexpr void borrow_from_right(Node *parent, std::uint16_t p_idx, Node *cur, Node *right) {
    // 1. Sift down the element of parent to the back of cur
    std::uint16_t c_len = cur->item_count_;
    cur->key_[c_len] = std::move(parent->key_[p_idx]);
    if constexpr (!std::is_void_v<Value>) cur->val_[c_len] = std::move(parent->val_[p_idx]);

    // 2. Move the front child of right to the back child of cur
    if (!cur->is_leaf_) {
      cur->child_[c_len + 1] = right->child_[0];
      cur->child_size_[c_len + 1] = right->child_size_[0];
    }

    // 3. Sift up the front of right to parent
    parent->key_[p_idx] = std::move(right->key_[0]);
    if constexpr (!std::is_void_v<Value>) parent->val_[p_idx] = std::move(right->val_[0]);

    // 4. Shift all elements of right to the left
    for (std::uint16_t i = 0; i < right->item_count_ - 1; ++i) {
      right->key_[i] = std::move(right->key_[i + 1]);
      if constexpr (!std::is_void_v<Value>) right->val_[i] = std::move(right->val_[i + 1]);
    }
    if (!right->is_leaf_) {
      for (std::uint16_t i = 0; i < right->item_count_; ++i) {
        right->child_[i] = right->child_[i + 1];
        right->child_size_[i] = right->child_size_[i + 1];
      }
    }

    right->key_[right->item_count_ - 1] = Key();
    if constexpr (!std::is_void_v<Value>) right->val_[right->item_count_ - 1] = Value();

    right->item_count_--;
    cur->item_count_++;
    calc_size(right);
    calc_size(cur);
  }

  constexpr void merge_node(Node *parent, std::uint16_t p_idx, Node *left, Node *right) {
    std::uint16_t l_len = left->item_count_;

    // 1. Sift down the element of parent to the back of left
    left->key_[l_len] = std::move(parent->key_[p_idx]);
    if constexpr (!std::is_void_v<Value>) left->val_[l_len] = std::move(parent->val_[p_idx]);

    // 2. Append all elements and childs of right to left
    for (std::uint16_t i = 0; i < right->item_count_; ++i) {
      left->key_[l_len + 1 + i] = std::move(right->key_[i]);
      if constexpr (!std::is_void_v<Value>) left->val_[l_len + 1 + i] = std::move(right->val_[i]);
    }
    if (!left->is_leaf_) {
      for (std::uint16_t i = 0; i <= right->item_count_; ++i) {
        left->child_[l_len + 1 + i] = right->child_[i];
        left->child_size_[l_len + 1 + i] = right->child_size_[i];
      }
    }
    left->item_count_ += right->item_count_ + 1;

    // 3. Shift elements of parent to the left
    for (std::uint16_t i = p_idx; i < parent->item_count_ - 1; ++i) {
      parent->key_[i] = std::move(parent->key_[i + 1]);
      if constexpr (!std::is_void_v<Value>) parent->val_[i] = std::move(parent->val_[i + 1]);
      parent->child_[i + 1] = parent->child_[i + 2];
      parent->child_size_[i + 1] = parent->child_size_[i + 2];
    }
    
    parent->key_[parent->item_count_ - 1] = Key();
    if constexpr (!std::is_void_v<Value>) parent->val_[parent->item_count_ - 1] = Value();
    
    parent->item_count_--;

    deallocate_node(right);
    calc_size(left);
  }

  private:
  template <typename V>
  constexpr std::pair<iterator, bool> insert_impl(const Key& key, const V& val) {
    if (!root_) {
      root_ = allocate_node(true);
      root_->key_[0] = key;
      if constexpr (!std::is_void_v<Value>) root_->val_[0] = val;
      root_->item_count_ = 1;
      root_->subtree_size_ = 1;
      tree_size_ = 1;
      return {iterator(root_, 0, root_, {}), true};
    }

    TraversalPath<Key, Value, B> path;
    Node *cur = root_;

    while (true) {
      std::uint16_t idx = cur->find_lower_bound(key, comp_);
      if (idx < cur->item_count_ && !comp_(key, cur->key_[idx])) {
        return {iterator(cur, idx, root_, path), false};
      }
      if (cur->is_leaf_) {
        path.push(cur, idx);
        break;
      }
      path.push(cur, idx);
      cur = cur->child_[idx];
    }

    auto step = path.pop();
    cur = step.node;
    std::uint16_t idx = step.idx;

    for (std::uint16_t i = cur->item_count_; i > idx; --i) {
      cur->key_[i] = std::move(cur->key_[i - 1]);
      if constexpr (!std::is_void_v<Value>) cur->val_[i] = std::move(cur->val_[i - 1]);
    }
    cur->key_[idx] = key;
    if constexpr (!std::is_void_v<Value>) cur->val_[idx] = val;
    cur->item_count_++;
    cur->subtree_size_++;
    tree_size_++;

    Node *child = nullptr;
    while (cur->item_count_ == 2 * B - 1) {
      if (path.size_ == 0) {
        Node *new_root = allocate_node(false);
        new_root->child_[0] = cur;
        new_root->child_size_[0] = cur->subtree_size_;
        split_child(new_root, 0);
        calc_size(new_root);
        root_ = new_root;
        break;
      }

      auto parent_step = path.pop();
      Node *parent = parent_step.node;
      std::uint16_t p_idx = parent_step.idx;

      split_child(parent, p_idx);
      calc_size(parent);
      cur = parent;
    }

    for (std::size_t i = 0; i < path.size_; ++i) {
      path.step_[i].node->subtree_size_++;
      path.step_[i].node->child_size_[path.step_[i].idx]++;
    }

    return {find(key), true};
  }

  public:
  [[nodiscard]] constexpr iterator find(const Key& key) const noexcept {
    Node *cur = root_;
    TraversalPath<Key, Value, B> path;
    while (cur) {
      std::uint16_t idx = cur->find_lower_bound(key, comp_);
      if (idx < cur->item_count_ && !comp_(key, cur->key_[idx])) {
        return iterator(cur, idx, root_, path);
      }
      if (cur->is_leaf_) break;
      path.push(cur, idx);
      cur = cur->child_[idx];
    }
    return end();
  }

  [[nodiscard]] constexpr iterator lower_bound(const Key& key) const noexcept {
    Node *cur = root_;
    iterator res = end();
    TraversalPath<Key, Value, B> path, best_path;
    while (cur) {
      std::uint16_t idx = cur->find_lower_bound(key, comp_);
      if (idx < cur->item_count_) {
        best_path = path;
        res = iterator(cur, idx, root_, best_path);
        if (!comp_(key, cur->key_[idx])) return res;
      }
      if (cur->is_leaf_) break;
      path.push(cur, idx);
      cur = cur->child_[idx];
    }
    return res;
  }

  [[nodiscard]] constexpr iterator upper_bound(const Key& key) const noexcept {
    Node *cur = root_;
    iterator res = end();
    TraversalPath<Key, Value, B> path, best_path;
    while (cur) {
      std::uint16_t idx = cur->find_upper_bound(key, comp_);
      if (idx < cur->item_count_) {
        best_path = path;
        res = iterator(cur, idx, root_, best_path);
      }
      if (cur->is_leaf_) break;
      path.push(cur, idx);
      cur = cur->child_[idx];
    }
    return res;
  }

  [[nodiscard]] constexpr std::size_t order_of_key(const Key& key) const noexcept {
    Node *cur = root_;
    std::size_t ans = 0;
    while (cur) {
      std::uint16_t idx = cur->find_lower_bound(key, comp_);
      if (!cur->is_leaf_) {
        [[assume(idx < 2 * B)]];
        for (std::uint16_t i = 0; i < idx; ++i) {
          ans += cur->child_size_[i];
        }
      }
      ans += idx;

      if (idx < cur->item_count_ && !comp_(key, cur->key_[idx])) {
        if (!cur->is_leaf_) ans += cur->child_size_[idx];
        return ans;
      }
      if (cur->is_leaf_) break;
      cur = cur->child_[idx];
    }
    return ans;
  }

  template <typename V = Value>
  requires (std::is_void_v<V>)
  constexpr std::pair<iterator, bool> insert(const Key& key) {
    return insert_impl(key, 0);
  }

  template <typename V = Value>
  requires (!std::is_void_v<V>)
  constexpr std::pair<iterator, bool> insert(const Key& key, const V& value) {
    return insert_impl(key, value);
  }

  template <typename V>
  constexpr iterator insert_multi_impl(const Key& key, const V& val) {
    if (!root_) {
      root_ = allocate_node(true);
      root_->key_[0] = key;
      if constexpr (!std::is_void_v<Value>) root_->val_[0] = val;
      root_->item_count_ = 1;
      root_->subtree_size_ = 1;
      tree_size_ = 1;
      return iterator(root_, 0, root_, {});
    }

    TraversalPath<Key, Value, B> path;
    Node *cur = root_;

    while (true) {
      std::uint16_t idx = cur->find_upper_bound(key, comp_);
      if (cur->is_leaf_) {
        path.push(cur, idx);
        break;
      }
      path.push(cur, idx);
      cur = cur->child_[idx];
    }

    auto step = path.pop();
    cur = step.node;
    std::uint16_t idx = step.idx;

    for (std::uint16_t i = cur->item_count_; i > idx; --i) {
      cur->key_[i] = std::move(cur->key_[i - 1]);
      if constexpr (!std::is_void_v<Value>) cur->val_[i] = std::move(cur->val_[i - 1]);
    }
    cur->key_[idx] = key;
    if constexpr (!std::is_void_v<Value>) cur->val_[idx] = val;
    cur->item_count_++;
    cur->subtree_size_++;
    tree_size_++;

    while (cur->item_count_ == 2 * B - 1) {
      if (path.size_ == 0) {
        Node *new_root = allocate_node(false);
        new_root->child_[0] = cur;
        new_root->child_size_[0] = cur->subtree_size_;
        split_child(new_root, 0);
        calc_size(new_root);
        root_ = new_root;
        break;
      }
      auto parent_step = path.pop();
      Node *parent = parent_step.node;
      std::uint16_t p_idx = parent_step.idx;

      split_child(parent, p_idx);
      calc_size(parent);
      cur = parent;
    }

    for (std::size_t i = 0; i < path.size_; ++i) {
      path.step_[i].node->subtree_size_++;
      path.step_[i].node->child_size_[path.step_[i].idx]++;
    }

    auto it = upper_bound(key);
    --it;
    return it;
  }

  constexpr bool erase(const Key& key) noexcept {
    if (!root_) return false;

    TraversalPath<Key, Value, B> path;
    Node *cur = root_;
    std::uint16_t idx = 0;
    bool is_found = false;

    // 1. Search the element
    while (cur) {
      idx = cur->find_lower_bound(key, comp_);
      if (idx < cur->item_count_ && !comp_(key, cur->key_[idx])) {
        is_found = true;
        break;
      }
      if (cur->is_leaf_) break;
      path.push(cur, idx);
      cur = cur->child_[idx];
    }
    if (!is_found) return false;

    // 2. Erase the element internally
    if (!cur->is_leaf_) {
      path.push(cur, idx);
      Node *pred = cur->child_[idx];
      while (!pred->is_leaf_) {
        path.push(pred, pred->item_count_);
        pred = pred->child_[pred->item_count_];
      }
      cur->key_[idx] = std::move(pred->key_[pred->item_count_ - 1]);
      if constexpr (!std::is_void_v<Value>) {
        cur->val_[idx] = std::move(pred->val_[pred->item_count_ - 1]);
      }
      cur = pred;
      idx = pred->item_count_ - 1;
    }

    // 3. Erase the element physically
    for (std::uint16_t i = idx; i < cur->item_count_ - 1; ++i) {
      cur->key_[i] = std::move(cur->key_[i + 1]);
      if constexpr (!std::is_void_v<Value>) cur->val_[i] = std::move(cur->val_[i + 1]);
    }
    cur->key_[cur->item_count_ - 1] = Key();
    if constexpr (!std::is_void_v<Value>) cur->val_[cur->item_count_ - 1] = Value();
    
    cur->item_count_--;
    cur->subtree_size_--;
    tree_size_--;
    if (tree_size_ == 0) {
      deallocate_node(root_);
      root_ = nullptr;
      return true;
    }

    // 4. Fix unappropriate nodes
    constexpr std::uint16_t min_keys = B - 1;
    while (cur != root_ && cur->item_count_ < min_keys) {
      auto step = path.pop();
      Node *parent = step.node;
      std::uint16_t p_idx = step.idx;
      Node *left = (p_idx > 0 ? parent->child_[p_idx - 1] : nullptr);
      Node *right = (p_idx < parent->item_count_ ? parent->child_[p_idx + 1] : nullptr);

      if (left && left->item_count_ > min_keys) {
        borrow_from_left(parent, p_idx, left, cur);
        parent->child_size_[p_idx - 1] = left->subtree_size_;
        parent->child_size_[p_idx] = cur->subtree_size_;
      }
      else if (right && right->item_count_ > min_keys) {
        borrow_from_right(parent, p_idx, cur, right);
        parent->child_size_[p_idx] = cur->subtree_size_;
        parent->child_size_[p_idx + 1] = right->subtree_size_;
      }
      else {
        if (left) {
          merge_node(parent, p_idx - 1, left, cur);
          parent->child_size_[p_idx - 1] = left->subtree_size_;
          cur = left;
        }
        else {
          merge_node(parent, p_idx, cur, right);
          parent->child_size_[p_idx] = cur->subtree_size_;
        }
      }
      calc_size(parent);
      cur = parent;
    }

    // 5. Confirm weather deallocate root
    if (cur == root_ && cur->item_count_ == 0 && !cur->is_leaf_) {
      root_ = cur->child_[0];
      deallocate_node(cur);
    }
  
    // 6. Update ancestors
    while (path.size_ > 0) {
      auto step = path.pop();
      Node *p = step.node, *c = p->child_[step.idx];
      p->child_size_[step.idx] = (c ? c->subtree_size_ : 0);
      calc_size(p);
    }
    if (root_) calc_size(root_);
  
    return true;
  }

  [[nodiscard]] constexpr std::size_t rank(const iterator& it) const noexcept {
    if (!it.node_) return tree_size_;
    std::size_t r = 0;
    for (std::size_t i = 0; i < it.path_.size_; ++i) {
      auto step = it.path_.step_[i];
      for (std::uint16_t j = 0; j < step.idx; ++j) {
        if (!step.node->is_leaf_) r += step.node->child_size_[j];
        r++;
      }
    }
    for (std::uint16_t j = 0; j < it.idx_; ++j) {
      if (!it.node_->is_leaf_) r += it.node_->child_size_[j];
      r++;
    }
    if (!it.node_->is_leaf_) r += it.node_->child_size_[it.idx_];
    return r;
  }

  constexpr iterator erase(iterator pos) noexcept {
    if (!pos.node_) return end();

    std::size_t r = rank(pos);
    TraversalPath<Key, Value, B> path = pos.path_;
    Node *cur = pos.node_;
    std::uint16_t idx = pos.idx_;

    if (!cur->is_leaf_) {
      path.push(cur, idx);
      Node *pred = cur->child_[idx];
      while (!pred->is_leaf_) {
        path.push(pred, pred->item_count_);
        pred = pred->child_[pred->item_count_];
      }
      cur->key_[idx] = std::move(pred->key_[pred->item_count_ - 1]);
      if constexpr (!std::is_void_v<Value>) {
        cur->val_[idx] = std::move(pred->val_[pred->item_count_ - 1]);
      }
      cur = pred;
      idx = pred->item_count_ - 1;
    }

    for (std::uint16_t i = idx; i < cur->item_count_ - 1; ++i) {
      cur->key_[i] = std::move(cur->key_[i + 1]);
      if constexpr (!std::is_void_v<Value>) cur->val_[i] = std::move(cur->val_[i + 1]);
    }
    cur->key_[cur->item_count_ - 1] = Key();
    if constexpr (!std::is_void_v<Value>) cur->val_[cur->item_count_ - 1] = Value();
    
    cur->item_count_--;
    cur->subtree_size_--;
    tree_size_--;

    if (tree_size_ == 0) {
      deallocate_node(root_);
      root_ = nullptr;
      return end();
    }

    constexpr std::uint16_t min_keys = B - 1;
    while (cur != root_ && cur->item_count_ < min_keys) {
      auto step = path.pop();
      Node *parent = step.node;
      std::uint16_t p_idx = step.idx;
      Node *left = (p_idx > 0 ? parent->child_[p_idx - 1] : nullptr);
      Node *right = (p_idx < parent->item_count_ ? parent->child_[p_idx + 1] : nullptr);

      if (left && left->item_count_ > min_keys) {
        borrow_from_left(parent, p_idx, left, cur);
        parent->child_size_[p_idx - 1] = left->subtree_size_;
        parent->child_size_[p_idx] = cur->subtree_size_;
      }
      else if (right && right->item_count_ > min_keys) {
        borrow_from_right(parent, p_idx, cur, right);
        parent->child_size_[p_idx] = cur->subtree_size_;
        parent->child_size_[p_idx + 1] = right->subtree_size_;
      }
      else {
        if (left) {
          merge_node(parent, p_idx - 1, left, cur);
          parent->child_size_[p_idx - 1] = left->subtree_size_;
          cur = left;
        }
        else {
          merge_node(parent, p_idx, cur, right);
          parent->child_size_[p_idx] = cur->subtree_size_;
        }
      }
      calc_size(parent);
      cur = parent;
    }

    if (cur == root_ && cur->item_count_ == 0 && !cur->is_leaf_) {
      root_ = cur->child_[0];
      deallocate_node(cur);
    }

    while (path.size_ > 0) {
      auto step = path.pop();
      Node *p = step.node, *c = p->child_[step.idx];
      p->child_size_[step.idx] = (c ? c->subtree_size_ : 0);
      calc_size(p);
    }
    if (root_) calc_size(root_);

    return at(r);
  }

  constexpr iterator erase(iterator first, iterator last) noexcept {
    if (first == last) return last;
    std::size_t num_to_erase = rank(last) - rank(first);
    for (std::size_t i = 0; i < num_to_erase; ++i) first = erase(first);
    return first;
  }
};