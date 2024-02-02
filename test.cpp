#include <radix_cpp.h>

#include <iostream>

int main(void) {
  radix_cpp::set<uint8_t> S0;
  for (auto & v : S0) {
    std::cerr << (int)v << "\n";
  }

  radix_cpp::set<uint8_t> S;
  S.insert(65);
  S.insert(125);
  for (auto & v : S) {
    std::cerr << (int)v << "\n";
  }

  radix_cpp::map<uint32_t, std::string> M;
  M.insert(std::make_pair(65536, "!!"));
  M.insert(std::make_pair(256, "World"));
  M[9999] = "Nice!";
  M.insert(std::make_pair(0, "Hello"));
  
  for (auto & [ key, value ] : M) {
    std::cerr << (int)key << " = " << value << "\n";
  }

  std::cerr << "S2\n";
  radix_cpp::set<uint16_t> S2;
  S2.insert(1000);
  S2.insert(1001);
  S2.insert(29999);
  S2.insert(30000);
  for (auto & v : S2) {
    std::cerr << v << "\n";
  }

  std::cerr << "S3\n";
  radix_cpp::set<uint32_t> S3;
  S3.insert(1000000000);
  S3.insert(100000000);
  S3.insert(10000000);
  S3.insert(1000000);
  S3.insert(100000);
  S3.insert(10000);
  S3.insert(1000);
  S3.insert(100);
  S3.insert(10);
  S3.insert(1);

  for (auto & v : S3) {
    std::cerr << v << "\n";
  }

  auto it = S3.find(1000);
  if (it != S3.end()) {
    std::cerr << "1000 = " << *it << "\n";
  }

  radix_cpp::set<uint32_t> S4;
  S4.insert(1000);
  auto it2 = S4.find(1000);
  std::cerr << "*it2 = " << *it2 << "\n";
  for (uint32_t i = 0; i < 1000000; i++) {
    S4.insert(i);
  }
  std::cerr << "*it = " << *it2 << "\n";
  std::cerr << "collisions per insert: "
	    << (static_cast<double>(S4.num_insert_collisions()) / static_cast<double>(S4.num_inserts()))
	    << "\n";
  
  size_t n = 0;
  for (auto & v : S4) {
    if (n != v) {
      std::cerr << "error, n = " << n << ", v = " << v << "\n";
      exit(1);
    }
    n++;
  }
  std::cerr << "S4, size = " << S4.size() << ", calc size = " << n << "\n";

  radix_cpp::set<std::string> S5;
  S5.insert("Hello World");
  S5.insert("c");
  S5.insert("b");
  S5.insert("a");
  S5.insert("A");
  S5.insert("B");
  S5.insert("Z");
  S5.insert("ab");
  S5.insert("abc");
  S5.insert("abcd");
  S5.insert("@");
  S5.insert("");

  std::cerr << "trying to find empty string\n";
  auto it3 = S5.find("");
  if (it3 == S5.end()) {
    std::cerr << "zero length element not found\n";
    exit(1);
  }

  std::cerr << "iterating string\n";
  for (auto & v : S5) {
    std::cerr << "string: " << v << "\n";
  }

  return 0;
}
