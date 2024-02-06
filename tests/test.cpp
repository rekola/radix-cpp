#include <catch2/catch_test_macros.hpp>

#include "radix_cpp.h"

TEST_CASE( "simple integer sets can be created", "[int_set]" ) {
  radix_cpp::set<uint8_t> S0;
  S0.insert(0);
  S0.insert(255);
  auto it0 = S0.begin();
  REQUIRE(*it0++ == 0);
  REQUIRE(*it0++ == 255);
  REQUIRE(it0 == S0.end());

  radix_cpp::set<uint16_t> S1;
  S1.insert(0);
  S1.insert(1);
  S1.insert(2);
  auto it1 = S1.begin();
  REQUIRE(*it1++ == 0);
  REQUIRE(*it1++ == 1);
  REQUIRE(*it1++ == 2);
  REQUIRE(it1 == S1.end());

  radix_cpp::set<uint32_t> S2;
  S2.insert(1000000000);
  S2.insert(100000000);
  S2.insert(10000000);
  S2.insert(1000000);
  S2.insert(100000);
  S2.insert(10000);
  S2.insert(1000);
  S2.insert(100);
  S2.insert(10);
  S2.insert(1);
  S2.insert(0);

  auto it2 = S2.begin();
  REQUIRE(*it2++ == 0);
  REQUIRE(*it2++ == 1);
  REQUIRE(*it2++ == 10);
  REQUIRE(*it2++ == 100);
  REQUIRE(*it2++ == 1000);
  REQUIRE(*it2++ == 10000);
  REQUIRE(*it2++ == 100000);
  REQUIRE(*it2++ == 1000000);
  REQUIRE(*it2++ == 10000000);
  REQUIRE(*it2++ == 100000000);
  REQUIRE(*it2++ == 1000000000);
  REQUIRE(it2 == S2.end());
}

TEST_CASE( "large integer sets work and iterators are stable", "[large_set]") {
  radix_cpp::set<uint32_t> S;
  S.insert(1000);
  auto it0 = S.find(1000);
  
  for (uint32_t i = 0; i < 1000000; i++) {
    S.insert(i);
  }
  REQUIRE(*it0 == 1000);
  
#if 0
  std::cerr << "collisions per insert: "
	    << (static_cast<double>(S.num_insert_collisions()) / static_cast<double>(S.num_inserts()))
	    << "\n";
#endif
  
  uint32_t next_consecutive_element = 0;
  for (auto & v : S) {
    if (v != next_consecutive_element) {
      break;
    }
    next_consecutive_element = v + 1;
  }

  REQUIRE(next_consecutive_element == 1000000);
}

TEST_CASE( "simple string sets can be created", "[string_set]" ) {
  radix_cpp::set<std::string> S;
  S.insert("a");
  S.insert("bc");
  S.insert("def");
  auto it = S.begin();
  REQUIRE(*it++ == "a");
  REQUIRE(*it++ == "bc");
  REQUIRE(*it++ == "def");
  REQUIRE(it == S.end());
}

TEST_CASE( "empty string can be added to set and found", "[empty_string_set]" ) {
  radix_cpp::set<std::string> S;
  S.insert("abc");
  S.insert("");
  S.insert("gamma");
  auto it = S.begin();
  REQUIRE(*it++ == "");
  REQUIRE(*it++ == "abc");
  REQUIRE(*it++ == "gamma");
  REQUIRE(it == S.end());

  it = S.find("");
  REQUIRE(*it == "");
}

TEST_CASE( "shared prefixes work with strings", "[string_set_shared_prefix]" ) {
  radix_cpp::set<std::string> S;
  S.insert("@");
  S.insert("a");
  S.insert("ab");
  S.insert("abc");
  S.insert("abcd");
  S.insert("abcde");
  S.insert("abcdef");
  auto it = S.begin();
  REQUIRE(*it++ == "@");
  REQUIRE(*it++ == "a");
  REQUIRE(*it++ == "ab");
  REQUIRE(*it++ == "abc");
  REQUIRE(*it++ == "abcd");
  REQUIRE(*it++ == "abcde");
  REQUIRE(*it++ == "abcdef");
  REQUIRE(it == S.end());
}

TEST_CASE( "positive floating point keys", "[pos_float_set]") {
  radix_cpp::set<float> S;
  S.insert(1.1f);
  S.insert(1000.0f);
  S.insert(4.2f);
  S.insert(30000000.f);
  S.insert(0);
  auto it = S.begin();
  REQUIRE(*it++ == 0.0f);
  REQUIRE(*it++ == 1.1f);
  REQUIRE(*it++ == 4.2f);
  REQUIRE(*it++ == 1000.0f);
  REQUIRE(*it++ == 30000000.0f);
  REQUIRE(it == S.end());
}

TEST_CASE( "empty set or map iteration doesn't fail", "[empty]") {
  radix_cpp::set<uint8_t> S;
  radix_cpp::map<uint16_t, std::string> M;
  auto it1 = S.begin();
  auto it2 = M.begin();
  REQUIRE(it1 == S.end());
  REQUIRE(it2 == M.end());
}

TEST_CASE( "the iterator returned by insert is valid", "[inser_iterator]" ) {
  radix_cpp::set<uint32_t> S;
  S.insert(1000);
  S.insert(3000);
  auto [ it, is_new ] = S.insert(2000);
  REQUIRE(*it++ == 2000);
  REQUIRE(*it++ == 3000);
}

TEST_CASE( "empty map find", "[empty_find]" ) {
  radix_cpp::map<std::string, bool> M;
  auto it = M.find("Hello");
  REQUIRE(it == M.end());
}
