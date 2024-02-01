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
    static constexpr size_t bucket_count = 256;

    using key_type = Key;
    using mapped_type = T;
    using value_type = typename std::conditional<is_set, Key, std::pair<Key, T>>::type;
    using size_type = size_t;
    using Self = Table<key_type, mapped_type>;

  private:
    struct Node {
      bool is_assigned = false, is_final = false, has_children = false;
      value_type data;
      key_type prefix_key;
      size_t depth = 0;
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
      
      void set_indices(size_t depth, key_type unordered_key, size_t start, size_t offset) noexcept {
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
	  size_t depth = depth_, start = start_, offset = offset_;
	  auto & node00 = table_->read_node(depth, prefix, start, offset);
	  if (node00.prefix_key != unordered_key_) {
	    abort();
	  }
	  // std::cerr << "++ start, prefix = " << prefix << ", key = " << getFirstConst(tmp.data) << ", depth = " << depth << ", start = " << start << ", range = " << range << "\n";

	  if (is_first && node00.is_final && node00.has_children) {
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
	    if (!node.is_assigned) {
	      start++;
	      offset = 0;
	      continue;
	    } else if (node.depth != depth || node.prefix_key != prefix || top(getFirstConst(node.data)) != start) {
	      offset++;
	      continue;
	    }
	    found = true;
	    if (node.is_final) {
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
	    if (!node.is_assigned) {
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
	    if (node.is_final) break;
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
	  depth_ = start_ = offset_ = 0;
	  return;
	}
	auto & node = table_->read_node(depth_, unordered_key_, start_, offset_);
	if (!node.is_assigned) {
	  std::cerr << "error, node not assigned\n";
	  abort();
	}
	if (node.depth != depth_) {
	  std::cerr << "wrong node 1\n";
	  abort();
	}
	size_t depth = depth_ - 1;
	key_type new_key = node.prefix_key;
	key_type new_prefix_key = prefix(node.prefix_key, keysize(new_key) - 1); // A Hack
	size_t start = top(new_key);
	size_t offset = 0;
#ifdef DEBUG
	std::cerr << "down(): depth = " << depth_ << ", start = " << start_ << ", offset = " << offset_ << ", key = " << getFirstConst(node.data) << ", prefix = " << node.prefix_key << ", new key = " << new_key << ", new prefix = " << new_prefix_key << ", start = " << start << "\n";
#endif
	while ( start < bucket_count ) {
	  auto & node = table_->read_node(depth, new_prefix_key, start, offset);
	  if (!node.is_assigned) {
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

      key_type unordered_key_;
      size_t depth_ = 0, start_ = 0, offset_ = 0;
      Self * table_;
    };
    
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;

    Table(size_t initial_size = 512) { // an arbitrary initial size
      data_.resize(initial_size);
    }

    void clear() {
      data_.clear();
      data_.resize(512); // an arbitrary initial size
    }

    iterator find(const key_type & key) {
      size_t n = keysize(key);
      size_t prefix_size = n - 1;
      key_type prefix_key = prefix(key, prefix_size);
      size_t start = top(key);
      size_t offset = 0;
      iterator it(this);
      while ( 1 ) {
	auto & node = read_node(n, prefix_key, start, offset);
	if (!node.is_assigned) {
	  break; // not found
	} else if (node.depth != n || node.prefix_key != prefix_key || top(getFirstConst(node.data)) != start) {
	  // collision
	  offset++;
	} else if (node.is_final && getFirstConst(node.data) == key) {
	  it.set_indices(n, prefix_key, start, offset);
	  break;
	} else {
	  break; // not final / wrong key
	}
      }
      return it;
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      // Check the load factor
      if (10 * num_entries_ / data_.size() >= 5) {
	resize(data_.size() * 2);
      }

      key_type key0 = getFirstConst(vt);
      iterator it(this);
      bool is_new = true;
      size_t collisions = 0;

#ifdef DEBUG
      std::cerr << "inserting " << key0 << "\n";
#endif
      
      for (size_t i = 0, n = keysize(key0); i < n; i++) {
	bool is_final = i + 1 == n;
	key_type prefix_key = prefix(key0, i);
	key_type key = prefix(key0, i + 1);
	size_t start = top(key);
	size_t offset = 0;
	value_type data = vt;
	
#ifdef DEBUG
	key_type orig_key = key;
#endif
	while ( 1 ) {
	  auto & node = read_node(i + 1, prefix_key, start, offset);
	  if (node.is_assigned) {
	    if (node.depth == i + 1 && node.prefix_key == prefix_key) {
	      if (getFirstConst(node.data) != key) {
		collisions++;
		offset++;
		continue;
	      } else if (is_final) {
		is_new = false; // already inserted
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
	  if (is_final) {
	    it.set_indices(i + 1, prefix_key, start, offset);
	  }
	  if (!node.is_assigned) {
	    num_entries_++;
	  } else if (is_final) {
	    is_new = false;
	  }
	  node.is_assigned = true;
	  if (is_final) {
	    node.is_final = true;
	    node.data = data;
	  } else {
	    node.has_children = true;
	    if (!node.is_final) {
	      node.data = mk_value_from_key(key);
	    }
	  }
	  node.prefix_key = prefix_key;
	  node.depth = i + 1;
	  break;
	}
      }
#if 0
      std::cerr << "insert collisions = " << collisions << ", keysize = " << keysize(key0) << ", table = " << num_entries_ << "/" << data_.size() << "\n";
#endif
      if (is_new) num_final_entries_++;
      return std::make_pair(std::move(it), is_new);
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

    void resize(size_t new_size) {
      // std::cerr << "resizing table to " << new_size << "\n";
      Self new_table(new_size);
      for (auto & node : data_) {
	if (node.is_assigned && node.is_final) {
	  new_table.insert(node.data);
	}
      }
      swap(data_, new_table.data_);
    }

    Node & read_node(size_t depth, const key_type & unordered_key, size_t ordered_key, size_t offset) {
      return data_[(calc_hash(depth, unordered_key, ordered_key) + offset) % data_.size()];
    }

    static inline size_t hash_combine(size_t seed, size_t hash) noexcept {
      // see https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
      seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }

    // hash function XORs the hash of the key size to the final hash,
    // so that all the prefixes of 0 get a different hash
    static size_t calc_hash(size_t depth, const key_type & unordered_key, size_t ordered_key) noexcept {
      return hash_combine(hash_combine(murmur3_hash(std::hash<key_type>{}(unordered_key)),
				       murmur3_hash(depth)),
			  murmur3_hash(std::hash<uint64_t>{}(ordered_key)));
    }

    size_t num_entries_ = 0, num_final_entries_ = 0;
    std::vector<struct Node> data_;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
