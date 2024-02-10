#ifndef _RADIXCPP_H_
#define _RADIXCPP_H_

#include <cstdint>
#include <utility>
#include <string>
#include <stdexcept>

// #define DEBUG

#ifdef DEBUG
#include <iostream>
#endif

namespace radix_cpp {
  inline constexpr uint32_t flag_is_assigned = 1;
  inline constexpr uint32_t flag_has_children = 2;
  inline constexpr uint32_t flag_is_deleted = 4;
   
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
    static constexpr size_t max_load_factor100 = 60;
    
    using key_type = Key;
    using mapped_type = T;
    using value_type = typename std::conditional<is_set, Key, std::pair<Key, T>>::type;
    using size_type = size_t;
    using Self = Table<key_type, mapped_type>;

  private:
    struct Node {
      value_type * keyval;
      key_type prefix_key;
      uint32_t depth;
      uint8_t flags, ordinal;
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
	  ptr_(nullptr),
	  ordinal_(0),
	  offset_(0),
	  prefix_key_(),
	  depth_(0) { }
      
      Iterator(Self * table, value_type * ptr, uint32_t depth, key_type prefix_key, size_t ordinal, size_t offset) noexcept
	: table_(table),
	  ptr_(ptr),
	  ordinal_(ordinal),
	  offset_(offset),
	  prefix_key_(std::move(prefix_key)),
	  depth_(depth) { }
      
      void set_indices(value_type * ptr, uint32_t depth, key_type prefix_key, size_t ordinal, size_t offset) noexcept {
	ptr_ = ptr;
	depth_ = depth;
	prefix_key_ = std::move(prefix_key);
	ordinal_ = ordinal;
	offset_ = offset;
      }

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
	} else {
	  auto & node = repair_and_get_node();
	  if (node.flags & flag_has_children) {
	    depth_++;
	    prefix_key_ = append(prefix_key_, ordinal_);
	    ordinal_ = 0;
	  } else {
	    ordinal_++;
	  }
	  offset_ = 0;
	}

	// iterate until a final Node is found
	size_t h = get_hash();
	while ( 1 ) {
	  if (ordinal_ == bucket_count) {
	    // we have run through the whole range => go down the tree
	    if (depth_ <= 1) {
	      // become an end iterator
	      set_indices(nullptr, 0, key_type{}, 1, 0);
	      return *this;
	    } else {
	      depth_--;
	      auto [ parent_ordinal, parent_prefix_key ] = deconstruct(prefix_key_);
	      prefix_key_ = parent_prefix_key;
	      ordinal_ = parent_ordinal + 1;
	      offset_ = 0;
	      h = get_hash();
	    }
	  } else {
	    auto & node = table_->read_node(h, offset_);
	    if (!node.flags) {
	      // Node is not assigned
	      ordinal_++;
	      offset_ = 0;
	      h = get_hash();
	    } else if (node.depth != depth_ || node.ordinal != ordinal_ || node.prefix_key != prefix_key_) {
	      // collision
	      offset_++;
	    } else if (node.flags & flag_is_deleted) {
	      // Node is deleted
	      ordinal_++;
	      offset_ = 0;
	      h = get_hash();	      
	    } else if (node.keyval) {
	      // a final Node was found
	      set_ptr(node.keyval);
	      return *this;
	    } else {
	      // non-final node => go up the tree
	      depth_++;
	      prefix_key_ = append(prefix_key_, ordinal_);
	      ordinal_ = offset_ = 0;
	      h = get_hash();
	    }
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
	auto h = get_hash();
	while (1) {
	  auto & node = table_->read_node(h, offset_);
	  if (!node.flags) break;
	  else if (node.depth == 0) {
	    if (node.flags & flag_is_deleted) {
	      break;
	    } else {
	      set_ptr(node.keyval);
	      return;
	    }
	  } else {
	    offset_++;
	  }
	}

	depth_ = 1;
	offset_ = 0;
	h = get_hash();
       
	while ( 1 ) {
	  auto & node = table_->read_node(h, offset_);
	  if (!node.flags) {
	    ordinal_++;
	    offset_ = 0;
	    h = get_hash();
	  } else if (depth_ == node.depth && ordinal_ == node.ordinal && prefix_key_ == node.prefix_key) {
	    if (node.flags & flag_is_deleted) {
	      ordinal_++;
	      offset_ = 0;
	      h = get_hash();
	    } else if (node.keyval) {
	      set_ptr(node.keyval);
	      return;
	    } else {
	      depth_++;
	      prefix_key_ = append(prefix_key_, ordinal_);
	      ordinal_ = 0;
	      h = get_hash();
	    }
	  } else {
	    offset_++;
	  }
	  if (ordinal_ == bucket_count) {
#ifdef DEBUG
	    std::cerr << "fast forward: could not find next subiterator for prefix " << prefix_key_ << "\n";
#endif
	    abort();
	  }
	}
      }

      size_t get_offset() const noexcept { return offset_; }
      size_t get_hash() const noexcept { return calc_hash(depth_, prefix_key_, ordinal_); }
      
      void set_ptr(value_type * ptr) { ptr_ = ptr; }

    private:
      Node & repair_and_get_node() {
	size_t h = get_hash();
	auto & node0 = table_->read_node(h, offset_);
	if (ptr_ == node0.keyval) return node0;
	offset_ = 0;
	while ( 1 ) {
	  auto & node = table_->read_node(h, offset_);
	  if (!node.flags) {
#ifdef DEBUG
	    std::cerr << "repair failed\n";
#endif
	    abort();
	  } else if (ptr_ == node.keyval) {
	    return node;
	  }
	  offset_++;
	}
      }
      
      Self * table_;
      value_type * ptr_;
      size_t ordinal_, offset_;
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
	nodes_(std::exchange(other.nodes_, nullptr)),
    	arena_(std::move(other.arena_)) { }

    Table & operator=(Table && other) noexcept {
      std::swap(num_entries_, other.num_entries);
      std::swap(num_final_entries_, other.num_final_entries_);
      std::swap(num_inserts_, other.num_inserts_);
      std::swap(num_insert_collisions_, other.num_insert_collisons_);
      std::swap(table_size_, other.table_size_);
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
	if (node.flags) node.~Node();
      }
      std::free(nodes_);
      num_entries_ = num_final_entries_ = num_inserts_ = num_insert_collisions_ = table_size_ = 0;
      nodes_ = nullptr;
      arena_.clear();
    }

    iterator find(const key_type & key) noexcept {
      if (!table_size_) return end();
      auto depth = static_cast<uint32_t>(keysize(key));
      auto [ ordinal, prefix_key ] = deconstruct(key);
      size_t offset = 0, h = calc_hash(depth, prefix_key, ordinal);

      while ( 1 ) {
	auto & node = read_node(h, offset);
	if (!node.flags) {
	  break; // not found
	} else if (node.depth != depth || node.ordinal != ordinal || node.prefix_key != prefix_key) {
	  // collision
	  offset++;
	} else if (node.flags & flag_is_deleted) {
	  return end();
	} else if (node.keyval) {
	  return iterator(this, node.keyval, depth, prefix_key, ordinal, offset);
	} else {
	  break; // not final / wrong key
	}
      }
      return end();
    }
    
    template <typename Q = mapped_type>
    typename std::enable_if<!std::is_void<Q>::value, std::pair<iterator,bool>>::type insert_or_assign(const Key& k, Q && obj) {
      auto [ it, hash ] = create_nodes_for_key(k);
      auto & node = read_node(hash, it.get_offset());
      bool is_new = true;	
      if (!node.keyval) {
	node.keyval = arena_.alloc();
	it.set_ptr(node.keyval);
	new (static_cast<void*>(node.keyval)) value_type(k, std::move(obj));
	num_final_entries_++;
      } else {
	*(node.keyval) = value_type(k, std::move(obj));
	if (node.flags & flag_is_deleted) {
	  node.flags ^= flag_is_deleted;
	  num_final_entries_++;
	} else {
	  is_new = false;
	}
      }
      return std::make_pair(it, is_new);
    }

    template <typename... Args>
    std::pair<iterator,bool> emplace(Args&&... args) {
      value_type vt{std::forward<Args>(args)...};
      auto [ it, hash ] = create_nodes_for_key(getFirstConst(vt));
      auto & node = read_node(hash, it.get_offset());
      bool is_new = true;
      if (!node.keyval) {
	node.keyval = arena_.alloc();
	it.set_ptr(node.keyval);
	new (static_cast<void*>(node.keyval)) value_type(std::move(vt));
	num_final_entries_++;
      } else if (node.flags & flag_is_deleted) {
	*(node.keyval) = vt;
	node.flags ^= flag_is_deleted;
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
      size_t offset = pos.get_offset(), h = pos.get_hash();
      auto & node = read_node(h, offset);
      if (node.flags && !(node.flags & flag_is_deleted)) {
	node.flags |= flag_is_deleted;
	num_final_entries_--;
      }
      return ++pos;
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
	if (pages_.empty() || n_ == page_size) {
	  pages_.push_back(reinterpret_cast<value_type*>(std::malloc(page_size * sizeof(value_type))));
	  n_ = 0;
	}
	return pages_.back() + n_++;
      }

      void clear() noexcept {
	for (size_t i = 0; i < pages_.size(); i++) {
	  size_t n = i + 1 == pages_.size() ? n_ : page_size;
	  auto data = pages_[i];
	  for (size_t j = 0; j < n; j++) {
	    data[j].~value_type();
	  }
	  std::free(data);
	}
	pages_.clear();
      }
      
    private:
      size_t n_ = 0;
      std::vector<value_type*> pages_;
    };
    
    std::pair<iterator, size_t> create_nodes_for_key(key_type key) {
      if (!nodes_) {
	init(bucket_count);
      } else if (100 * num_entries_ / table_size_ >= max_load_factor100) { // Check the load factor
	resize(table_size_ * 2);
      }

      num_inserts_++;

      auto n = keysize(key);
      auto depth = static_cast<uint32_t>(n);
      iterator it = end();
      size_t first_hash = 0;
      bool is_final = true;
            
      // insert digits from least significant to most significant
      // even if keysize is zero, add at least one digit (for empty strings)
      for ( size_t i = 0; i < (n == 0 ? 1 : n); i++, depth-- ) {
	auto [ ordinal, prefix_key ] = deconstruct(key);
	size_t offset = 0, h = calc_hash(depth, prefix_key, ordinal);
	
	while ( 1 ) {
	  auto & node = read_node(h, offset);
	  if (!node.flags) {
	    new (static_cast<void*>(&(node.prefix_key))) key_type(prefix_key);
	    node.flags = flag_is_assigned;
	    if (!is_final) {
	      node.flags |= flag_has_children;
	    }
	    node.depth = static_cast<uint32_t>(depth);
	    node.ordinal = static_cast<uint8_t>(ordinal);
	    num_entries_++;
	  } else if (node.depth != depth || node.ordinal != ordinal || node.prefix_key != prefix_key) {
	    // collision
	    offset++;
	    num_insert_collisions_++;
	    continue;
	  }
	  if (is_final) {
	    it = iterator(this, node.keyval, depth, prefix_key, ordinal, offset);
	    first_hash = h;
	    is_final = false;
	  }
	  break;
	}

	key = std::move(prefix_key);
      }

      return std::pair(it, first_hash);
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
      nodes_ = alloc_nodes(s);
    }

    static Node * alloc_nodes(size_t s) {
      auto nodes = reinterpret_cast<Node*>(std::malloc(s * sizeof(Node)));
      for (size_t i = 0; i < s; i++) {
	auto & node = nodes[i];
	node.flags = 0;
	node.keyval = nullptr;
      }
      return nodes;
    }
    
    void resize(size_t new_size) {
      auto new_nodes = alloc_nodes(new_size);
      
      for (size_t i = 0; i < table_size_; i++) {
	Node & node = nodes_[i];
	if (node.flags) {
	  size_t h = calc_hash(node.depth, node.prefix_key, node.ordinal);
	  size_t offset = 0;
	  while ( 1 ) {
	    auto & output_node = new_nodes[(h + offset) & (new_size - 1)];
	    if (output_node.flags) {
	      offset++;
	      num_insert_collisions_++;
	    } else {
	      std::swap(node, output_node);
	      node.~Node();
	      break;
	    }
	  }
	}
      }
      std::free(nodes_);
      nodes_ = new_nodes;
      table_size_ = new_size;
    }

    Node & read_node(size_t h, size_t offset) noexcept {
      return nodes_[(h + offset) & (table_size_ - 1)];
    }

    // calc_hash() uses Murmur3 to calculate hash for a Node.
    // Murmur3 operations are specialized for both 32 bit and 64 bit size_t
    static inline size_t calc_hash(uint32_t depth, const key_type & prefix_key, size_t ordinal) noexcept {
      auto k1 = murmur3_mix_k1(std::hash<key_type>{}(prefix_key));
      auto h1 = murmur3_mix_h1(std::size_t{0}, k1);

      k1 = murmur3_mix_k1((static_cast<size_t>(depth) << 8) | ordinal);
      h1 = murmur3_mix_h1(h1, k1);

      return murmur3_fmix(h1);
    }

    size_t num_entries_ = 0, num_final_entries_ = 0;
    size_t num_inserts_ = 0, num_insert_collisions_ = 0;
    size_t table_size_ = 0;
    Node* nodes_ = nullptr;
    Arena arena_;
  };

  template <typename Key>
  using set = Table<Key, void>;

  template <typename Key, typename Value>
  using map = Table<Key, Value>;
};

#endif
