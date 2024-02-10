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

  constexpr uint32_t n_vals = 1000000;
  
  for (uint32_t i = 0; i < n_vals; i++) {
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

  REQUIRE(next_consecutive_element == n_vals);
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
  S.insert("abc");
  S.insert("ab");
  S.insert("a");
  S.insert("");
  REQUIRE(S.size() == 5);
  auto it = S.begin();
  REQUIRE(*it++ == "");
  REQUIRE(*it++ == "@");
  REQUIRE(*it++ == "a");
  REQUIRE(*it++ == "ab");
  REQUIRE(*it++ == "abc");
  REQUIRE(it == S.end());
}

#if 0
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
#endif

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

TEST_CASE( "updating iterators", "[iter_update]" ) {
  radix_cpp::map<std::string, int> M;
  M["A"] = 1;
  auto it = M.find("A");
  REQUIRE(it->second == 1);
  it->second = 2;
  REQUIRE(M["A"] == 2);
}

TEST_CASE( "updating inserted", "[inserted_update]" ) {
  radix_cpp::map<std::string, int> M;
  auto [ it, is_new ] = M.insert(std::pair("Hello", 10));
  REQUIRE(M["Hello"] == 10);
  it->second = 11;
  REQUIRE(M["Hello"] == 11);
}

TEST_CASE( "map assignment", "[assignment]" ) {
  radix_cpp::map<std::string, bool> M;
  M["Hello"] = true;
  REQUIRE(M["Hello"] == true);
  REQUIRE(M.size() == 1);
}

TEST_CASE( "move construction and assign", "[move_construct_assign]" ) {
  radix_cpp::set<std::string> S0;
  S0.insert("AB");
  S0.insert("ABC");
  REQUIRE(S0.size() == 2);
  auto S1(std::move(S0));
  auto S2 = std::move(S1);
  REQUIRE(S0.size() == 0);
  REQUIRE(S1.size() == 0);
  REQUIRE(S2.size() == 2);
}

TEST_CASE( "insert doesn't overwrite", "[insert_no_overwrite]" ) {
  radix_cpp::map<std::string, int> M;
  auto [ it1, inserted1 ] = M.insert(std::pair("abc", 1));
  auto [ it2, inserted2 ] = M.insert(std::pair("ab", 1));
  auto [ it3, inserted3 ] = M.insert(std::pair("ab", 2));
  REQUIRE(it1->second == 1);
  REQUIRE(inserted1 == true);
  REQUIRE(it2->second == 1);
  REQUIRE(inserted2 == true);
  REQUIRE(it3->second == 1);
  REQUIRE(inserted3 == false);
  REQUIRE(M.size() == 2);
}

TEST_CASE( "insert with move semantics", "[insert_move]") {
  radix_cpp::set<std::string> S;
  std::string s = "This string is very long!!!!!!!!!!";
  S.insert(std::move(s));
  REQUIRE(s.empty());
  REQUIRE(S.size() == 1);
  auto it = S.begin();
  REQUIRE(*it++ == "This string is very long!!!!!!!!!!");
  REQUIRE(it == S.end());
}

TEST_CASE( "emplace with set", "[emplace_set]") {
  radix_cpp::set<std::string> S;
  S.emplace("a string");
  S.emplace("another string");
  auto it = S.begin();
  REQUIRE(*it++ == "a string");
  REQUIRE(*it++ == "another string");
  REQUIRE(it == S.end());
}

TEST_CASE( "emplace with map", "[emplace_map]") {
  radix_cpp::map<std::string, bool> M;
  M.emplace("a string", true);
  M.emplace("another string", true);
  REQUIRE(M["a string"] == true);
  REQUIRE(M["another string"] == true);
}

TEST_CASE( "at works correctly", "[at]") {
  radix_cpp::map<std::string, int> M;
  M.emplace("s1", 1);
  REQUIRE(M.at("s1") == 1);
  REQUIRE_THROWS_AS( M.at("s2"), std::out_of_range);
}

TEST_CASE( "insert_or_assign works", "[insert_or_assign]") {
  radix_cpp::map<std::string, int> M;
  M.insert_or_assign("key", 1);
  REQUIRE(M["key"] == 1);
  M.insert_or_assign("key", 2);
  REQUIRE(M["key"] == 2);
}

TEST_CASE( "iterator comparison works", "[iterator_cmp]") {
  radix_cpp::set<uint32_t> S;
  S.insert(1);
  S.insert(2);
  S.insert(3);
  auto it = S.begin();
  REQUIRE(it == S.begin());
  REQUIRE(it != S.end());
  REQUIRE(it == S.find(1));
  REQUIRE(it != S.find(2));
  REQUIRE(it != S.find(3));

  it++;
  
  REQUIRE(it != S.begin());
  REQUIRE(it != S.end());
  REQUIRE(it != S.find(1));
  REQUIRE(it == S.find(2));
  REQUIRE(it != S.find(3));

  it++;
  
  REQUIRE(it != S.begin());
  REQUIRE(it != S.end());
  REQUIRE(it != S.find(1));
  REQUIRE(it != S.find(2));
  REQUIRE(it == S.find(3));

  it++;

  REQUIRE(it == S.end());
}

TEST_CASE( "utf8 strings work", "[utf8]" ) {
  radix_cpp::set<std::string> S;
  S.insert("Ångström");
  S.insert("Å");
  S.insert("A");
  auto it = S.begin();
  REQUIRE(*it++ == "A");
  REQUIRE(*it++ == "Å");
  REQUIRE(*it++ == "Ångström");
}

TEST_CASE( "erase with set", "[erase with set]" ) {
  radix_cpp::set<std::string> S;
  S.insert("k1");
  S.insert("k2");

  REQUIRE(S.size() == 2);
  REQUIRE(S.find("k1") != S.end());
  REQUIRE(S.find("k2") != S.end());

  auto it = S.begin();
  REQUIRE(*it == "k1");
  it = S.erase(it);
  REQUIRE(S.size() == 1);
  REQUIRE(*it == "k2");
  it = S.erase(it);
  REQUIRE(S.size() == 0);
  REQUIRE(it == S.end());
  REQUIRE(S.begin() == S.end());

  REQUIRE(S.find("k1") == S.end());
  REQUIRE(S.find("k2") == S.end());
}
