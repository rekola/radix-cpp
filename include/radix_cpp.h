#ifndef _RADIXCPP_H_
#define _RADIXCPP_H_

#include <cstdint>
#include <utility>
#include <string>

#define RADIXCPP_FLAG_IS_ASSIGNED	1
#define RADIXCPP_FLAG_IS_FINAL		2
#define RADIXCPP_FLAG_HAS_CHILDREN	4

// #define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

namespace radix_cpp {
  static inline uint8_t prefix(uint8_t key, size_t n_digits) noexcept {
    return n_digits == 0 ? 0 : key;
  }

  static inline uint16_t prefix(uint16_t key, size_t n_digits) noexcept {
    return n_digits == 0 ? 0 : n_digits >= sizeof(key) ? key : (key >> ((sizeof(key) - n_digits) * 8));
  }

  static inline uint32_t prefix(uint32_t key, size_t n_digits) noexcept {
    return n_digits == 0 ? 0 : n_digits >= sizeof(key) ? key : (key >> ((sizeof(key) - n_digits) * 8));
  }

  static inline uint64_t prefix(uint64_t key, size_t n_digits) noexcept {
    return n_digits == 0 ? 0 : n_digits >= sizeof(key) ? key : (key >> ((sizeof(key) - n_digits) * 8));
  }

  static inline std::string prefix(const std::string & key, size_t n_digits) noexcept {
    return n_digits >= key.size() ? key : key.substr(0, n_digits);
  }

  static inline size_t top(uint8_t key) noexcept {
    return key;
  }

  static inline size_t top(uint16_t key) noexcept {
    return key & 0xff;
  }

  static inline size_t top(uint32_t key) noexcept {
    return key & 0xff;
  }

  static inline size_t top(uint64_t key) noexcept {
    return key & 0xff;
  }

  static inline size_t top(const std::string & key) noexcept {
    return key.empty() ? 0 : static_cast<unsigned char>(key.back());
  }

  static inline size_t keysize(uint8_t key) noexcept {
    return sizeof(key);
  }

  static inline size_t keysize(uint16_t key) noexcept {
    return sizeof(key);
  }

  static inline size_t keysize(uint32_t key) noexcept {
    return sizeof(key);
  }

  static inline size_t keysize(uint64_t key) noexcept {
    return sizeof(key);
  }

  static inline size_t keysize(const std::string & key) noexcept {
    return key.size();
  }

  /* MurmurHash3 was written by Austin Appleby, and is placed in the public domain.
     The author(s) hereby disclaim copyright to the MurmurHash3 source code.
  */

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  static inline T rotl(T x, int_fast8_t r) noexcept {
    return (x << r) | (x >> (32 - r));
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  static inline T rotl(T x, int_fast8_t r) noexcept {
    return (x << r) | (x >> (64 - r));
  }

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  static inline T murmur3_mix_k1(T k1) noexcept {
    k1 *= 0xcc9e2d51;
    k1 = rotl(k1, 15);
    k1 *= 0x1b873593;
    return k1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  static inline T murmur3_mix_k1(T k1) noexcept {
    k1 *= UINT64_C(0x87c37b91114253d5);
    k1 = rotl(k1, 31);
    k1 *= UINT64_C(0x4cf5ad432745937f);
    return k1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  static inline T murmur3_mix_h1(T h1, T k1) noexcept {
    h1 ^= k1;
    h1 = rotl(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
    return h1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  static inline T murmur3_mix_h1(T h1, T k1) noexcept {
    h1 ^= k1;
    h1 = rotl(h1, 27);
    h1 = h1 * 5 + 0x52dce729;
    return h1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 4>::type* = nullptr>
  static inline T murmur3_fmix(T h1) noexcept {
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;
    return h1;
  }

  template<typename T, typename std::enable_if<sizeof(T) == 8>::type* = nullptr>
  static inline T murmur3_fmix(T h1) noexcept {
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
    static constexpr size_t max_load_factor = 6;
    
    using key_type = Key;
    using mapped_type = T;
    using value_type = typename std::conditional<is_set, Key, std::pair<Key, T>>::type;
    using size_type = size_t;
    using Self = Table<key_type, mapped_type>;

  private:
    struct Node {
      value_type data;
      key_type prefix_key;
      uint32_t flags, depth;
      size_t hash;
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
	  table_size_(table->data_size_) { }
      
      Iterator(Self * table, uint32_t depth, key_type prefix_key, size_t ordinal, size_t offset) noexcept
	: table_(table),
	  ordinal_(ordinal),
	  offset_(offset),
	  prefix_key_(std::move(prefix_key)),
	  depth_(depth),
      	  table_size_(table->data_size_) { }
      
      void set_indices(uint32_t depth, key_type prefix_key, size_t ordinal, size_t offset) noexcept {
	depth_ = depth;
	prefix_key_ = std::move(prefix_key);
	ordinal_ = ordinal;
	offset_ = offset;
      }

      reference operator*() const noexcept {
	size_t h = calc_hash(depth_, prefix_key_, ordinal_);
	return table_->read_node(h, offset_).data;
      }
      pointer operator->() noexcept {
	size_t h = calc_hash(depth_, prefix_key_, ordinal_);
	return &(table_->read_node(h, offset_).data);
      }
      Iterator& operator++() noexcept {
	if (table_size_ != table_->data_size_) {
	  // table size has changed => repair the iterator
	  
	  table_size_ = table_->data_size_;
	  offset_ = 0;
	  size_t h = calc_hash(depth_, prefix_key_, ordinal_);

	  while ( 1 ) {
	    auto & node = table_->read_node(h, offset_);
	    if (node.flags & RADIXCPP_FLAG_IS_ASSIGNED &&
		node.depth == depth_ && node.prefix_key == prefix_key_ &&
		top(getFirstConst(node.data)) == ordinal_) {
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
	    if (node.flags & RADIXCPP_FLAG_IS_FINAL && node.flags & RADIXCPP_FLAG_HAS_CHILDREN) {
	      depth++;
	      prefix = getFirstConst(node.data);
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
	    } else if (node.depth != depth || node.prefix_key != prefix || top(getFirstConst(node.data)) != ordinal) {
	      offset++;
	      continue;
	    }
	    found = true;
	    if (node.flags & RADIXCPP_FLAG_IS_FINAL) {
	      break;
	    } else {
	      depth++;
	      prefix = getFirstConst(node.data);
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
	    } else if (depth_ + 1 == node.depth && node.prefix_key == prefix_key && top(getFirstConst(node.data)) ==ordinal) {
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
	    std::cerr << "ff: node key = " << getFirstConst(node.data) << ", depth = " << depth_ << ", prefix_key = " << prefix_key_ << ", ordinal = " << ordinal_ << ", offset = " << offset_ << "\n";
#endif
	    if (node.flags & RADIXCPP_FLAG_IS_FINAL) break;
	    prefix_key = getFirstConst(node.data);
	  } else {
#ifdef DEBUG
	    std::cerr << "fast forward: could not find next subiterator for prefix " << prefix_key << "\n";
#endif
	    abort();
	  }
	}
      }

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
	uint32_t depth = depth_ - 1;
	key_type new_key = node.prefix_key;
	key_type new_prefix_key = prefix(node.prefix_key, keysize(new_key) - 1); // A Hack
	size_t ordinal = top(new_key);
	size_t offset = 0;
	h = calc_hash(depth, new_prefix_key, ordinal);
#ifdef DEBUG
	std::cerr << "down(): depth = " << depth_ << ", ordinal = " << ordinal_ << ", offset = " << offset_ << ", key = " << getFirstConst(node.data) << ", prefix = " << node.prefix_key << ", new key = " << new_key << ", new prefix = " << new_prefix_key << ", ordinal = " << ordinal << "\n";
#endif
	while ( ordinal < bucket_count ) {
	  auto & node = table_->read_node(h, offset);
	  if (!node.flags) {
	    ordinal++;
	    offset = 0;
	    h = calc_hash(depth, new_prefix_key, ordinal);
	  } else if (node.depth != depth || getFirstConst(node.data) != new_key) {
	    offset++;
	  } else {
	    break;
	  }
	}
	if (ordinal == bucket_count) {
#ifdef DEBUG
	  std::cerr << "down failed\n";
#endif
	  abort();
	}
	auto & node2 = table_->read_node(h, offset);
	if (getFirstConst(node2.data) != node.prefix_key) {
#ifdef DEBUG
	  std::cerr << "wrong node 2\n";
#endif
	  abort();
	}
#ifdef DEBUG
	std::cerr << "down(): depth = " << depth << ", prefix = " << new_prefix_key << ", key = " << getFirstConst(node2.data) << ", ordinal = " << ordinal << ", offset = " << offset << "\n";
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
	data_size_(std::exchange(other.data_size_, 0)),
	data_(std::exchange(other.data, NULL)) { }

    Table & operator=(Table && other) noexcept {
      std::swap(num_entries_, other.num_entries);
      std::swap(num_final_entries_, other.num_final_entries_);
      std::swap(num_inserts_, other.num_inserts_);
      std::swap(num_insert_collisions_, other.num_insert_collisons_);
      std::swap(data_size_, other.data_size_);
      std::swap(data_, other.data_);
      return *this;
    }
    
    Table(const Table & other) = delete;
    Table& operator=(const Table & other) = delete;

    ~Table() {
      clear();
    }

    void clear() noexcept {
      for (size_t i = 0; i < data_size_; i++) {
	auto & node = data_[i];
	if (node.flags) {
	  node.data.~value_type();
	  node.prefix_key.~key_type();
	}
      }
      std::free(data_);
      num_entries_ = num_final_entries_ = num_inserts_ = num_insert_collisions_ = data_size_ = 0;
      data_ = NULL;
    }

    iterator find(const key_type & key) noexcept {
      uint32_t depth = static_cast<uint32_t>(keysize(key));
      auto prefix_key = key_type();
      size_t ordinal = 0;
      if (depth) {
	prefix_key = prefix(key, depth - 1);
	ordinal = top(key);
      }
      size_t offset = 0, h = calc_hash(depth, prefix_key, ordinal);

      while ( 1 ) {
	auto & node = read_node(h, offset);
	if (!node.flags) {
	  break; // not found
	} else if (node.depth != depth || node.prefix_key != prefix_key || top(getFirstConst(node.data)) != ordinal) {
	  // collision
	  offset++;
	} else if (node.flags & RADIXCPP_FLAG_IS_FINAL && getFirstConst(node.data) == key) {
	  return iterator(this, depth, prefix_key, ordinal, offset);
	} else {
	  break; // not final / wrong key
	}
      }
      return end();
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      if (!data_) {
	init(bucket_count);
      } else if (10 * num_entries_ / data_size_ >= max_load_factor) { // Check the load factor
	resize(data_size_ * 2);
      }

      auto & key0 = getFirstConst(vt);
      auto prefix_key = key_type();
      
      num_inserts_++;

      // for empty key, start from depth of 0
      uint32_t n = static_cast<uint32_t>(keysize(key0));
      uint32_t depth = n == 0 ? 0 : 1;

#ifdef DEBUG
	std::cerr << "inserting node " << key0 << "\n";
#endif
      
      for ( ; depth <= n; depth++) {
	bool is_final = depth == n;
	auto key = key_type();
	size_t ordinal = 0;
	if (depth) {
	  key = prefix(key0, depth);
	  ordinal = top(key);
	}
	size_t offset = 0, h = calc_hash(depth, prefix_key, ordinal);

	while ( 1 ) {
	  auto & node = read_node(h, offset);
	  if (node.flags) {
	    if (node.depth == depth && node.prefix_key == prefix_key) {
	      if (getFirstConst(node.data) != key) {
		num_insert_collisions_++;
		offset++;
		continue;
	      } else {
		// already inserted
	      }
	    } else {
	      // collision
	      offset++;
	      num_insert_collisions_++;
	      continue;
	    }
	  }
#ifdef DEBUG
	  std::cerr << "  h = " << h << ", key = " << key << ", ordinal = " << ordinal << ", offset = " << offset << ", prefix = " << prefix_key << "\n";
#endif
	  bool is_new = true;
	  if (!node.flags) {
	    if (is_final) {
	      new (static_cast<void*>(&(node.data))) value_type(vt);
	    } else {
	      new (static_cast<void*>(&(node.data))) value_type(mk_value_from_key(key));
	    }
	    new (static_cast<void*>(&(node.prefix_key))) key_type(std::move(prefix_key));
	    node.flags = RADIXCPP_FLAG_IS_ASSIGNED;
	    node.depth = depth;
	    node.hash = h;
	    num_entries_++;
	  } else if (is_final) {
	    is_new = false;
	    node.data = vt; // update data
	  }
	  node.flags |= is_final ? RADIXCPP_FLAG_IS_FINAL : RADIXCPP_FLAG_HAS_CHILDREN;
	  if (is_final) {
	    if (is_new) num_final_entries_++;
	    return std::make_pair(iterator(this, depth, prefix_key, ordinal, offset), is_new);
	  } else {
	    break;
	  }
	}

	prefix_key = std::move(key);
      }

      // error
      return std::make_pair(end(), false);
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
      if (data_) std::free(data_);
      data_ = reinterpret_cast<Node*>(std::malloc(s * sizeof(Node)));
      data_size_ = s;
      for (size_t i = 0; i < data_size_; i++) data_[i].flags = 0;
    }
    void resize(size_t new_size) {
      Self new_table;
      new_table.init(new_size);
      size_t collisions = 0;
      for (size_t i = 0; i < data_size_; i++) {
	Node & node = data_[i];
	if (node.flags & RADIXCPP_FLAG_IS_ASSIGNED) {
	  size_t offset = 0;
	  while ( 1 ) {
	    Node & output_node = new_table.read_node(node.hash, offset);
	    if (output_node.flags) {
	      offset++;
	      collisions++;
	    } else {
	      std::swap(node, output_node);
	      break;
	    }
	  }
	}
      }
      num_insert_collisions_ += collisions;
      std::swap(data_, new_table.data_);
      std::swap(data_size_, new_table.data_size_);
    }

    inline Node & read_node(size_t h, size_t offset) noexcept {
      return data_[(h + offset) & (data_size_ - 1)];
    }

    // calc_hash() uses Murmur3 to calculate hash for a Node.
    // Murmur3 operations are specialized for both 32 bit and 64 bit size_t
    static inline size_t calc_hash(uint32_t depth, const key_type & prefix_key, size_t ordinal) noexcept {
      auto k1 = murmur3_mix_k1(std::hash<key_type>{}(prefix_key));
      auto h1 = murmur3_mix_h1(std::size_t{0}, k1);

      // FIXME: on 32 bit system, the ordered key is lost
      k1 = murmur3_mix_k1((ordinal << 32) | depth);
      h1 = murmur3_mix_h1(h1, k1);

      return murmur3_fmix(h1);
    }

    size_t num_entries_ = 0, num_final_entries_ = 0;
    size_t num_inserts_ = 0, num_insert_collisions_ = 0;
    size_t data_size_ = 0;
    Node* data_ = NULL;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
