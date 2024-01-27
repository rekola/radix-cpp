#ifndef _RADIXCPP_H_
#define _RADIXCPP_H_

#include <iterator>
#include <cstddef>
#include <utility>
#include <vector>
#include <limits>

namespace radix_cpp {
  template <typename Key, typename T>
  class Table {
  public:
    static constexpr bool is_map = !std::is_void<T>::value;
    static constexpr bool is_set = !is_map;
    static constexpr size_t num_digits = sizeof(T);

    using key_type = Key;
    using mapped_type = T;
#if 1
    using value_type = typename std::conditional<is_set, Key, std::pair<Key, T>>::type;
#else
    using value_type = Key;
#endif
    using size_type = size_t;
    using Self = Table<key_type, mapped_type>;

  private:
    struct Node {
      bool is_assigned = false;
      value_type data;
    };

  public:

    struct Iterator 
    {
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = typename Self::value_type;
      using pointer           = value_type*;
      using reference         = value_type&;

      Iterator(Node * node) : node_(node) {

      }

      reference operator*() const { return node_->data; }
      pointer operator->() { return &(node_->data); }
      Iterator& operator++() {
	do {
	  node_++;
	} while (!node_->is_assigned);
	return *this;
      }
      Iterator operator++(int) {
	Iterator tmp = *this;
	++(*this);
	return tmp;
      }
      friend bool operator== (const Iterator& a, const Iterator& b) {
	return a.node_ == b.node_;
      };
      friend bool operator!= (const Iterator& a, const Iterator& b) {
	return a.node_ != b.node_;
      };

    private:
      Node * node_;
    };
    
    typedef Iterator iterator;
    
    Table() {
      data_.resize((size_t)std::numeric_limits<Key>::max() + 2);
      data_.back().is_assigned = true;
    }

    void clear() {
      data_.clear();
    }

    std::pair<iterator,bool> insert(const value_type& vt) {
      auto & key = getFirstConst(vt);
      auto & node = data_[key];
      if (node.is_assigned) {
	return std::make_pair(Iterator(&node), false);
      } else {
	node.data = vt;
	node.is_assigned = true;
	return std::make_pair(Iterator(&node), false);
      }
    }

    Iterator begin() {
      for (size_t i = 0; i < data_.size(); i++) {
	if (data_[i].is_assigned) return Iterator(&data_[i]);
      }
      return end();
    }
    Iterator end() { return Iterator(&(data_.back())); }

  private:
    // getFirstConst returns the key for either set or map
    // This version is for sets
    key_type const& getFirstConst(key_type const& k) const noexcept {
      return k;
    }
    // this one is for maps
    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, key_type const&>::type
    getFirstConst(value_type const& vt) const noexcept {
      return vt.first;
    }

    std::vector<struct Node> data_;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
