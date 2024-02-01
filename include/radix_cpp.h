#ifndef _RADIXCPP_H_
#define _RADIXCPP_H_

#include <iterator>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include <limits>
#include <functional>
#include <iostream>
#include <cstring>

#define RADIXCPP_FLAG_IS_ASSIGNED	1
#define RADIXCPP_FLAG_IS_FINAL		2
#define RADIXCPP_FLAG_HAS_CHILDREN	4

// #define DEBUG

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

  static inline size_t murmur3_hash(size_t x) noexcept {
    x ^= x >> 33U;
    x *= UINT64_C(0xff51afd7ed558ccd);
    x ^= x >> 33U;
    x *= UINT64_C(0xc4ceb9fe1a85ec53);
    x ^= x >> 33U;
    return x;
  }

  template <typename Key, typename T>
  class Table {
  public:
    static constexpr bool is_map = !std::is_void<T>::value;
    static constexpr bool is_set = !is_map;
    static constexpr size_t bucket_count = 256; // bucket count for the ordered portion of the key

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
      
      Iterator(Self * table) noexcept : table_(table) { }
      Iterator(Self * table, uint32_t depth, key_type unordered_key, size_t start, size_t offset) noexcept
	: table_(table),
	  start_(start),
	  offset_(offset),
	  unordered_key_(std::move(unordered_key)),
	  depth_(depth) { }
      
      void set_indices(uint32_t depth, key_type unordered_key, size_t start, size_t offset) noexcept {
	depth_ = depth;
	unordered_key_ = unordered_key;
	start_ = start;
	offset_ = offset;
      }

      reference operator*() const {
	return table_->read_node(depth_, unordered_key_, start_, offset_).data;
      }
      pointer operator->() {
	return &(table_->read_node(depth_, unordered_key_, start_, offset_).data);
      }
      Iterator& operator++() {
	bool is_first = true;
	while ( depth_ > 0 ) {
	  key_type prefix = unordered_key_;
	  uint32_t depth = depth_;
	  size_t start = start_, offset = offset_;
	  auto & node00 = table_->read_node(depth, prefix, start, offset);
	  if (node00.prefix_key != unordered_key_) {
	    abort();
	  }
	  // std::cerr << "++ start, prefix = " << prefix << ", key = " << getFirstConst(tmp.data) << ", depth = " << depth << ", start = " << start << ", range = " << range << "\n";

	  if (is_first && node00.flags & RADIXCPP_FLAG_IS_FINAL && node00.flags & RADIXCPP_FLAG_HAS_CHILDREN) {
	    depth++;
	    prefix = getFirstConst(node00.data);
	    start = offset = 0;
	  } else {
	    start++;
	    offset = 0;
	  }

	  is_first = false;
	  bool found = false;
	  while ( start < bucket_count ) {
	    auto & node = table_->read_node(depth, prefix, start, offset);
	    if (!node.flags) {
	      start++;
	      offset = 0;
	      continue;
	    } else if (node.depth != depth || node.prefix_key != prefix || top(getFirstConst(node.data)) != start) {
	      offset++;
	      continue;
	    }
	    found = true;
	    if (node.flags & RADIXCPP_FLAG_IS_FINAL) {
	      break;
	    } else {
	      depth++;
	      prefix = getFirstConst(node.data);
	      start = offset = 0;
	    }
	  }

	  if (start < bucket_count) {
#ifdef DEBUG
	    std::cerr << "++ depth = " << depth << ", prefix = " << prefix << ", start = " << start << ", offset = " << offset << "\n";
#endif
	    set_indices(depth, prefix, start, offset);
	    return *this;
	  } else if (found) {
	    std::cerr << "could not find next subiterator\n";
	    abort();
	  }
	  // next element in the range was not found
	  down();
	}
	return *this;
      }
      Iterator operator++(int) {
	Iterator<IsConst> tmp = *this;
	++(*this);
	return tmp;
      }
      template <bool O>
      bool operator== (const Iterator<O>& o) const noexcept {
	return depth_ == o.depth_ && start_ == o.start_ && offset_ == o.offset_;
      }
      template <bool O>
      bool operator!= (const Iterator<O>& o) const noexcept {
	return depth_ != o.depth_ || start_ != o.start_ || offset_ != o.offset_;
      }

      void fast_forward() {
	while (1) {
	  key_type prefix_key;
	  if (depth_ == 0) {
	    prefix_key = key_type();
	  } else {
	    auto & prev_node = table_->read_node(depth_, unordered_key_, start_, offset_);
	    prefix_key = getFirstConst(prev_node.data);
	  }
	  size_t start = 0, offset = 0;
	  while (start < bucket_count) {
	    auto & node = table_->read_node(depth_ + 1, prefix_key, start, offset);
	    if (!node.flags) {
	      start++;
	      offset = 0;
	    } else if (depth_ + 1 != node.depth || node.prefix_key != prefix_key) {
	      offset++;
	    } else {
	      break;
	    }
	  }
	  if (start < bucket_count) {
	    auto & node = table_->read_node(depth_ + 1, prefix_key, start, offset);
	    if (node.prefix_key != prefix_key) {
	      abort();
	    }
	    set_indices(depth_ + 1, prefix_key, start, offset);
	    std::cerr << "ff: node key = " << getFirstConst(node.data) << ", depth = " << depth_ << ", prefix_key = " << unordered_key_ << ", start = " << start_ << ", offset = " << offset_ << "\n";
	    if (node.flags & RADIXCPP_FLAG_IS_FINAL) break;
	  } else {
	    std::cerr << "fast forward: could not find next subiterator for prefix " << prefix_key << "\n";
	    abort();
	  }
	}
      }

    private:
      void down() {
	if (depth_ == 1) {
	  // become an end iterator
	  start_ = offset_ = depth_ = 0;
	  return;
	}
	auto & node = table_->read_node(depth_, unordered_key_, start_, offset_);
	if (!node.flags) {
	  std::cerr << "error, node not assigned\n";
	  abort();
	}
	if (node.depth != depth_) {
	  std::cerr << "wrong node 1\n";
	  abort();
	}
	uint32_t depth = depth_ - 1;
	key_type new_key = node.prefix_key;
	key_type new_prefix_key = prefix(node.prefix_key, keysize(new_key) - 1); // A Hack
	size_t start = top(new_key);
	size_t offset = 0;
#ifdef DEBUG
	std::cerr << "down(): depth = " << depth_ << ", start = " << start_ << ", offset = " << offset_ << ", key = " << getFirstConst(node.data) << ", prefix = " << node.prefix_key << ", new key = " << new_key << ", new prefix = " << new_prefix_key << ", start = " << start << "\n";
#endif
	while ( start < bucket_count ) {
	  auto & node = table_->read_node(depth, new_prefix_key, start, offset);
	  if (!node.flags) {
	    start++;
	    offset = 0;
	  } else if (node.depth != depth || getFirstConst(node.data) != new_key) {
	    offset++;
	  } else {
	    break;
	  }
	}
	if (start == bucket_count) {
	  std::cerr << "down failed\n";
	  abort();
	}
	auto & node2 = table_->read_node(depth, new_prefix_key, start, offset);
	if (getFirstConst(node2.data) != node.prefix_key) {
	  std::cerr << "wrong node 2\n";
	  abort();
	}
#ifdef DEBUG
	std::cerr << "down(): depth = " << depth << ", prefix = " << new_prefix_key << ", key = " << getFirstConst(node2.data) << ", start = " << start << ", offset = " << offset << "\n";
#endif
	set_indices(depth, new_prefix_key, start, offset);
      }

      Self * table_;
      size_t start_ = 0, offset_ = 0;
      key_type unordered_key_;
      uint32_t depth_ = 0;
    };
    
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    Table() { }

    ~Table() {
      for (size_t i = 0; i < data_size_; i++) {
	auto & node = data_[i];
	if (node.flags) {
	  node.data.~value_type();
	  node.prefix_key.~key_type();
	}
      }
      std::free(data_);
    }

    void clear() {
      std::free(data_);
      data_ = NULL;
    }

    iterator find(const key_type & key) {
      uint32_t depth = static_cast<uint32_t>(keysize(key));
      key_type prefix_key = prefix(key, depth - 1);
      size_t start = top(key), offset = 0;
      while ( 1 ) {
	auto & node = read_node(depth, prefix_key, start, offset);
	if (!node.flags) {
	  break; // not found
	} else if (node.depth != depth || node.prefix_key != prefix_key || top(getFirstConst(node.data)) != start) {
	  // collision
	  offset++;
	} else if (node.flags & RADIXCPP_FLAG_IS_FINAL && getFirstConst(node.data) == key) {
	  return iterator(this, depth, prefix_key, start, offset);
	} else {
	  break; // not final / wrong key
	}
      }
      return end();
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      // Check the load factor
      if (!data_) {
	init(bucket_count);
      } else if (10 * num_entries_ / data_size_ >= 5) {
	resize(data_size_ * 2);
      }

      key_type key0 = getFirstConst(vt);
      size_t collisions = 0;

#ifdef DEBUG
      std::cerr << "inserting " << key0 << "\n";
#endif
      
      for (size_t i = 0, n = keysize(key0); i < n; i++) {
	uint32_t depth = static_cast<uint32_t>(i + 1);
	bool is_final = i + 1 == n;
	key_type prefix_key = prefix(key0, i);
	key_type key = prefix(key0, i + 1);
	size_t start = top(key);
	size_t offset = 0;

	while ( 1 ) {
	  auto & node = read_node(depth, prefix_key, start, offset);
	  if (node.flags) {
	    if (node.depth == depth && node.prefix_key == prefix_key) {
	      if (getFirstConst(node.data) != key) {
		collisions++;
		offset++;
		continue;
	      } else {
		// already inserted
	      }
	    } else {
	      // collision
	      offset++;
	      collisions++;
	      continue;
	    }
	  }
#ifdef DEBUG
	  std::cerr << "  key = " << key << ", start = " << start << ", offset = " << offset << ", prefix = " << prefix_key << "\n";
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
	    num_entries_++;
	  } else if (is_final) {
	    is_new = false;
	    node.data = vt; // update data
	  }
	  node.flags |= is_final ? RADIXCPP_FLAG_IS_FINAL : RADIXCPP_FLAG_HAS_CHILDREN;
	  if (is_final) {
	    if (is_new) num_final_entries_++;
	    return std::make_pair(iterator(this, depth, prefix_key, start, offset), is_new);
	  } else {
	    break;
	  }
	}
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

    iterator begin() {
      iterator it(this);
      it.fast_forward();
      return it;
    }
    iterator end() noexcept {
      // iterator is by default and end iterator (the depth is zero)
      return iterator(this);
    }

    size_t size() const noexcept { return num_final_entries_; }
    
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
    
    static value_type mk_value_from_key(value_type k) noexcept {
      return k;
    }
    // this one is for maps
    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, value_type>::type
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
      for (size_t i = 0; i < data_size_; i++) {
	Node & node = data_[i];
	if (node.flags & RADIXCPP_FLAG_IS_ASSIGNED) {
	  size_t start = top(getFirstConst(node.data));
	  size_t offset = 0;
	  size_t h = calc_hash(node.depth, node.prefix_key, start);
	  while ( 1 ) {
	    Node & output_node = new_table.data_[(h + offset) & (new_size - 1)];
	    if (output_node.flags) {
	      offset++;
	    } else {
	      std::swap(node, output_node);
	      break;
	    }
	  }
	}
      }
      std::swap(data_, new_table.data_);
      std::swap(data_size_, new_table.data_size_);
    }

    Node & read_node(uint32_t depth, const key_type & unordered_key, size_t ordered_key, size_t offset) {
      return data_[(calc_hash(depth, unordered_key, ordered_key) + offset) & (data_size_ - 1)];
    }

    static inline size_t hash_combine(size_t seed, size_t hash) noexcept {
      // see https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
      seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }

    // hash function XORs the hash of the key size to the final hash,
    // so that all the prefixes of 0 get a different hash
    static size_t calc_hash(uint32_t depth, const key_type & unordered_key, size_t ordered_key) noexcept {
      return hash_combine(hash_combine(murmur3_hash(std::hash<key_type>{}(unordered_key)),
				       murmur3_hash(depth)),
			  murmur3_hash(ordered_key));
    }

    size_t num_entries_ = 0, num_final_entries_ = 0;
    size_t data_size_ = 0;
    Node* data_ = NULL;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
