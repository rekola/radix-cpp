#include <radix_cpp.h>

#include <iostream>
#include <set>

int main(void) {
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
  for (uint32_t i = 0; i < 10000000; i++) {
    S4.insert(i);
  }
  std::cerr << "S4, size = " << S4.size() << "\n";
  
  radix_cpp::set<std::string> S5;
  S5.insert("Hello World");
  S5.insert("c");
  S5.insert("b");
  S5.insert("a");
  S5.insert("A");
  S5.insert("B");
  S5.insert("Z");
#if 0
  S5.insert("ab");
  S5.insert("abc");
  S5.insert("abcd");
#endif
  S5.insert("@");
  for (auto & v : S5) {
    std::cerr << v << "\n";
  }
  
  return 0;
}
