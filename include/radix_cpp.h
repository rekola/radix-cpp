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
  static inline uint8_t prefix(uint8_t key, size_t n_digits) {
    return n_digits == 0 ? 0 : key;
  }

  static inline uint16_t prefix(uint16_t key, size_t n_digits) {
    return n_digits == 0 ? 0 : n_digits >= sizeof(key) ? key : (key >> ((sizeof(key) - n_digits) * 8));
  }

  static inline uint32_t prefix(uint32_t key, size_t n_digits) {
    return n_digits == 0 ? 0 : n_digits >= sizeof(key) ? key : (key >> ((sizeof(key) - n_digits) * 8));
  }

  static inline uint64_t prefix(uint64_t key, size_t n_digits) {
    return n_digits == 0 ? 0 : n_digits >= sizeof(key) ? key : (key >> ((sizeof(key) - n_digits) * 8));
  }

  static inline std::string prefix(std::string key, size_t n_digits) {
    return n_digits >= key.size() ? key : key.substr(0, n_digits);
  }

  static inline size_t top(uint8_t key) {
    return key;
  }

  static inline size_t top(uint16_t key) {
    return key & 0xff;
  }

  static inline size_t top(uint32_t key) {
    return key & 0xff;
  }

  static inline size_t top(uint64_t key) {
    return key & 0xff;
  }

  static inline size_t top(std::string key) {
    return key.empty() ? 0 : static_cast<unsigned char>(key.back());
  }

  template <typename Key, typename T>
  class Table {
  public:
    static constexpr bool is_map = !std::is_void<T>::value;
    static constexpr bool is_set = !is_map;
    static constexpr size_t key_size = sizeof(Key);
    static constexpr size_t bucket_count = 256;

    using key_type = Key;
    using mapped_type = T;
    using value_type = typename std::conditional<is_set, Key, std::pair<Key, T>>::type;
    using size_type = size_t;
    using Self = Table<key_type, mapped_type>;

  private:
    struct Node {
      bool is_assigned = false;
      value_type data;
      key_type key = 0, prefix_key = 0;
      size_t prefix_size = 0;
    };

  public:

    struct Iterator
    {
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = typename Self::value_type;
      using pointer           = value_type*;
      using reference         = value_type&;

      Iterator(Self * table) : table_(table) { }

      void pushSubIterator(size_t start, size_t end) {
	indices_start_.push_back(start);
	indices_end_.push_back(end);
      }

      void popSubIterator() {
	indices_start_.pop_back();
	indices_end_.pop_back();
      }

      reference operator*() const {
	size_t idx = indices_start_.back();
	return table_->data_[idx].data;
      }
      pointer operator->() {
	size_t idx = indices_start_.back();
	return &(table_->data_[idx].data);
      }
      Iterator& operator++() {
	while ( !indices_start_.empty() ) {
	  auto & node00 = table_->data_[indices_start_.back() % table_->data_.size()];
	  size_t prefix_size = node00.prefix_size;
	  key_type prefix = node00.prefix_key;
	  while ( indices_start_.back() != indices_end_.back() ) {
	    indices_start_.back()++;
	    if (indices_start_.back() >= table_->data_.size()) indices_start_.back() -= table_->data_.size();
	    
	    auto & node = table_->data_[indices_start_.back() % table_->data_.size()];
	    if (!node.is_assigned) {
	      // unassigned
	    } else if (node.prefix_size == prefix_size && node.prefix_key == prefix) {
	      if (indices_start_.size() < key_size) {
		auto & node0 = table_->data_[indices_start_.back() % table_->data_.size()];
		size_t start = (0 + std::hash<key_type>{}(node0.key)) % table_->data_.size();
		size_t end = (bucket_count + std::hash<key_type>{}(node0.key)) % table_->data_.size();
		while ( start != end ) {
		  auto & node = table_->data_[start];
		  if (!node.is_assigned) {
		    start++;
		    if (start >= table_->data_.size()) start -= table_->data_.size();
		  } else if (prefix_size + 1 != node.prefix_size || node.prefix_key != node0.key) {
		    start++;
		    if (start >= table_->data_.size()) start -= table_->data_.size();
		  } else {
		    break;
		  }
		}
		if (start == end) {
		  std::cerr << "could not find next subiterator\n";
		  abort();
		}
		pushSubIterator(start, end);
	      }

#ifdef DEBUG
	      std::cerr << "++:";
	      for (size_t i = 0; i < indices_start_.size(); i++) {
		size_t idx = indices_start_[i];
		std::cerr << " " << indices_start_[i] << ":" << indices_end_[i] << " (prefix = " << table_->data_[idx].prefix_key << ", key = " << table_->data_[idx].key << ")";
	      }
	      std::cerr << "\n";
#endif
	      return *this;
	    }
	     // collision
	  }
	  popSubIterator();
	}
	return *this;
      }
      Iterator operator++(int) {
	Iterator tmp = *this;
	++(*this);
	return tmp;
      }
      friend bool operator== (const Iterator& a, const Iterator& b) {
	size_t n = a.size();
	if (n != b.size()) return false;
	for (size_t i = 0; i < n; i++) if (a.indices_start_[i] != b.indices_start_[i]) return false;
	return true;
      };
      friend bool operator!= (const Iterator& a, const Iterator& b) {
	size_t n = a.size();
	if (n != b.size()) return true;
	for (size_t i = 0; i < n; i++) if (a.indices_start_[i] != b.indices_start_[i]) return true;
	return false;
      };

      void fast_forward() {
	for (size_t i = 0; i < key_size; i++) {
	  size_t start, end;
	  key_type prefix_key;
	  if (i == 0) {
	    prefix_key = 0;
	    start = 0;
	    end = bucket_count % table_->data_.size();
	  } else {
	    auto & prev_node = table_->data_[indices_start_.back()];
	    prefix_key = prev_node.key;
	    start = (0 + std::hash<key_type>{}(prefix_key)) % table_->data_.size();
	    end = (bucket_count + std::hash<key_type>{}(prefix_key)) % table_->data_.size();
	  }
	  while (start != end) {
	    auto & node = table_->data_[start];
	    if (!node.is_assigned) {
	      start++;
	      if (start >= table_->data_.size()) start -= table_->data_.size();
	    } else if (i != node.prefix_size || node.prefix_key != prefix_key) {
	      start++;
	      if (start >= table_->data_.size()) start -= table_->data_.size();
	    } else {
#ifdef DEBUG
	      std::cerr << "fast forward digit " << i << ": idx " << start << ":" << end << ", node (" << node.prefix_size << ", " << node.prefix_key << ")\n";
#endif
	      break;
	    }
	  }

	  if (start != end) {
	    std::cerr << "fast forward: found iterator " << start << ":" << end << " for prefix " << prefix_key << "\n";
	    indices_start_.push_back(start);
	    indices_end_.push_back(end);
	  } else {
	    std::cerr << "fast forward: could not find next subiterator for prefix " << prefix_key << "\n";
	    abort();
	    break;
	  }
	}
      }

      std::vector<size_t> indices_start_, indices_end_;

    private:
      size_t size() const { return indices_start_.size(); }

      Self * table_;
    };
    
    typedef Iterator iterator;
    
    Table() {
      clear();
    }

    void clear() {
      data_.clear();
      data_.resize(512);
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      // Check the load factor
      if (10 * num_entries_ / data_.size() >= 7) {
	resize(data_.size() * 2);
      }
      
      key_type key0 = getFirstConst(vt);
      Iterator it(this);
      bool is_new = true;
      for (size_t i = 0; i < key_size; i++) {
	bool is_final = i + 1 == key_size;
	key_type prefix_key = prefix(key0, i);
	key_type key = prefix(key0, i + 1);
	size_t start = top(key);
	size_t end = 0;
	if (i > 0) {
	  size_t h = std::hash<key_type>{}(prefix_key);
	  start = (start + h) % data_.size();
	  end = (bucket_count + h) % data_.size();
	}
	while ( 1 ) {
	  auto & node = data_[start];
	  if (node.is_assigned && !(node.prefix_key == prefix_key && node.key == key)) {
	    // collision
	    start++;
	    if (start >= data_.size()) start -= data_.size();
	  }
	  it.pushSubIterator(start, end);
	  if (!node.is_assigned) {
	    num_entries_++;
	  } else if (is_final) {
	    is_new = false;
	  }
	  node.data = vt;
	  node.is_assigned = true;
	  node.key = key;
	  node.prefix_key = prefix_key;
	  node.prefix_size = i;
	  break;
	}
      }
      return std::make_pair(std::move(it), is_new);
    }

    Iterator begin() {
      Iterator it(this);
      it.fast_forward();
#ifdef DEBUG
      std::cerr << "begin:";
      for (size_t i = 0; i < it.indices_start_.size(); i++) {
	std::cerr << " " << it.indices_start_[i];
      }
      std::cerr << "\n";
#endif
      return it;
    }
    Iterator end() {
      return Iterator(this);
    }

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
      std::cerr << "resize not implemented\n";
      abort();
    }

    size_t num_entries_ = 0;
    std::vector<struct Node> data_;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
