#include <radix_cpp.h>

#include <iostream>

int main(void) {
  radix_cpp::set<unsigned short> S;
  S.insert(10);
  S.insert(1255);
  for (auto & v : S) {
    std::cout << v << "\n";
  }

  radix_cpp::map<unsigned short, std::string> M;
  M.insert(std::make_pair(1, "Hello"));
  M.insert(std::make_pair(2, "World"));

  for (auto & [ key, value ] : M) {
    std::cout << key << " = " << value << "\n";
  }
  return 0;
}
