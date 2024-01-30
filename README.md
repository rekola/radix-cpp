# radix-cpp

## Radix set and map implementation.

radix-cpp is an experimental implementation of ordered set and map. It
provides very fast insert and lookup by using a hash table to
implement radix sort. Ordinary hash tables are not ordered, and while
in theory, an order preserving hash function could be used, it would
lead to large number of collisions. In this implementation the key is
divided into multiple 8-bit digits, and each digit is ordered
seperately while preserving the ordering of the previous stages.

## Implementation

When inserting an element, the most significant digit is first mapped
to the beginning of the hash table.

Currently this implementation only works with unsigned integers, but
more support is forthcoming.

## Extending types

To implement set and map for custom type, the following free functions must be defined:

| Function | Description |
| key_type prefix(key_type key, size_t n) | Returns the prefix of n digits of the key |
| size_t top(key_type key) | Returns the numeric value of the least significant digit of the key |

## Missing functionality

- Deletion
- Signed types
