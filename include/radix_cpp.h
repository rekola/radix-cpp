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
      bool is_assigned = false, is_final = false;
      value_type data;
      key_type key, prefix_key;
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
      
      void set_indices(size_t depth, key_type unordered_key, size_t start, size_t range) noexcept {
	depth_ = depth;
	unordered_key_ = unordered_key;
	start_ = start;
	range_ = range;
      }

      reference operator*() const {
	return table_->read_node(depth_, unordered_key_, start_).data;
      }
      pointer operator->() {
	return &(table_->read_node(depth_, unordered_key_, start_).data);
      }
      Iterator& operator++() {
	while ( depth_ > 0 ) {
	  key_type prefix = unordered_key_;
	  size_t depth = depth_, start = start_, range = range_;
	  auto & node00 = table_->read_node(depth, prefix, start);
	  if (node00.prefix_key != unordered_key_) {
	    abort();
	  }
	  // std::cerr << "++ start, prefix = " << prefix << ", key = " << tmp.key << ", depth = " << depth << ", start = " << start << ", range = " << range << "\n";
	  while ( range >= 2 ) {
	    start++;
	    
	    auto & node = table_->read_node(depth, prefix, start);
	    if (!node.is_assigned) {
	      range--;
	    } else if (node.depth == depth && node.prefix_key == prefix) {
	      range--;

	      // auto & tmp = table_->read_node(depth, prefix, start);
	      // std::cerr << "++ advance, prefix = " << prefix << ", key = " << tmp.key << ", depth = " << depth << ", start = " << start << ", range = " << range << "\n";
	      
	      while ( 1 ) {
		auto & node0 = table_->read_node(depth, prefix, start);
		if (node0.is_final) {
		  break;
		} else {
		  // std::cerr << "up\n";
		  depth++;
		  prefix = node0.key;
		  start = 0;
		  range = bucket_count;
		  while ( range >= 1 ) {
		    auto & node = table_->read_node(depth, prefix, start);
		    if (!node.is_assigned) {
		      start++;
		      range--;
		    } else if (depth != node.depth || node.prefix_key != prefix) {
		      start++;
		    } else {
		      break;
		    }
		  }
		  if (!range) {
		    std::cerr << "could not find next subiterator\n";
		    abort();
		  }
		}
	      }
	      
	      set_indices(depth, prefix, start, range);

#ifdef DEBUG
	      auto & nnn = table_->read_node(depth_, unordered_key_, start_);
	      std::cerr << "++: depth = " << depth_ << ", unordered_key = " << unordered_key_ << ", start = " << start_ << ":" << range_ << " (prefix = " << nnn.prefix_key << ", key = " << nnn.key << ")\n";
#endif
	      return *this;
	    }
	     // collision
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
	return depth_ == o.depth_ && start_ == o.start_ && range_ == o.range_;
      }
      template <bool O>
      bool operator!= (const Iterator<O>& o) const noexcept {
	return depth_ != o.depth_ || start_ != o.start_ || range_ != o.range_;
      }

      void fast_forward() {
	while (1) {
	  key_type prefix_key;
	  if (depth_ == 0) {
	    prefix_key = key_type();
	  } else {
	    auto & prev_node = table_->read_node(depth_, unordered_key_, start_);
	    prefix_key = prev_node.key;
	  }
	  size_t start = 0, range = bucket_count;
	  while (range >= 1) {
	    auto & node = table_->read_node(depth_ + 1, prefix_key, start);
	    if (!node.is_assigned) {
	      start++;
	      range--;
	    } else if (depth_ + 1 != node.depth || node.prefix_key != prefix_key) {
	      start++;
	    } else {
	      break;
	    }
	  }

	  if (range) {
	    auto & node = table_->read_node(depth_ + 1, prefix_key, start);
	    if (node.prefix_key != prefix_key) {
	      abort();
	    }
#ifdef DEBUG
	    std::cerr << "fast forward: found iterator " << start << ":" << range << " for prefix " << prefix_key << " with key " << node.key << "\n";
#endif
	    set_indices(depth_ + 1, prefix_key, start, range);
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
	  depth_ = start_ = range_ = 0;
	  return;
	}
	auto & node = table_->read_node(depth_, unordered_key_, start_);
	if (!node.is_assigned) {
	  std::cerr << "error, node not assigned\n";
	  abort();
	}
	if (node.depth != depth_) {
	  std::cerr << "wrong node 1\n";
	  abort();
	}
	size_t depth = depth_ - 1;
	size_t prefix_size = depth - 1;
	key_type new_key = node.prefix_key;
	key_type new_prefix_key = prefix(new_key, prefix_size); // Is this wrong?
	size_t start = top(new_key);
	size_t range = bucket_count - start;
	while ( 1 ) {
	  auto & node = table_->read_node(depth, new_prefix_key, start);
	  if (node.is_assigned && node.depth == depth && node.key == new_key) {
	    break;
	  } else {
	    start++;
	  }
	}
	auto & node2 = table_->read_node(depth, new_prefix_key, start);
	if (node2.key != node.prefix_key) {
	  std::cerr << "wrong node 2\n";
	  abort();
	}
	// std::cerr << "down(): depth = " << depth << ", prefix = " << new_prefix_key << ", key = " << node2.key << ", start = " << start << ", range = " << range << "\n";
	set_indices(depth, new_prefix_key, start, range);
      }

      key_type unordered_key_;
      size_t depth_ = 0, start_ = 0, range_ = 0;
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
      size_t range = bucket_count - start;
      iterator it(this);
      while ( 1 ) {
	auto & node = read_node(n, prefix_key, start);
	if (!node.is_assigned) {
	  break; // not found
	} else if (node.depth != n || node.prefix_key != prefix_key) {
	  // collision
	  start++;
	} else if (node.is_final && node.key == key) {
	  it.set_indices(n, prefix_key, start, range);
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
	size_t range = bucket_count - start;
	value_type data = vt;
	bool is_assigned = false;
	
#ifdef DEBUG
	key_type orig_key = key;
#endif
	while ( 1 ) {
	  auto & node = read_node(i + 1, prefix_key, start);
	  if (node.is_assigned) {
	    if (node.depth == i + 1 && node.prefix_key == prefix_key) {
	      if (node.key != key) {
		// collision with a friend
		std::swap(node.data, data);
		std::swap(node.key, key);
		std::swap(node.is_final, is_final);
		it.set_indices(i + 1, prefix_key, start, range);
		is_assigned = true;
		collisions++;

		// std::cerr << "# friendly collision at " << calc_hash(i + 1, prefix_key, start) << ", start = " << start << ", sh = " << std::hash<uint64_t>{}((uint64_t)start) << "\n";
		
		start++;
		range--;
		continue;
	      } else if (is_final) {
		is_new = false; // already inserted
	      }
	    } else {
	      // collision
	      // std::cerr << "# unfriendly collision at " << calc_hash(i + 1, prefix_key, start) << ", start = " << start << ", sh = " << std::hash<uint64_t>{}((uint64_t)start) << "\n";
	      start++;
	      collisions++;
	      continue;
	    }
	  }
	  if (!is_assigned && is_final) {
#ifdef DEBUG
	    std::cerr << "  start = " << start << ", range = " << range << ", prefix = " << prefix_key << ", key = " << orig_key << "\n";
#endif
	    it.set_indices(i + 1, prefix_key, start, range);
	  }
	  if (!node.is_assigned) {
	    num_entries_++;
	  } else if (is_final) {
	    is_new = false;
	  }
	  node.data = data;
	  node.is_assigned = true;
	  node.is_final = is_final;
	  node.key = key;
	  node.prefix_key = prefix_key;
	  node.depth = i + 1;
	  break;
	}
      }
#if 1
      // std::cerr << "insert collisions = " << collisions << ", keysize = " << keysize(key0) << ", table = " << num_entries_ << "/" << data_.size() << "\n";
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
    key_type const& getFirstConst(key_type const& k) const noexcept {
      return k;
    }
    // this one is for maps
    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, key_type const&>::type
    getFirstConst(value_type const& vt) const noexcept {
      return vt.first;
    }

    void resize(size_t new_size) {
      std::cerr << "resizing table to " << new_size << "\n";
      Self new_table(new_size);
      for (auto & node : data_) {
	if (node.is_assigned && node.is_final) {
	  new_table.insert(node.data);
	}
      }
      swap(data_, new_table.data_);
    }

    Node & read_node(size_t depth, const key_type & unordered_key, size_t ordered_key) {
      return data_[calc_hash(depth, unordered_key, ordered_key) % data_.size()];
    }

    static inline size_t hash_combine(size_t seed, size_t hash) noexcept {
      // see https://www.boost.org/doc/libs/1_55_0/doc/html/hash/reference.html#boost.hash_combine
      seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
      return seed;
    }

    // hash function XORs the hash of the key size to the final hash,
    // so that all the prefixes of 0 get a different hash
    static size_t calc_hash(size_t depth, const key_type & unordered_key, size_t ordered_key) noexcept {
      return hash_combine(murmur3_hash(std::hash<key_type>{}(unordered_key)),
			  std::hash<uint64_t>{}(ordered_key));
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
