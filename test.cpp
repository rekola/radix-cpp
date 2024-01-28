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

  radix_cpp::set<uint16_t> S2;
  S2.insert(1000);
  S2.insert(1001);
  S2.insert(1);
  S2.insert(30000);
  
  return 0;
}
