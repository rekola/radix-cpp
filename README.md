# radix-cpp

[![CI](https://github.com/rekola/radix-cpp/workflows/Ubuntu-CI/badge.svg)]()
[![VS17-CI](https://github.com/rekola/radix-cpp/workflows/VS17-CI/badge.svg)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

## Radix set and map implementation for C++

radix-cpp is an experimental flat implementation of ordered set and
map. It provides very fast lookup by using a hash table with open
addressing to implement a radix sort. Ordinarily hash tables are not
ordered, and while in theory, an order preserving hash function could
be used, it would lead to large number of collisions. In this
implementation the key is divided into multiple 8-bit digits, and each
digit is inserted seperately while preserving the ordering of the
previous stages.

Iterators are automatically repaired if the underlying table changes,
so they are stable.

Currently this implementation only works with strings and unsigned
integers, but more support is forthcoming.

### Time Complexity

| Operation | Average | Worst Case |
| - | - | - |
| Search | Θ(1) | O(n) |
| Insert | Θ(w) | O(w*n) |

* w is the key length in bytes

Iterating over nodes in order can be somewhat expensive if the next
node has different prefix. Also, it's unclear what the complexity of
the iteration operation is.

## Implementation

radix-cpp uses Murmur3 as the hash function. The keys can be of
arbitrary size.

### Inserting

When inserting a value is inserted, each 8-bit digit is inserted
separately with its prefix. A prefix tree is thus created inside the
hash table.

### Search

When searching for a known value, only the Node for the last digit
needs to be found. The input key is split into a prefix of n-1 bytes
and 1 byte ordinal, where n is the size of the key. If a Node with the
prefix and ordinal is found, it is returned.

### Deletion

Deletion is not yet implemented.

### Iteration

An iterator has four variables: the depth (in the prefix tree), the
unordered prefix, the 8-bit ordinal value, and the offset. While the
ordinal is smaller than 255, we know that there are still nodes
available in the ordered range, and when advancing to the next stored
value, we can check them all in order. When the range runs out, we
fall back to the previous digit and advance that one. If the new node
is not a final node, we go upwards in the tree and find the smallest
final node.

### Limitations

- Maximum key length is 2^32
- Deletion is not yet implemented
- No signed types
- No 32-bit support
- How to sort std::any?

## Extending types

To implement set and map for custom type, the following free functions must be defined:

| Function | Description |
| - | - |
| key_type prefix(key_type key, size_t n) | Returns the prefix of n digits of the key |
| size_t top(key_type key) | Returns the numeric value of the least significant digit of the key |
| size_t keysize(key_type key) | Returns the number of digits in the key |
| | A hash function for key_type in std |
