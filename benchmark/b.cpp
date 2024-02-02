#include <radix_cpp.h>

#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <random>
#include <algorithm>
#include <random>

#include <sys/time.h>
#include <time.h>

static std::vector<uint32_t> make_test_data(int n) {
  std::vector<uint32_t> v;
  for (int i = 0; i < n; i++) v.push_back(static_cast<uint32_t>(i));
  
  auto rng = std::default_random_engine {};
  std::shuffle(std::begin(v), std::end(v), rng);

  return v;
}

static double get_wall_time() {
  struct timeval time;
  if (gettimeofday(&time,NULL)){
    //  Handle error
    return 0;
  }
  return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

int main() {
  std::map<int, std::pair<double, double> > results;

  int runs = 1;
  for (int run = 0; run < runs; run++) {
    std::cerr << "run " << run << "\n";
    for (int n = 1000000; n <= 10000000; n += 500000) {
      std::cerr << "  test " << n << "\n";
      auto v = make_test_data(n);
      
      double t0 = get_wall_time();
      
      {
	radix_cpp::set<uint32_t> S1;
	for (auto & a : v) {
	  S1.insert(a);
	}
      }
      
      double t1 = get_wall_time();
      
      {
	std::set<uint32_t> S2;
	for (auto & a : v) {
	  S2.insert(a);
	}
      }
      
      double t2 = get_wall_time();
      
      results[n] = std::pair(t1 - t0, t2 - t1);
    }
  }

  for (auto & [ n, d ] : results) {
    double t1 = d.first / runs;
    double t2 = d.second / runs;
    std::cout << n << ";" << t1 << ";" << t2 << "\n";
  }

  return 0;
}
