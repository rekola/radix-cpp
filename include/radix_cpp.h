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
      using reference	      = typename std::conditional<IsConst, value_type const&, value_type&>::type;
      using pointer = typename std::conditional<IsConst, value_type const*, value_type*>::type;
      
      Iterator(Self * table) : table_(table) { }

      size_t depth() const { return depth_; }
      
      void set_indices(size_t depth, size_t start, size_t range) {
	depth_ = depth;
	start_ = start;
	range_ = range;
      }

      reference operator*() const {
	return table_->data_[start_].data;
      }
      pointer operator->() {
	return &(table_->data_[start_].data);
      }
      Iterator& operator++() {
	while ( depth_ > 0 ) {
	  auto & node00 = table_->data_[start_];
	  size_t prefix_size = node00.depth - 1;
	  key_type prefix = node00.prefix_key;
	  size_t depth = depth_, start = start_, range = range_;
	  while ( range >= 2 ) {
	    start++;
	    if (start == table_->data_.size()) start_ = 0;
	    
	    auto & node = table_->data_[start];
	    if (!node.is_assigned) {
	      range--;
	    } else if (node.depth == prefix_size + 1 && node.prefix_key == prefix) {
	      range--;
	      
	      auto & node0 = table_->data_[start];
	      if (!node0.is_final) {
		depth++;
		start = hash(node0.depth - 1, node0.key, 0) % table_->data_.size();
		range = bucket_count;
		while ( range >= 1 ) {
		  auto & node = table_->data_[start];
		  if (!node.is_assigned) {
		    start++;
		    if (start == table_->data_.size()) start = 0;
		    range--;
		  } else if (prefix_size + 2 != node.depth || node.prefix_key != node0.key) {
		    start++;
		    if (start == table_->data_.size()) start = 0;
		  } else {
		    break;
		  }
		}
		if (!range) {
		  std::cerr << "could not find next subiterator\n";
		  abort();
		}
	      }
	      set_indices(depth, start, range);

#ifdef DEBUG
	      std::cerr << "++: " << start_ << ":" << range_ << " (prefix = " << table_->data_[start_].prefix_key << ", key = " << table_->data_[start_].key << ")\n";
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
	  depth_++;
	  size_t start, range = bucket_count;
	  size_t prefix_size = depth_ - 1;
	  key_type prefix_key;
	  if (prefix_size == 0) {
	    prefix_key = key_type();
	    start = 0;
	  } else {
	    auto & prev_node = table_->data_[start_];
	    prefix_key = prev_node.key;
	    start = hash(prefix_size, prefix_key, 0) % table_->data_.size();
	  }
	  while (range >= 1) {
	    auto & node = table_->data_[start];
	    if (!node.is_assigned) {
	      start++;
	      if (start == table_->data_.size()) start = 0;
	      range--;
	    } else if (depth_ != node.depth || node.prefix_key != prefix_key) {
	      start++;
	      if (start == table_->data_.size()) start = 0;
	    } else {
	      break;
	    }
	  }

	  if (range) {
	    auto & node = table_->data_[start];
	    if (node.prefix_key != prefix_key) {
	      abort();
	    }
#ifdef DEBUG
	    std::cerr << "fast forward: found iterator " << start << ":" << range << " for prefix " << prefix_key << " with key " << node.key << "\n";
#endif
	    set_indices(depth_, start, range);
	    if (node.is_final) break;
	  } else {
	    std::cerr << "fast forward: could not find next subiterator for prefix " << prefix_key << "\n";
	    abort();
	  }
	}
      }

    private:
      void down() {
	if (!--depth_) {
	  // become an end iterator
	  start_ = range_ = 0;
	  return;
	}
	auto & node = table_->data_[start_];
	if (!node.is_assigned) {
	  std::cerr << "error, node not assigned\n";
	  abort();
	}
	size_t prefix_size = depth_ - 1;
	if (node.depth != prefix_size + 2) {
	  std::cerr << "wrong node\n";
	  abort();
	}
	key_type new_key = node.prefix_key;
	key_type new_prefix_key = prefix(new_key, prefix_size); // Is this wrong?
	start_ = top(new_key);
	range_ = bucket_count - start_;
	if (prefix_size) {
	  start_ = hash(prefix_size, new_prefix_key, start_) % table_->data_.size();
	}
	while ( 1 ) {
	  auto & node = table_->data_[start_];
	  if (node.is_assigned && node.depth == depth_ && node.key == new_key) {
	    break;
	  } else {
	    start_++;
	    if (start_ == table_->data_.size()) start_ = 0;
	  }
	}
	auto & node2 = table_->data_[start_];
	if (node2.key != node.prefix_key) {
	  std::cerr << "wrong node\n";
	  abort();
	}
      }

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
      if (prefix_size > 0) {
	start = hash(prefix_size, prefix_key, start) % data_.size();
      }
      iterator it(this);
      while ( 1 ) {
	auto & node = data_[start];
	if (!node.is_assigned) {
	  break; // not found
	} else if (node.depth != n || node.prefix_key != prefix_key) {
	  // collision
	  start++;
	} else if (node.is_final && node.key == key) {
	  it.set_indices(n, start, range);
	  break;
	} else {
	  break; // not final / wrong key
	}
      }
      return it;
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      // Check the load factor
      if (10 * num_entries_ / data_.size() >= 7) {
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
	
	if (i > 0) {
	  start = hash(i, prefix_key, start) % data_.size();
	}
#ifdef DEBUG
	size_t orig_key = key;
#endif
	while ( 1 ) {
	  auto & node = data_[start];
	  if (node.is_assigned) {
	    if (node.depth == i + 1 && node.prefix_key == prefix_key) {
	      if (node.key != key) {
		// collision with a friend
		std::swap(node.data, data);
		std::swap(node.key, key);
		std::swap(node.is_final, is_final);
		it.set_indices(i + 1, start, range);
		is_assigned = true;
		collisions++;
		
		start++;
		if (start >= data_.size()) start -= data_.size();
		range--;
		continue;
	      } else if (is_final) {
		is_new = false; // already inserted
	      }
	    } else {
	      // collision
	      start++;
	      if (start >= data_.size()) start -= data_.size();
	      collisions++;
	      continue;
	    }
	  }
	  if (!is_assigned && is_final) {
#ifdef DEBUG
	    std::cerr << "  start = " << start << ", range = " << range << ", prefix = " << prefix_key << ", key = " << orig_key << "\n";
#endif
	    it.set_indices(i + 1, start, range);
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
#if 0
      std::cerr << "insert collisions = " << collisions << "\n";
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
    iterator end() {
      // iterator is by default and end iterator (the depth is zero)
      return iterator(this);
    }

    size_t size() const { return num_final_entries_; }
    
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
	if (node.is_assigned) {
	  new_table.insert(node.data);
	}
      }
      swap(data_, new_table.data_);
    }

    // hash function XORs the hash of the key size to the final hash,
    // so that all the prefixes of 0 get a different hash
    static size_t hash(size_t key_size, key_type unordered_key, size_t ordered_key) {
      return ordered_key + (std::hash<size_t>{}(key_size) ^ std::hash<key_type>{}(unordered_key));
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
