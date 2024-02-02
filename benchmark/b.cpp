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
  std::map<int, std::tuple<double, double, double, double> > results;

  int runs = 5;
  for (int run = 0; run < runs; run++) {
    std::cerr << "run " << run << "\n";
    for (int n = 1000000; n <= 10000000; n += 500000) {
      std::cerr << "  test " << n << "\n";
      auto v = make_test_data(n);
      
      double t_a0, t_a1, t_a2;
      double t_b0, t_b1, t_b2;
      
      {
	std::set<uint32_t> S1;
	t_a0 = get_wall_time();
	for (auto & a : v) {
	  S1.insert(a);
	}
	t_a1 = get_wall_time();
	int n = 0;
	for (auto & a : S1) {
	  n++;
	}
	t_a2 = get_wall_time();
      }
      
      {
	radix_cpp::set<uint32_t> S2;
	t_b0 = get_wall_time();
	for (auto & a : v) {
	  S2.insert(a);
	}
	t_b1 = get_wall_time();
	int n = 0;
	for (auto & a : S2) {
	  n++;
	}
	t_b2 = get_wall_time();
      }

      auto & r = results[n];
      std::get<0>(r) += t_a1 - t_a0;
      std::get<1>(r) += t_a2 - t_a1;
      std::get<2>(r) += t_b1 - t_b0;
      std::get<3>(r) += t_b2 - t_b1;
    }
  }

  for (auto & [ n, d ] : results) {
    double t0 = std::get<0>(d) / runs;
    double t1 = std::get<1>(d) / runs;
    double t2 = std::get<2>(d) / runs;
    double t3 = std::get<3>(d) / runs;
    std::cout << n << ";" << t0 << ";" << t1 << ";" << t2 << ";" << t3 << "\n";
  }

  return 0;
}
