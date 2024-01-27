# radix-cpp

## Radix set and map implementation.

radix-cpp is an implementation of ordered set and map, which provides
very fast lookup using hash tables. Ordinarily hash tables are not
ordered, and while in theory, an order preserving hash function could
be used, it would lead to large number of collisions. In this
implementation, as in radix sort, the key is divided in multiple parts
called digits. When inserting an element, the most significant digit
is first mapped to an ordered table, and then used as an unordered
hash key in the second table to provide offset for the second digit.

Currently this implementation only works with unsigned integers, but
more support is forthcoming.
