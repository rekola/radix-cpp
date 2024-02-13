#ifndef _RADIXCPP_H_
#define _RADIXCPP_H_

#include <cstdint>
#include <utility>
#include <string>
#include <stdexcept>

#define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

namespace radix_cpp {
  inline constexpr uint32_t flag_is_assigned = 1;
   
  inline uint8_t append(uint8_t key, size_t digit) noexcept {
    return static_cast<uint8_t>(digit);
  }

  inline std::pair<size_t, std::uint8_t> deconstruct(uint8_t key) noexcept {
    return std::pair(key, 0);
  }

  inline uint16_t append(uint16_t key, size_t digit) noexcept {
    return static_cast<uint16_t>((key << 8) | digit);
  }

  inline std::pair<size_t, std::uint16_t> deconstruct(uint16_t key) noexcept {
    return std::pair(key & 0xff, key >> 8);
  }

  inline uint32_t append(uint32_t key, size_t digit) noexcept {
    return static_cast<uint32_t>((key << 8) | digit);
  }

  inline std::pair<size_t, std::uint32_t> deconstruct(uint32_t key) noexcept {
    return std::pair(key & 0xff, key >> 8);
  }

  inline uint64_t append(uint64_t key, size_t digit) noexcept {
    return static_cast<uint64_t>((key << 8) | digit);
  }

  inline std::pair<size_t, std::uint64_t> deconstruct(uint64_t key) noexcept {
    return std::pair(key & 0xff, key >> 8);
  }

  inline float append(float key, size_t digit) noexcept {
    union {
      float f;
      uint32_t i;
    } u;
    u.f = key;
    u.i = append(u.i, digit);
    return u.f;
  }
  
  inline std::pair<size_t, float> deconstruct(float key) noexcept {
    union {
      float f;
      uint32_t i;
    } u;
    u.f = key;
    return deconstruct(u.i);
  }

  inline double append(double key, size_t digit) noexcept {
    union {
      double f;
      uint64_t i;
    } u;
    u.f = key;
    u.i = append(u.i, digit);
    return u.f;
  }

  inline std::pair<size_t, double> deconstruct(double key) noexcept {
    union {
      double f;
      uint64_t i;
    } u;
    u.f = key;
    return deconstruct(u.i);
  }

  inline std::string append(const std::string & key, size_t digit) {
    std::string key2 = key;
    key2 += static_cast<char>(static_cast<uint8_t>(digit));
    return key2;
  }
  
  inline std::pair<size_t, std::string> deconstruct(const std::string & key) noexcept {
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
    static constexpr size_t min_load_factor100 = 15;
    static constexpr size_t max_load_factor100 = 60;
    
    using key_type = Key;
    using mapped_type = T;
    using value_type = typename std::conditional<is_set, Key, std::pair<Key, T>>::type;
    using size_type = size_t;
    using Self = Table<key_type, mapped_type>;

  private:
    struct Node {
      void set_payload(value_type * payload) { payload_ = payload; }
      value_type * get_payload() { return payload_; }
      const value_type * get_payload() const { return payload_; }
      bool is_assigned() const { return combined_ != 0; }
      uint32_t get_depth_lsb() const { return (combined_ >> 8) & 0xff; }
      const key_type & get_prefix_key() const { return prefix_key_; }
      size_t get_ordinal() const { return combined_ & 0xff; }
      size_t get_value_count() const { return combined_ >> 16; }

      void reset() {
	combined_ = 0;
      }

      void assign(size_t depth, key_type prefix_key, size_t ordinal) {
	new (static_cast<void*>(&(prefix_key_))) key_type(std::move(prefix_key));
	combined_ = (1 << 16) | ((depth & 0xff) << 8) | ordinal;
	payload_ = nullptr;
      }
      
      void inc_value_count() {
	combined_ += 65536;
      }

      bool dec_value_count() {
	combined_ -= 65536;
	if (combined_ < 65536) {
	  combined_ = 0;
	  return true;
	} else {
	  return false;
	}
      }

      bool equals(size_t depth, const key_type & prefix_key, size_t ordinal) const {
	return (depth & 0xff) == get_depth_lsb() && ordinal == get_ordinal() && prefix_key == prefix_key_;
      }

    private:
      uint64_t combined_; // from low to high, bits 1-8 = ordinal, 9-16 = lsb of depth, the rest = value count
      value_type * payload_;
      key_type prefix_key_;
    };

  public:

    template <bool IsConst>
    struct Iterator
    {
      using value_type        = typename Self::value_type;
      using TablePtr	      = typename std::conditional<IsConst, Table<key_type, mapped_type> const*, Table<key_type, mapped_type>*>::type;
      using PayloadPtr        = typename std::conditional<IsConst, value_type const*, value_type *>::type;
      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using reference         = typename std::conditional<IsConst, value_type const&, value_type&>::type;
      using pointer           = typename std::conditional<IsConst, value_type const*, value_type*>::type;

      // end iterator
      Iterator(TablePtr table) noexcept
	: table_(table),
	  ptr_(nullptr),
	  ordinal_(0),
	  offset_(0),
	  hash0_(0),
	  hash_(0),
	  prefix_key_(),
	  depth_(0) { }
      
      Iterator(TablePtr table, PayloadPtr ptr, uint32_t depth, key_type prefix_key, size_t ordinal, size_t offset, size_t hash0, size_t hash) noexcept
	: table_(table),
	  ptr_(ptr),
	  ordinal_(ordinal),
	  offset_(offset),
	  hash0_(hash0),
	  hash_(hash),
	  prefix_key_(std::move(prefix_key)),
	  depth_(depth) { }
      
      reference operator*() const noexcept {
	return *ptr_;
      }
      
      pointer operator->() noexcept {
	return ptr_;
      }
      
      Iterator& operator++() noexcept {
	if (!ptr_) {
	  return *this; // already ended
	}
	
	// go to the next direct Node
	if (depth_ == 0) {
	  // empty key
	  depth_++;
	  ordinal_ = offset_ = 0;
	  hash0_ = calc_unordered_hash(depth_, prefix_key_);
	} else {
	  auto node = repair_and_get_node();
	  if (node->get_value_count() > 1) {
	    depth_++;
	    prefix_key_ = append(prefix_key_, ordinal_);
	    ordinal_ = 0;
	    hash0_ = calc_unordered_hash(depth_, prefix_key_);
	  } else {
	    ordinal_++;
	  }
	  offset_ = 0;
	}

	// iterate until a final Node is found
	hash_ = calc_final_hash(hash0_, ordinal_);
	auto node = table_->read_node(hash_);
	auto nodes_start = table_->get_nodes_start(), nodes_end = table_->get_nodes_end();

	while ( 1 ) {
	  if (ordinal_ == bucket_count) {
	    // we have run through the whole range => go down the tree
	    if (depth_ <= 1) {
	      clear(); // become an end iterator
	      return *this;
	    } else {
	      auto [ parent_ordinal, parent_prefix_key ] = deconstruct(prefix_key_);
	      depth_--;
	      prefix_key_ = parent_prefix_key;
	      ordinal_ = parent_ordinal + 1;
	      offset_ = 0;
	      hash0_ = calc_unordered_hash(depth_, prefix_key_);
	      hash_ = calc_final_hash(hash0_, ordinal_);
	      node = table_->read_node(hash_);
	    }
	  } else if (!node->is_assigned()) {
	    // Node is not assigned
	    ordinal_++;
	    offset_ = 0;
	    hash_ = calc_final_hash(hash0_, ordinal_);
	    node = table_->read_node(hash_);
	  } else if (!node->equals(depth_, prefix_key_, ordinal_)) {
	    // collision
	    if (++node == nodes_end) node = nodes_start;
	    offset_++;
	  } else if (node->get_payload()) {
	    // a final Node was found
	    set_ptr(node->get_payload());
	    return *this;
	  } else {
	    // non-final node => go up the tree
	    depth_++;
	    prefix_key_ = append(prefix_key_, ordinal_);
	    ordinal_ = offset_ = 0;
	    offset_ = 0;
	    hash0_ = calc_unordered_hash(depth_, prefix_key_);
	    hash_ = calc_final_hash(hash0_, ordinal_);
	    node = table_->read_node(hash_);
	  }
	}
      }
      
      Iterator operator++(int) noexcept {
	Iterator<IsConst> tmp = *this;
	++(*this);
	return tmp;
      }
      
      template <bool O>
      bool operator== (const Iterator<O>& o) const noexcept {
	return ptr_ == o.ptr_;
      }
      
      template <bool O>
      bool operator!= (const Iterator<O>& o) const noexcept {
	return ptr_ != o.ptr_;
      }
      
      void fast_forward() noexcept {
	// first look for 0-length node (depth = ordinal = 0)
	depth_ = 0;
	offset_ = 0;
	prefix_key_ = key_type();
	ordinal_ = 0;
	hash0_ = calc_unordered_hash(depth_, prefix_key_);
	hash_ = calc_final_hash(hash0_, ordinal_);
		      
	while (1) {
	  auto node = table_->read_node(hash_, offset_);
	  if (node->is_assigned()) {
	    if (!node->equals(depth_, prefix_key_, 0)) {
	      // collision
	      offset_++;
	      continue;
	    } else if (node->get_payload()) {
	      set_ptr(node->get_payload());
	      return;
	    }
	  }
	  break;
	}
	
	depth_ = 1;
	offset_ = 0;
 	hash0_ = calc_unordered_hash(depth_, prefix_key_);
 	hash_ = calc_final_hash(hash0_, ordinal_);
       
	while ( 1 ) {
	  if (ordinal_ == bucket_count) {
#ifdef DEBUG
	    std::cerr << "fast-forward failed\n";
	    abort();
#endif
	  } else {
	    auto node = table_->read_node(hash_, offset_);
	    if (!node->is_assigned()) {
	      // unassigned
	      ordinal_++;
	      offset_ = 0;
	      hash_ = calc_final_hash(hash0_, ordinal_);
	    } else if (!node->equals(depth_, prefix_key_, ordinal_)) {
	      // collision
	      offset_++;
	    } else if (node->get_payload()) {
	      set_ptr(node->get_payload());
	      return;
	    } else {
	      depth_++;
	      prefix_key_ = append(prefix_key_, ordinal_);
	      ordinal_ = 0;
	      hash0_ = calc_unordered_hash(depth_, prefix_key_);
	      hash_ = calc_final_hash(hash0_, ordinal_);
	    }
	  }
	}
      }

      void down() {
	if (depth_ <= 1) {
	  clear(); // become an end iterator
	} else {
	  depth_--;
	  auto [ parent_ordinal, parent_prefix_key ] = deconstruct(prefix_key_);
	  prefix_key_ = parent_prefix_key;
	  ordinal_ = parent_ordinal;
	  offset_ = 0;
	  ptr_ = nullptr;
	  hash0_ = calc_unordered_hash(depth_, prefix_key_);
	  hash_ = calc_final_hash(hash0_, ordinal_);
	  while ( 1 ) {
	    auto node = table_->read_node(hash_, offset_);
	    if (!node->is_assigned()) {
#ifdef DEBUG
	      std::cerr << "down() failed\n";
#endif
	      abort();
	    } else if (node->equals(depth_, prefix_key_, ordinal_)) {
	      break;
	    } else {
	      offset_++;
	    }
	  }
	}
      }
      size_t get_offset() const noexcept { return offset_; }
      size_t get_hash() const noexcept { return hash_; }
      
      void set_ptr(PayloadPtr ptr) { ptr_ = ptr; }
      
    private:
      void clear() {
	ptr_ = nullptr;
	depth_ = 0;
	prefix_key_ = key_type{};
	ordinal_ = 0;
	offset_ = 0;
	hash0_ = 0;
	hash_ = 0;
      }

      Node * repair_and_get_node() {
	auto node0 = table_->read_node(hash_, offset_);
	if (ptr_ == node0->get_payload()) return node0;
	auto node = table_->read_node(hash_);
	offset_ = 0;
	while ( 1 ) {
	  auto node = table_->read_node(hash_, offset_);
	  if (!node->is_assigned()) {
#ifdef DEBUG
	    std::cerr << "repair failed\n";
#endif
	    abort();
	  } else if (ptr_ == node->get_payload()) {
	    return node;
	  }
	  offset_++;
	}
	return node;
      }
      
      TablePtr table_;
      PayloadPtr ptr_;

      // * cached values
      // they are all obtainable from ptr_, but it's faster to cache them
      // only temporarily can an iterator might point to a non-final Node (a node that has no ptr_)
      size_t ordinal_, offset_, hash0_, hash_;
      key_type prefix_key_;
      uint32_t depth_;
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
	table_mask_(std::exchange(other.table_mask_, 0)),
	nodes_(std::exchange(other.nodes_, nullptr)),
    	arena_(std::move(other.arena_)) { }

    Table & operator=(Table && other) noexcept {
      std::swap(num_entries_, other.num_entries);
      std::swap(num_final_entries_, other.num_final_entries_);
      std::swap(num_inserts_, other.num_inserts_);
      std::swap(num_insert_collisions_, other.num_insert_collisons_);
      std::swap(table_size_, other.table_size_);
      std::swap(table_mask_, other.table_mask_);
      std::swap(nodes_, other.nodes_);
      std::swap(arena_, other.arena_);
      return *this;
    }
    
    Table(const Table & other) = delete;
    Table& operator=(const Table & other) = delete;

    ~Table() noexcept {
      clear();
    }

    void clear() noexcept {
      for (size_t i = 0; i < table_size_; i++) {
	auto & node = nodes_[i];
	if (node.is_assigned()) {
	  node.get_prefix_key().~key_type();
	  if (node.get_payload()) {
	    node.get_payload()->~value_type();
	  }
	}
      }
      std::free(nodes_);
      num_entries_ = num_final_entries_ = num_inserts_ = num_insert_collisions_ = table_size_ = table_mask_ = 0;
      nodes_ = nullptr;
      arena_.clear();
    }
    
    iterator find(const key_type & key) noexcept {
      if (!table_size_) return end();
      auto depth = static_cast<uint32_t>(keysize(key));
      auto [ ordinal, prefix_key ] = deconstruct(key);
      auto hash0 = calc_unordered_hash(depth, prefix_key);
      auto hash = calc_final_hash(hash0, ordinal);  
      auto node_initial = read_node(hash);
      auto nodes_start = get_nodes_start(), nodes_end = get_nodes_end();

      auto node = node_initial;
      while ( 1 ) {
	if (!node->is_assigned()) {
	  break; // not found
	} else if (!node->equals(depth, prefix_key, ordinal)) {
	  // collision
	  if (++node == nodes_end) node = nodes_start;
	} else if (node->get_payload()) {
	  return iterator(this, node->get_payload(), depth, prefix_key, ordinal, static_cast<size_t>(node - node_initial), hash0, hash);
	} else {
	  break; // not final / wrong key
	}
      }
      return end();
    }

    const_iterator find(const key_type & key) const noexcept {
      if (!table_size_) return cend();
      auto depth = static_cast<uint32_t>(keysize(key));
      auto [ ordinal, prefix_key ] = deconstruct(key);
      auto hash0 = calc_unordered_hash(depth, prefix_key);
      auto hash = calc_final_hash(hash0, ordinal);  
      auto node_initial = read_node(hash);
      auto nodes_start = get_nodes_start(), nodes_end = get_nodes_end();

      auto node = node_initial;
      while ( 1 ) {
	if (!node->is_assigned()) {
	  break; // not found
	} else if (!node->equals(depth, prefix_key, ordinal)) {
	  // collision
	  if (++node == nodes_end) node = nodes_start;
	} else if (node->get_payload()) {
	  return const_iterator(this, node->get_payload(), depth, prefix_key, ordinal, static_cast<size_t>(node - node_initial), hash0, hash);
	} else {
	  break; // not final / wrong key
	}
      }
      return cend();
    }

    size_t count(const key_type & key) const noexcept {
      return find(key) == cend() ? 0 : 1;
    }
    
    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, std::pair<iterator,bool>>::type insert_or_assign(const Key& k, Q && obj) {
      auto [ node, it ] = create_nodes_for_key(k);
      bool is_new = true;	
      if (!node->get_payload()) {
	node->set_payload(arena_.alloc());
	it.set_ptr(node->get_payload());
	new (static_cast<void*>(node->get_payload())) value_type(k, std::move(obj));
	num_final_entries_++;
      } else {
	*(node->get_payload()) = value_type(k, std::move(obj));
	is_new = false;
      }
      return std::make_pair(it, is_new);
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      value_type vt{std::forward<Args>(args)...};
      auto [ node, it ] = create_nodes_for_key(getFirstConst(vt));
      bool is_new = true;
      if (!node->get_payload()) {
	node->set_payload(arena_.alloc());
	it.set_ptr(node->get_payload());
	new (static_cast<void*>(node->get_payload())) value_type(std::move(vt));
	num_final_entries_++;
      } else {
	is_new = false;
      }
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
    typename std::enable_if<!std::is_void<Q>::value, Q&>::type operator[](const key_type& key) noexcept {
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

    iterator erase(iterator pos) {
      auto node = read_node(pos.get_hash(), pos.get_offset());
      if (!node->is_assigned() || !node->get_payload()) {
#ifdef DEBUG
	std::cerr << "invalid parameter to erase()\n";
#endif
	abort();
      }
      auto next_pos = pos;
      ++next_pos;

      node->get_payload()->~value_type();
      arena_.dealloc(node->get_payload());
      node->set_payload(nullptr);

      if (node->dec_value_count()) {
	node->get_prefix_key().~key_type();
	num_entries_--;
      }
      num_final_entries_--;
      
      pos.down();
      while ( pos != end() ) {
	auto node = read_node(pos.get_hash(), pos.get_offset());
	if (node->dec_value_count()) {
	  node->get_prefix_key().~key_type();
	  num_entries_--;
	}
	pos.down();
      }

      if (table_size_ > bucket_count && get_load_factor() < min_load_factor100) { // Check the load factor
	resize(table_size_ >> 1);
      }
      
      return next_pos;
    }

    iterator erase(iterator first, iterator last) {
      while ( 1 ) {
	bool is_last = first == last;
	first = erase(first);
	if (first == end()) {
#ifdef DEBUG
	  std::cerr << "last not found\n";
#endif
	  abort();
	}
	if (is_last) return first;
      }
    }

    size_t erase(const key_type & key) {
      auto it = find(key);
      if (it != end()) {
	erase(it);
	return 1;
      } else {
	return 0;
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

    const_iterator cbegin() noexcept {
      if (size()) {
	const_iterator it(this);
	it.fast_forward();
	return it;
      } else {
	return end();
      }
    }
    const_iterator cend() const noexcept {
      // iterator is by default and end iterator (the depth is zero)
      return const_iterator(this);
    }

    bool empty() const noexcept { return num_final_entries_ == 0; }
    size_t size() const noexcept { return num_final_entries_; }
    size_t num_inserts() const noexcept { return num_inserts_; }
    size_t num_insert_collisions() const noexcept { return num_insert_collisions_; }
    
  private:
    class Arena {
    private:
      static constexpr size_t page_size = 4096;
      
    public:
      Arena() { }
      Arena(Arena && other) noexcept
	: n_(std::exchange(other.n_, 0)),
	  pages_(std::move(other.pages_)) { }
      ~Arena() noexcept {
	clear();
      }

      Arena & operator=(Arena && other) noexcept {
	std::swap(n_, other.n);
	std::swap(pages_, other.pages_);
	return *this;
      }
    
      Arena(const Arena & other) = delete;
      Arena& operator=(const Arena & other) = delete;

      value_type * alloc() {
	if (!free_list_.empty()) {
	  value_type * ptr = free_list_.back();
	  free_list_.pop_back();
	  return ptr;
	} else {
	  if (pages_.empty() || n_ == page_size) {
	    auto p = reinterpret_cast<value_type*>(std::malloc(page_size * sizeof(value_type)));
	    if (!p) throw std::bad_alloc();
	    pages_.push_back(p);
	    n_ = 0;
	  }
	  return pages_.back() + n_++;
	}
      }

      void dealloc(value_type * ptr) {
	free_list_.push_back(ptr);
      }

      void clear() noexcept {
	for (size_t i = 0; i < pages_.size(); i++) {
	  std::free(pages_[i]);
	}
	n_ = 0;
	pages_.clear();	
      }
      
    private:
      size_t n_ = 0;
      std::vector<value_type*> pages_;
      std::vector<value_type*> free_list_;
    };

    size_t get_load_factor() const noexcept { return 100 * num_entries_ / table_size_; }
    
    std::pair<Node *, iterator> create_nodes_for_key(key_type key) {
      if (!nodes_) {
	init(bucket_count);
      } else if (get_load_factor() >= max_load_factor100) { // Check the load factor
	resize(table_size_ * 2);
      }

      num_inserts_++;

      auto n = keysize(key);
      auto depth = static_cast<uint32_t>(n);
      auto nodes_start = get_nodes_start(), nodes_end = get_nodes_end();
      iterator it = end();
      Node * final_node = nullptr;
      
      // insert digits from least significant to most significant
      // even if keysize is zero, add at least one digit (for empty strings)
      for ( size_t i = 0; i < (n == 0 ? 1 : n); i++, depth-- ) {
	auto [ ordinal, prefix_key ] = deconstruct(key);
	auto hash0 = calc_unordered_hash(depth, prefix_key);
	auto hash = calc_final_hash(hash0, ordinal);
	auto node_initial = read_node(hash);

	auto node = node_initial;
	while ( 1 ) {
	  if (!node->is_assigned()) {
	    node->assign(depth, prefix_key, ordinal);
	    num_entries_++;
	  } else if (!node->equals(depth, prefix_key, ordinal)) {
	    // collision
	    if (++node == nodes_end) node = nodes_start;
	    num_insert_collisions_++;
	    continue;
	  } else {
	    node->inc_value_count();
	  }
	  if (!final_node) {
	    final_node = node;
	    it = iterator(this, node->get_payload(), depth, prefix_key, ordinal, static_cast<size_t>(node - node_initial), hash0, hash);
	  }
	  break;
	}

	key = std::move(prefix_key);
      }

      return std::pair(final_node, it);
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
      table_size_ = s;
      table_mask_ = s - 1;
      nodes_ = alloc_nodes(s);
    }

    static Node * alloc_nodes(size_t s) {
      auto nodes = reinterpret_cast<Node*>(std::malloc(s * sizeof(Node)));
      if (!nodes) throw std::bad_alloc();
      for (size_t i = 0; i < s; i++) {
	nodes[i].reset();
      }
      return nodes;
    }
    
    void resize(size_t new_size) {
      auto new_mask = new_size - 1;
      auto new_nodes = alloc_nodes(new_size);
      auto new_nodes_end = new_nodes + new_size;
      
      auto node = nodes_;
      auto end = nodes_ + table_size_;
      for (; node != end; node++) {
	if (node->is_assigned()) {
	  auto hash0 = calc_unordered_hash(node->get_depth_lsb(), node->get_prefix_key());
	  auto hash = calc_final_hash(hash0, node->get_ordinal());
	  auto new_node = new_nodes + (hash & new_mask);
	  
	  while ( 1 ) {
	    if (new_node->is_assigned()) {
	      if (++new_node == new_nodes_end) new_node = new_nodes;
	      num_insert_collisions_++;
	    } else {
	      std::swap(*node, *new_node);
	      node->~Node();
	      break;
	    }
	  }
	}
      }
      std::free(nodes_);
      nodes_ = new_nodes;
      table_size_ = new_size;
      table_mask_ = new_mask;
    }

    Node * read_node(size_t h, size_t offset) noexcept {return nodes_ + ((h + offset) & table_mask_); }
    const Node * read_node(size_t h, size_t offset) const noexcept { return nodes_ + (h + offset) & table_mask_; }

    Node * read_node(size_t h) noexcept { return nodes_ + (h & table_mask_); }
    const Node * read_node(size_t h) const noexcept { return nodes_ + (h & table_mask_); }

    const Node * get_nodes_start() const noexcept { return nodes_; }
    const Node * get_nodes_end() const noexcept { return nodes_ + table_size_; }
    Node * get_nodes_start() noexcept { return nodes_; }
    Node * get_nodes_end()  noexcept { return nodes_ + table_size_; }

    // hash calculation functions use Murmur3 to calculate hash for a Node.
    // Murmur3 operations are specialized for both 32 bit and 64 bit size_t
    static inline size_t calc_unordered_hash(size_t depth, const key_type & prefix_key) noexcept {
      auto k1 = murmur3_mix_k1(std::hash<key_type>{}(prefix_key));
      return murmur3_mix_h1(depth, k1);
    }

    static inline size_t calc_final_hash(size_t h1, size_t ordinal) noexcept {
      auto k1 = murmur3_mix_k1(ordinal);
      h1 = murmur3_mix_h1(h1, k1);
      return murmur3_fmix(h1);
    }

    size_t num_entries_ = 0, num_final_entries_ = 0;
    size_t num_inserts_ = 0, num_insert_collisions_ = 0;
    size_t table_size_ = 0, table_mask_ = 0;
    Node* nodes_ = nullptr;
    Arena arena_;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
