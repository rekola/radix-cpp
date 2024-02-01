# radix-cpp

[![CI](https://github.com/rekola/radix-cpp/workflows/Ubuntu-CI/badge.svg)]()
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](http://makeapullrequest.com)

## Radix set and map implementation for C++

radix-cpp is an experimental flat implementation of ordered set and
map. It provides very fast insert and lookup by using a hash table to
implement a radix sort. Ordinarily hash tables are not ordered, and
while in theory, an order preserving hash function could be used, it
would lead to large number of collisions. In this implementation the
key is divided into multiple 8-bit digits, and each digit is ordered
seperately while preserving the ordering of the previous stages.

Currently this implementation only works with strings and unsigned
integers, but more support is forthcoming.

### Time Complexity

| Operation | Average | Worst Case |
| - | - | - |
| Search | Θ(1) | O(n) |
| Insert | Θ(w) | O(w*n) |

* w is the key length

Iterating over nodes in order can be somewhat expensive if the next
node has different prefix. Also, it's unclear what the complexity of
Next operation is.

## Implementation

### Inserting

When inserting an element, the most significant digit is first mapped
to the first 256 entries in the beginning of the hash table and a node
is added if it doesn't already exist. The value of the first digit is
then hashes and the hash value used as an offset to the 256 entry
table where the second digit is stored. Then the 2-digit prefix is
hashed and used as the offset for the third digit. This is continued
until all of the digits are added. A prefix tree is thus created
inside the hash table.

### Search

### Deletion

Deletion is not yet implemented.

### Iteration

An iterator has three variables: the depth (in the prefix tree), the
start index (the node that the iterator points to) and the range. The
range tells us how many nodes are still remaining in the ordered range
that is currently visiting. While the range is positive, we know that
there are still nodes available in the ordered range, and when
advancing to the next stored value, we can check them all in
order. When the range runs out, we fall back to the previous digit and
advance that one. If the new node is not a final node (leaf node), we
go upwards in the tree and find the smallest leaf node.

### Limitations

- Maximum key length is 2^32
- Deletion is not yet implemented
- No signed types

## Extending types

To implement set and map for custom type, the following free functions must be defined:

| Function | Description |
| - | - |
| key_type prefix(key_type key, size_t n) | Returns the prefix of n digits of the key |
| size_t top(key_type key) | Returns the numeric value of the least significant digit of the key |
| size_t keysize(key_type key) | Returns the number of digits in the key |
| | A hash function for key_type in std |
