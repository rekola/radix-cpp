#ifndef _RADIXCPP_H_
#define _RADIXCPP_H_

#include <cstdint>
#include <utility>
#include <string>
#include <stdexcept>

// #define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

namespace radix_cpp {
  inline constexpr uint32_t flag_is_assigned = 1;
  inline constexpr uint32_t flag_is_final = 2;
  inline constexpr uint32_t flag_has_children = 4;
  
  inline std::pair<size_t, std::uint8_t> remove_top(uint8_t key) noexcept {
    return std::pair(key, 0);
  }

  inline std::pair<size_t, std::uint16_t> remove_top(uint16_t key) noexcept {
    return std::pair(key & 0xff, key >> 8);
  }

  inline std::pair<size_t, std::uint32_t> remove_top(uint32_t key) noexcept {
    return std::pair(key & 0xff, key >> 8);
  }

  inline std::pair<size_t, std::uint64_t> remove_top(uint64_t key) noexcept {
    return std::pair(key & 0xff, key >> 8);
  }

  inline std::pair<size_t, float> remove_top(float key) noexcept {
    union {
      float f;
      uint32_t i;
    } u;
    u.f = key;
    return remove_top(u.i);
  }

  inline std::pair<size_t, double> remove_top(double key) noexcept {
    union {
      double f;
      uint64_t i;
    } u;
    u.f = key;
    return remove_top(u.i);
  }

  inline std::pair<size_t, std::string> remove_top(const std::string & key) noexcept {
    if (key.empty()) {
      return std::pair(0, key);
    } else {
      return std::pair(static_cast<uint8_t>(key.back()), key.substr(0, key.size() - 1));
    }
  }

  inline size_t keysize(uint8_t key) noexcept {
    return sizeof(key);
  }

  inline size_t keysize(uint16_t key) noexcept {
    return sizeof(key);
  }

  inline size_t keysize(uint32_t key) noexcept {
    return sizeof(key);
  }

  inline size_t keysize(uint64_t key) noexcept {
    return sizeof(key);
  }

  inline size_t keysize(float key) noexcept {
    return sizeof(float);
  }

  inline size_t keysize(double key) noexcept {
    return sizeof(float);
  }

  inline size_t keysize(const std::string & key) noexcept {
    return key.size();
  }

  /* MurmurHash3 was written by Austin Appleby, and is placed in the public domain.
     The author(s) hereby disclaim copyright to the MurmurHash3 source code.
  */

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  inline T rotl(T x, int_fast8_t r) noexcept {
    return (x << r) | (x >> (32 - r));
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  inline T rotl(T x, int_fast8_t r) noexcept {
    return (x << r) | (x >> (64 - r));
  }

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  inline T murmur3_mix_k1(T k1) noexcept {
    k1 *= 0xcc9e2d51;
    k1 = rotl(k1, 15);
    k1 *= 0x1b873593;
    return k1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  inline T murmur3_mix_k1(T k1) noexcept {
    k1 *= UINT64_C(0x87c37b91114253d5);
    k1 = rotl(k1, 31);
    k1 *= UINT64_C(0x4cf5ad432745937f);
    return k1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  inline T murmur3_mix_h1(T h1, T k1) noexcept {
    h1 ^= k1;
    h1 = rotl(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
    return h1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  inline T murmur3_mix_h1(T h1, T k1) noexcept {
    h1 ^= k1;
    h1 = rotl(h1, 27);
    h1 = h1 * 5 + 0x52dce729;
    return h1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  inline T murmur3_fmix(T h1) noexcept {
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  inline T murmur3_fmix(T h1) noexcept {
    h1 ^= h1 >> 33;
    h1 *= UINT64_C(0xff51afd7ed558ccd);
    h1 ^= h1 >> 33;
    h1 *= UINT64_C(0xc4ceb9fe1a85ec53);
    h1 ^= h1 >> 33;
    return h1;
  }

  template <typename Key, typename T>
  class Table {
  public:
    static constexpr bool is_map = !std::is_void<T>::value;
    static constexpr bool is_set = !is_map;
    static constexpr size_t bucket_count = 256; // bucket count for the ordered portion of the key
    static constexpr size_t max_load_factor100 = 60;
    
    using key_type = Key;
    using mapped_type = T;
    using value_type = typename std::conditional<is_set, Key, std::pair<Key, T>>::type;
    using size_type = size_t;
    using Self = Table<key_type, mapped_type>;

  private:
    struct Node {
      size_t hash;
      uint32_t depth;
      uint8_t flags, ordinal;
      key_type prefix_key;
    };

  public:

    template <bool IsConst>
    struct Iterator
    {
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = typename Self::value_type;
      using reference         = typename std::conditional<IsConst, value_type const&, value_type&>::type;
      using pointer           = typename std::conditional<IsConst, value_type const*, value_type*>::type;

      // end iterator
      Iterator(Self * table) noexcept
	: table_(table),
	  ordinal_(1),
	  offset_(0),
	  prefix_key_(),
	  depth_(0),
	  table_size_(table->table_size_) { }
      
      Iterator(Self * table, uint32_t depth, key_type prefix_key, size_t ordinal, size_t offset) noexcept
	: table_(table),
	  ordinal_(ordinal),
	  offset_(offset),
	  prefix_key_(std::move(prefix_key)),
	  depth_(depth),
      	  table_size_(table->table_size_) { }
      
      void set_indices(uint32_t depth, key_type prefix_key, size_t ordinal, size_t offset) noexcept {
	depth_ = depth;
	prefix_key_ = std::move(prefix_key);
	ordinal_ = ordinal;
	offset_ = offset;
      }

      reference operator*() const noexcept {
	size_t h = calc_hash(depth_, prefix_key_, ordinal_);
	return table_->read_keyval(h, offset_);
      }
      pointer operator->() noexcept {
	size_t h = calc_hash(depth_, prefix_key_, ordinal_);
	return &(table_->read_keyval(h, offset_));
      }
      Iterator& operator++() noexcept {
	if (table_size_ != table_->table_size_) {
	  // table size has changed => repair the iterator
	  
	  table_size_ = table_->table_size_;
	  offset_ = 0;
	  size_t h = calc_hash(depth_, prefix_key_, ordinal_);

	  while ( 1 ) {
	    auto & node = table_->read_node(h, offset_);
	    if (node.flags & flag_is_assigned &&
		node.depth == depth_ && node.ordinal == ordinal_ &&
		node.prefix_key == prefix_key_) {
	      break;
	    }
	    offset_++;
	  }
	}

	bool is_first = true;
	while ( !(depth_ == 0 && ordinal_ == 1) ) {
	  key_type prefix = prefix_key_;
	  uint32_t depth = depth_;
	  size_t ordinal = ordinal_, offset = 0;

	  if (depth == 0) {
	    depth++;
	    ordinal = 0;
	  } else if (!is_first) {
	    ordinal++;
	  } else {
	    size_t h = calc_hash(depth, prefix, ordinal);
	    auto & node = table_->read_node(h, offset);
	    if (node.flags & flag_is_final && node.flags & flag_has_children) {
	      depth++;
	      prefix = getFirstConst(table_->read_keyval(h, offset));
	      ordinal = 0;
	    } else {
	      ordinal++;
	    }
	    is_first = false;
	  }

	  size_t h = calc_hash(depth, prefix, ordinal);
	  bool found = false;
	  while ( ordinal < bucket_count ) {
	    auto & node = table_->read_node(h, offset);
	    if (!node.flags) {
	      ordinal++;
	      offset = 0;
	      h = calc_hash(depth, prefix, ordinal);
	      continue;
	    } else if (node.depth != depth || node.ordinal != ordinal || node.prefix_key != prefix) {
	      offset++;
	      continue;
	    }
	    found = true;
	    if (node.flags & flag_is_final) {
	      break;
	    } else {
	      depth++;
	      prefix = getFirstConst(table_->read_keyval(h, offset));
	      ordinal = offset = 0;
	      h = calc_hash(depth, prefix, ordinal);
	    }
	  }

	  if (ordinal < bucket_count) {
#ifdef DEBUG
	    std::cerr << "++ depth = " << depth << ", prefix = " << prefix << ", ordinal = " << ordinal << ", offset = " << offset << "\n";
#endif
	    set_indices(depth, prefix, ordinal, offset);
	    return *this;
	  } else if (found) {
#ifdef DEBUG
	    std::cerr << "could not find next subiterator\n";
#endif
	    abort();
	  }
	  // next element in the range was not found
	  down();
	}
	return *this;
      }
      Iterator operator++(int) noexcept {
	Iterator<IsConst> tmp = *this;
	++(*this);
	return tmp;
      }
      template <bool O>
      bool operator== (const Iterator<O>& o) const noexcept {
	return depth_ == o.depth_ && ordinal_ == o.ordinal_ && offset_ == o.offset_;
      }
      template <bool O>
      bool operator!= (const Iterator<O>& o) const noexcept {
	return depth_ != o.depth_ || ordinal_ != o.ordinal_ || offset_ != o.offset_;
      }

      void fast_forward() {
	// first look for 0-length node (depth = ordinal = 0)
	size_t h0 = calc_hash(0, key_type(), 0);
	size_t offset0 = 0;
	while (1) {
	  auto & node = table_->read_node(h0, offset0);
	  if (!node.flags) break;
	  else if (node.depth == 0) {
	    set_indices(0, key_type(), 0, offset0);
	    return;
	  } else {
	    offset0++;
	  }
	}

	auto prefix_key = key_type();
	while (1) {
	  size_t ordinal = 0, offset = 0;
	  size_t h = calc_hash(depth_ + 1, prefix_key, ordinal);
	  while (ordinal < bucket_count) {
	    auto & node = table_->read_node(h, offset);
	    if (!node.flags) {
	      ordinal++;
	      offset = 0;
	      h = calc_hash(depth_ + 1, prefix_key, ordinal);
	    } else if (depth_ + 1 == node.depth && node.ordinal == ordinal && node.prefix_key == prefix_key) {
	      break;
	    } else {
	      offset++;
	    }
	  }
	  if (ordinal < bucket_count) {
	    auto & node = table_->read_node(h, offset);
	    if (node.prefix_key != prefix_key) {
	      abort();
	    }
	    set_indices(depth_ + 1, prefix_key, ordinal, offset);
#ifdef DEBUG
	    std::cerr << "ff: depth = " << depth_ << ", prefix_key = " << prefix_key_ << ", ordinal = " << ordinal_ << ", offset = " << offset_ << "\n";
#endif
	    if (node.flags & flag_is_final) break;
	    prefix_key = getFirstConst(table_->read_keyval(h, offset));
	  } else {
#ifdef DEBUG
	    std::cerr << "fast forward: could not find next subiterator for prefix " << prefix_key << "\n";
#endif
	    abort();
	  }
	}
      }

      size_t get_offset() const { return offset_; }

    private:
      void down() {
	if (depth_ <= 1) {
	  // become an end iterator
	  ordinal_ = 1;
	  offset_ = depth_ = 0;
	  return;
	}
	size_t h = calc_hash(depth_, prefix_key_, ordinal_);
	auto & node = table_->read_node(h, offset_);
	if (!node.flags) {
#ifdef DEBG
	  std::cerr << "error, node not assigned\n";
#endif
	  abort();
	}
	if (node.depth != depth_) {
#ifdef DEBUG
	  std::cerr << "wrong node 1\n";
#endif
	  abort();
	}
	auto depth = depth_ - 1;
	auto [ ordinal, new_prefix_key ] = remove_top(node.prefix_key);
	auto offset = size_t{0};
	h = calc_hash(depth, new_prefix_key, ordinal);
#ifdef DEBUG
	std::cerr << "down(): depth = " << depth_ << ", ordinal = " << ordinal_ << ", offset = " << offset_ << ", prefix = " << node.prefix_key << ", new key = " << new_key << ", new prefix = " << new_prefix_key << "\n";
#endif
	while ( 1 ) {
	  auto & node = table_->read_node(h, offset);
	  if (!node.flags) {
#ifdef DEBUG
	    std::cerr << "down failed\n";
#endif
	    abort();
	  } else if (node.depth == depth && node.ordinal == ordinal && node.prefix_key == new_prefix_key) {
	    break;
	  } else {
	    // collision
	    offset++;
	  }
	}
#ifdef DEBUG
	std::cerr << "down(): depth = " << depth << ", prefix = " << new_prefix_key << ", ordinal = " << ordinal << ", offset = " << offset << "\n";
#endif
	set_indices(depth, new_prefix_key, ordinal, offset);
      }

      Self * table_;
      size_t ordinal_, offset_;
      key_type prefix_key_;
      uint32_t depth_;
      size_t table_size_;
    };
    
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    Table() noexcept { }
    Table(Table && other) noexcept
      : num_entries_(std::exchange(other.num_entries_, 0)),
	num_final_entries_(std::exchange(other.num_final_entries_, 0)),
	num_inserts_(std::exchange(other.num_inserts_, 0)),
	num_insert_collisions_(std::exchange(other.num_insert_collisions_, 0)),
	table_size_(std::exchange(other.table_size_, 0)),
	nodes_(std::exchange(other.nodes_, nullptr)),
	keyvals_(std::exchange(other.keyvals_, nullptr)) { }

    Table & operator=(Table && other) noexcept {
      std::swap(num_entries_, other.num_entries);
      std::swap(num_final_entries_, other.num_final_entries_);
      std::swap(num_inserts_, other.num_inserts_);
      std::swap(num_insert_collisions_, other.num_insert_collisons_);
      std::swap(table_size_, other.table_size_);
      std::swap(nodes_, other.nodes_);
      std::swap(keyvals_, other.keyvals_);
      return *this;
    }
    
    Table(const Table & other) = delete;
    Table& operator=(const Table & other) = delete;

    ~Table() {
      clear();
    }

    void clear() noexcept {
      for (size_t i = 0; i < table_size_; i++) {
	auto & node = nodes_[i];
	if (node.flags) {
	  node.prefix_key.~key_type();
	  keyvals_[i].~value_type();
	}
      }
      std::free(nodes_);
      std::free(keyvals_);
      num_entries_ = num_final_entries_ = num_inserts_ = num_insert_collisions_ = table_size_ = 0;
      nodes_ = nullptr;
      keyvals_ = nullptr;
    }

    iterator find(const key_type & key) noexcept {
      if (!table_size_) return end();
      auto depth = static_cast<uint32_t>(keysize(key));
      auto [ ordinal, prefix_key ] = remove_top(key);
      size_t offset = 0, h = calc_hash(depth, prefix_key, ordinal);

      while ( 1 ) {
	auto & node = read_node(h, offset);
	if (!node.flags) {
	  break; // not found
	} else if (node.depth != depth || node.ordinal != ordinal || node.prefix_key != prefix_key) {
	  // collision
	  offset++;
	} else if (node.flags & flag_is_final) {
	  return iterator(this, depth, prefix_key, ordinal, offset);
	} else {
	  break; // not final / wrong key
	}
      }
      return end();
    }
    
    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, std::pair<iterator,bool>>::type insert_or_assign(const Key& k, Q && obj) {
      auto [ it, hash ] = create_nodes_for_key(k);
      auto & node = read_node(hash, it.get_offset());
      auto & keyval = read_keyval(hash, it.get_offset());
      bool is_new = true;
      if (!(node.flags & flag_is_assigned)) {
	new (static_cast<void*>(&keyval)) value_type(k, std::move(obj));
	node.flags = flag_is_assigned;
      } else if (!(node.flags & flag_is_final)) {
	node.flags |= flag_is_final;
	keyval = value_type(k, std::move(obj)); // node exists, but it is not final: update the data
      } else {
	keyval = value_type(k, std::move(obj)); // node exists, and is final: update the data
	is_new = false;
      }
      node.flags |= flag_is_final;
      return std::make_pair(it, is_new);
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      value_type vt{std::forward<Args>(args)...};
      auto [ it, hash ] = create_nodes_for_key(getFirstConst(vt));
      auto & node = read_node(hash, it.get_offset());
      auto & keyval = read_keyval(hash, it.get_offset());
      bool is_new = true;
      if (!(node.flags & flag_is_assigned)) {
	new (static_cast<void*>(&keyval)) value_type(std::move(vt));
	node.flags = flag_is_assigned;
      } else if (!(node.flags & flag_is_final)) {
	node.flags |= flag_is_final;
	keyval = std::move(vt); // node exists, but it is not final: update data
      } else {
	is_new = false;
      }
      node.flags |= flag_is_final;
      return std::make_pair(it, is_new);
    }

    std::pair<iterator, bool> insert(const value_type& keyval) {
      return emplace(keyval);
    }

    std::pair<iterator, bool> insert(value_type&& keyval) {
      return emplace(std::move(keyval));
    }

    iterator insert(const_iterator hint, const value_type& keyval) {
      (void)hint;
      return emplace(keyval).first;
    }

    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, Q&>::type operator[](const key_type& key) {
      auto it = find(key);
      if (it != end()) {
	return it->second;
      } else {
	auto [ it, is_new ] = insert(std::pair(key, mapped_type()));
	return it->second;
      }
    }

    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, Q&>::type at(const key_type& key) {
      auto it = find(key);
      if (it != end()) {
	return it->second;
      } else {
	throw std::out_of_range("key not found");
      }
    }

    iterator begin() noexcept {
      if (size()) {
	iterator it(this);
	it.fast_forward();
	return it;
      } else {
	return end();
      }
    }
    iterator end() noexcept {
      // iterator is by default and end iterator (the depth is zero)
      return iterator(this);
    }

    size_t size() const noexcept { return num_final_entries_; }
    size_t num_inserts() const noexcept { return num_inserts_; }
    size_t num_insert_collisions() const noexcept { return num_insert_collisions_; }
    
  private:
    std::pair<iterator, size_t> create_nodes_for_key(key_type key) {
      if (!nodes_) {
	init(bucket_count);
      } else if (100 * num_entries_ / table_size_ >= max_load_factor100) { // Check the load factor
	resize(table_size_ * 2);
      }

      num_inserts_++;

      auto n = keysize(key);
      auto depth = static_cast<uint32_t>(n);
      iterator it = end();
      size_t first_hash = 0;
      bool is_final = true;
      
#ifdef DEBUG
      std::cerr << "inserting node " << key << "\n";
#endif
      
      // insert digits from least significant to most significant
      // even if keysize is zero, add at least one digit (for empty strings)
      for ( size_t i = 0; i < (n == 0 ? 1 : n); i++, depth-- ) {
	auto [ ordinal, prefix_key ] = remove_top(key);
	size_t offset = 0, h = calc_hash(depth, prefix_key, ordinal);
	
	while ( 1 ) {
	  auto & node = read_node(h, offset);
	  if (!node.flags) {
	    if (is_final) {
	      num_final_entries_++;
	      new (static_cast<void*>(&(node.prefix_key))) key_type(prefix_key);
	      // don't set flag_is_assigned for final nodes
	    } else {
	      auto & keyval = read_keyval(h, offset);
	      new (static_cast<void*>(&keyval)) value_type(mk_value_from_key(key));
	      new (static_cast<void*>(&(node.prefix_key))) key_type(prefix_key);
	      node.flags = flag_is_assigned | flag_has_children;
	    }
	    node.hash = h;
	    node.depth = static_cast<uint32_t>(depth);
	    node.ordinal = static_cast<uint8_t>(ordinal);
	    num_entries_++;
	  } else if (node.depth != depth || node.ordinal != ordinal || node.prefix_key != prefix_key) {
	    // collision
	    offset++;
	    num_insert_collisions_++;
	    continue;
	  } else if (is_final && !(node.flags & flag_is_final)) {
	    num_final_entries_++;
	    // don't set flag_is_final yet
	  }
	  if (is_final) {
	    it = iterator(this, depth, prefix_key, ordinal, offset);
	    first_hash = h;
	    is_final = false;
	  }
	  break;
	}

	key = std::move(prefix_key);
      }

      return std::pair(it, first_hash);
    }
    // getFirstConst returns the key from value_type for either set or map
    // This version is for sets, where value_type == key_type
    static key_type const& getFirstConst(key_type const& k) noexcept {
      return k;
    }
    // this one is for maps
    template <typename Q = mapped_type>
    static typename std::enable_if<!std::is_void<Q>::value, key_type const&>::type
    getFirstConst(value_type const& vt) noexcept {
      return vt.first;
    }

    // mk_value_from_key creates a value_type from key_type.
    // This version is for sets, where value_type == key_type
    static value_type mk_value_from_key(value_type k) noexcept {
      return k;
    }
    // this one is for maps
    template <typename Q = mapped_type>
    static typename std::enable_if<!std::is_void<Q>::value, value_type>::type
    mk_value_from_key(key_type const& k) noexcept {
      return std::make_pair(k, mapped_type());
    }

    // size must be a power of two
    void init(size_t s) {
      if (nodes_) std::free(nodes_);
      if (keyvals_) std::free(keyvals_);
      nodes_ = reinterpret_cast<Node*>(std::malloc(s * sizeof(Node)));
      keyvals_ = reinterpret_cast<value_type*>(std::malloc(s * sizeof(value_type)));
      table_size_ = s;
      for (size_t i = 0; i < table_size_; i++) nodes_[i].flags = 0;
    }
    void resize(size_t new_size) {
      Self new_table;
      new_table.init(new_size);
      size_t collisions = 0;
      for (size_t i = 0; i < table_size_; i++) {
	Node & node = nodes_[i];
	if (node.flags) {
	  size_t offset = 0;
	  while ( 1 ) {
	    auto & output_node = new_table.read_node(node.hash, offset);
	    if (output_node.flags) {
	      offset++;
	      collisions++;
	    } else {
	      auto & output_keyval = new_table.read_keyval(node.hash, offset);
	      std::swap(node, output_node);
	      std::swap(keyvals_[i], output_keyval);
	      break;
	    }
	  }
	}
      }
      num_insert_collisions_ += collisions;
      std::swap(nodes_, new_table.nodes_);
      std::swap(keyvals_, new_table.keyvals_);
      std::swap(table_size_, new_table.table_size_);
    }

    Node & read_node(size_t h, size_t offset) noexcept {
      return nodes_[(h + offset) & (table_size_ - 1)];
    }

    value_type & read_keyval(size_t h, size_t offset) noexcept {
      return keyvals_[(h + offset) & (table_size_ - 1)];
    }

    // calc_hash() uses Murmur3 to calculate hash for a Node.
    // Murmur3 operations are specialized for both 32 bit and 64 bit size_t
    static inline size_t calc_hash(uint32_t depth, const key_type & prefix_key, size_t ordinal) noexcept {
      auto k1 = murmur3_mix_k1(std::hash<key_type>{}(prefix_key));
      auto h1 = murmur3_mix_h1(std::size_t{0}, k1);

      k1 = murmur3_mix_k1((static_cast<size_t>(depth) << 8) | ordinal);
      h1 = murmur3_mix_h1(h1, k1);

      return murmur3_fmix(h1);
    }

    size_t num_entries_ = 0, num_final_entries_ = 0;
    size_t num_inserts_ = 0, num_insert_collisions_ = 0;
    size_t table_size_ = 0;
    Node* nodes_ = nullptr;
    value_type* keyvals_ = nullptr;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
