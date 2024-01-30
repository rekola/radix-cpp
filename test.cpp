#include <radix_cpp.h>

#include <iostream>

int main(void) {
  radix_cpp::set<uint8_t> S;
  S.insert(10);
  S.insert(125);
  for (auto & v : S) {
    std::cout << (int)v << "\n";
  }

  radix_cpp::map<uint8_t, std::string> M;
  M.insert(std::make_pair(1, "Hello"));
  M.insert(std::make_pair(2, "World"));

  for (auto & [ key, value ] : M) {
    std::cout << (int)key << " = " << value << "\n";
  }

  std::cout << "S2\n";
  radix_cpp::set<uint16_t> S2;
  S2.insert(1000);
  S2.insert(1001);
  S2.insert(29999);
  S2.insert(30000);
  for (auto & v : S2) {
    std::cout << v << "\n";
  }

  std::cout << "S3\n";
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
    std::cout << v << "\n";
  }

  radix_cpp::set<std::string> S4;
  S4.insert("Hello World");
  S4.insert("A");
  S4.insert("B");
  for (auto & v : S4) {
    std::cout << v << "\n";
  } 

  return 0;
}
