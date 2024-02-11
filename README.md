# radix-cpp

[![GitHub License](https://img.shields.io/github/license/rekola/radix-cpp?logo=github&logoColor=lightgrey&color=yellow)](https://github.com/rekola/radix-cpp/blob/main/LICENSE)
[![CI](https://github.com/rekola/radix-cpp/workflows/Ubuntu-CI/badge.svg)]()
[![VS17-CI](https://github.com/rekola/radix-cpp/workflows/VS17-CI/badge.svg)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

## Radix set and map implementation for C++

radix-cpp is an experimental flat implementation of ordered set and
map. It uses a hash table with open addressing to implement a form of
radix sort combined with prefix trees, thus providing Θ(1) search time
as is usual for hash tables, but also quick in order traversal of the
keys. Ordinarily hash tables are not ordered, and while in theory, an
order preserving hash function could be used, it would lead to large
number of collisions. In this implementation the key is divided into
multiple 8-bit digits, and each digit is inserted separately in to the
hash table. The prefix key is also stored along with each digit to
construct the prefix tree. According to benchmarks (given uint32_t
keys) the radix-cpp set construction is much faster than that of
std::set, and also faster than sorting the keys using std::sort. Both
std::sort and std::set use comparison sorthing so they have time
complexity of O(n) = n log n.

Iterators are automatically repaired if the underlying table changes,
so they are stable.

Currently only strings and unsigned integers are supported as keys,
but more support is forthcoming.

### Time Complexity

| Operation | Average | Worst Case |
| - | - | - |
| Search | Θ(1) | O(n) |
| Insert | Θ(w) | O(w*n) |
| Delete | Θ(w) | O(w*n) |

* w is the key length in bytes

Iterating over nodes in order can be somewhat expensive if the next
node has different prefix. Also, it's unclear what the time complexity
of the iteration operation is.

### Benchmarks

In these initial benchmarks, radix_cpp::set has been compared with
std::set using uint32_t as the key type. For comparison, a test with
std::sort and a std::vector is also done, and they have similar speed
as std::flat_set. The test consists of constructing a set out of
shuffled array of N consecutive integers and doing an ordered
iteration over the entire set. Search speed comparison has been
intentionally left out, since it would not be very useful given that
radix-cpp has avarage complexity of Θ(1).

![Ordered Set Construction Time](https://github.com/rekola/radix-cpp/assets/6755525/ec1adb25-52dc-407c-86c5-af1b2d97eca9 "Ordered Set Construction Time")
![Ordered Set Iteration Time](https://github.com/rekola/radix-cpp/assets/6755525/fe83baa4-7b15-4642-8f5e-1efed45f17a7 "Ordered Set Iteration Time")

## Implementation

radix-cpp uses Murmur3 as the hash function. The keys can be of
arbitrary size.

### Node

```c++
struct Node {
  value_type * keyval;
  size_t value_count;
  key_type prefix_key;
  uint32_t depth;
  uint8_t flags, ordinal;
};
```

### Inserting

When inserting a key, each 8-bit digit is inserted along with its
prefix. A prefix tree is thus created inside the hash table.

### Search

When searching for a known key, only the Node for the last digit needs
to be found. The input key is split into a prefix of n-1 bytes and 1
byte ordinal, where n is the size of the key. If a Node with the
prefix and ordinal is found, it is returned.

### Deletion

Deletion works by using tombstones.

### Iteration

An iterator has four variables: the depth (in the prefix tree), the
unordered prefix, the 8-bit ordinal value, and the offset. While the
ordinal is smaller than 255, we know that there are still nodes
available in the ordered range, and when advancing to the next stored
value, we can check them all in order. When the range runs out, we
fall back to the previous digit and advance that one. If the new node
is not a final node, we go upwards in the tree and find the smallest
final node. The offset is used for probing in case of collisions.

### Limitations and Future Plans

- Maximum key length is 2^32
- No signed types (digits of two's complement numbers are not ordered correctly)
- 32-bit support is untested
- How to sort std::any?
- Unordered iteration is need (e.g. for set union and intersection)

## Extending types

To implement set and map for custom type, the following free functions must be defined:

| Function | Description |
| - | - |
| key_type append(key_type key, size_t digit) | Returns a new key with digit appended as the new least significant digit |
| std::pair<key_type, size_t> deconstruct(key_type key) | Returns a pair with the numeric value of the least significant digit of the key and the key with the least significant digit removed |
| size_t keysize(key_type key) | Returns the number of digits in the key |

Additionally, there must exist a specialization of std::hash for the custom key.
