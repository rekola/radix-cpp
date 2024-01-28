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
      size_t prefix_key = 0;
    };

    struct SubTable {
      SubTable(size_t unordered_size) : unordered_size_(unordered_size) {
	data_.resize(257);
	data_.back().is_assigned = data_.back().is_sentinel = true;
      }

      struct Node & operator[](size_t i) {
	return data_[i];
      }

      size_t size() const { return data_.size(); }
      void resize(size_t new_size) { data_.resize(new_size); }

      Node & back() { return data_.back(); }

      size_t unordered_size_ = 0;
      size_t num_entries_ = 0;
      std::vector<struct Node> data_;
    };

  public:

    struct Iterator
    {
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = typename Self::value_type;
      using pointer           = value_type*;
      using reference         = value_type&;

      Iterator() { }

      void addNode(SubTable * subtable, size_t idx) {
	subtables_.push_back(subtable);
	indices_.push_back(idx);
      }

      reference operator*() const {
	auto & subtable = subtables_.back();
	size_t idx = indices_.back();
	return (*subtable)[idx].data;
      }
      pointer operator->() {
	auto & subtable = subtables_.back();
	size_t idx = indices_.back();
	return &((*subtable)[idx].data);
      }
      Iterator& operator++() {
	while ( !indices_.empty() ) {
	  auto & subtable = subtables_.back();
	  size_t idx = indices_.back();
	  size_t prefix = (*subtable)[idx].prefix_key;
	  while ( 1 ) {
	    indices_.back()++;
	    auto & node = (*subtable)[indices_.back()];
	    if (node.is_assigned) { // remove this
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
	SubTable * table = subtables_.back();
	while ( 1 ) {
	  size_t idx = indices_.back();
	  auto node = (*table)[idx];
	  if (node.is_assigned) break;
	  indices_.back()++;
	}
      }

    private:
      size_t size() const { return indices_.size(); }

      std::vector<SubTable *> subtables_;
      std::vector<size_t> indices_;
    };
    
    typedef Iterator iterator;
    
    Table() {
      clear();
    }

    void clear() {
      data_.clear();
      for (size_t i = 0; i < key_size; i++) {
	data_.emplace_back(i);
      }
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      size_t key0 = getFirstConst(vt);
      Iterator it;
      bool is_new = true;
      for (size_t i = 0; i < key_size; i++) {
	size_t key = (key0 >> ((key_size - 1 - i) * 8)) & 0xff;
	size_t prefix_key = 0;
	if (i > 0) {
	  prefix_key = key0 >> ((key_size - i) * 8);
	  key = (key + std::hash<uint64_t>{}(prefix_key)) % data_[i].size();
	}
	
	it.addNode(&(data_[i]), key);
	auto & node = data_[i][key];
	if (!node.is_assigned) {
	  is_new = false;
	  node.data = vt;
	  node.is_assigned = true;
	  node.prefix_key = prefix_key;
	}
      }
      return std::make_pair(std::move(it), is_new);
    }

    Iterator begin() {
      Iterator it;
      for (size_t i = 0; i < data_.size(); i++) {
	it.addNode(&(data_[i]), 0);
      }
      it.fast_forward();
      return it;
    }
    Iterator end() {
      Iterator it;
      for (size_t i = 0; i < data_.size(); i++) {
	it.addNode(&(data_[i]), data_[i].size() - 1);
      }
      return it;
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

    std::vector<SubTable> data_;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
