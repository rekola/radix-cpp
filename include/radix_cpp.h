#ifndef _RADIXCPP_H_
#define _RADIXCPP_H_

#include <iterator>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include <limits>
#include <functional>

namespace radix_cpp {
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
      bool is_assigned = false, is_sentinel = false;
      value_type data;
      size_t key = 0, prefix_key = 0;
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

      void addNode(size_t idx) {
	indices_.push_back(idx);
	remaining_.push_back(bucket_count);
      }

      reference operator*() const {
	size_t idx = indices_.back();
	return table_->data_[idx].data;
      }
      pointer operator->() {
	size_t idx = indices_.back();
	return &(table_->data_[idx].data);
      }
      Iterator& operator++() {
	while ( !indices_.empty() ) {
	  size_t idx = indices_.back();
	  size_t prefix = table_->data_[idx].prefix_key;
	  while ( 1 ) {
	    indices_.back()++;
	    auto & node = table_->data_[indices_.back()];
	    if (node.is_sentinel) { // remove this
	      indices_.clear();
	      return *this;
	    } else if (node.is_assigned) { // remove this
	      return *this;
	    } else if (!node.is_assigned) { // and this
	      continue;
	    } else if (!node.is_assigned || node.is_sentinel || node.prefix_key != prefix) {
	      break;
	    } else if (node.prefix_key == prefix) {
	      // fill_nodes();
	      return *this;
	    }
	  }
	  indices_.pop_back();
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
	for (size_t i = 0; i < n; i++) if (a.indices_[i] != b.indices_[i]) return false;
	return true;
      };
      friend bool operator!= (const Iterator& a, const Iterator& b) {
	size_t n = a.size();
	if (n != b.size()) return true;
	for (size_t i = 0; i < n; i++) if (a.indices_[i] != b.indices_[i]) return true;
	return false;
      };

      void fast_forward() {
	while ( 1 ) {
	  size_t idx = indices_.back();
	  auto node = table_->data_[idx];
	  if (node.is_assigned) break;
	  indices_.back()++;
	}
      }

    private:
      size_t size() const { return indices_.size(); }

      Self * table_;
      std::vector<size_t> indices_;
      std::vector<size_t> remaining_;
    };
    
    typedef Iterator iterator;
    
    Table() {
      clear();
    }

    void clear() {
      data_.clear();
      data_.resize(257);
      data_.back().is_assigned = data_.back().is_sentinel = true;
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      size_t key0 = getFirstConst(vt);
      Iterator it(this);
      bool is_new = true;
      for (size_t i = 0; i < key_size; i++) {
	bool is_final = i + 1 == data_.size();
	size_t key = (key0 >> ((key_size - 1 - i) * 8)) & 0xff;
	size_t prefix_key = 0;
	if (i > 0) {
	  prefix_key = key0 >> ((key_size - i) * 8);
	  key = (key + std::hash<uint64_t>{}(prefix_key)) % data_.size();
	}
	
	it.addNode(key);
	auto & node = data_[key];
	if (node.is_assigned && node.prefix_key == prefix_key && node.key == key) {
	  if (is_final) {
	    is_new = false;
	  }
	}
	if (!node.is_assigned) {
	  node.data = vt;
	  node.is_assigned = true;
	  node.prefix_key = prefix_key;
	  num_entries_++;
	}
      }
      return std::make_pair(std::move(it), is_new);
    }

    Iterator begin() {
      Iterator it(this);
      for (size_t i = 0; i < key_size; i++) {
	it.addNode(0);
      }
      it.fast_forward();
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

    size_t num_entries_ = 0;
    std::vector<struct Node> data_;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
