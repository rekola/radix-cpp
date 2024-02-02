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
  std::map<int, std::tuple<double, double, double, double, double, double, double, double, double> > results;

  int runs = 5;
  for (int run = 0; run < runs; run++) {
    std::cerr << "run " << run << "\n";
    for (int n = 1000000; n <= 10000000; n += 500000) {
      std::cerr << "  test " << n << "\n";
      auto v = make_test_data(n);
      
      double t_a0, t_a1, t_a2;
      double t_b0, t_b1, t_b2;
      double t_c0, t_c1, t_c2;
      double sum_a = 0, sum_b = 0, sum_c = 0;
      
      {
	std::set<uint32_t> S1;
	t_a0 = get_wall_time();
	for (auto & a : v) {
	  S1.insert(a);
	}
	t_a1 = get_wall_time();
	for (auto & a : S1) {
	  sum_a += a;
	}
	t_a2 = get_wall_time();
      }

      {
	std::vector<uint32_t> S2;
	t_b0 = get_wall_time();
	for (auto & a : v) {
	  S2.push_back(a);
	}
	std::sort(S2.begin(), S2.end());
	t_b1 = get_wall_time();
	for (auto & a : S2) {
	  sum_b += a;
	}
	t_b2 = get_wall_time();
      }

      {
	radix_cpp::set<uint32_t> S3;
	t_c0 = get_wall_time();
	for (auto & a : v) {
	  S3.insert(a);
	}
	t_c1 = get_wall_time();
	for (auto & a : S3) {
	  sum_c += a;
	}
	t_c2 = get_wall_time();
      }

      auto & r = results[n];
      std::get<0>(r) += t_a1 - t_a0;
      std::get<1>(r) += t_a2 - t_a1;
      std::get<2>(r) += sum_a;
      
      std::get<3>(r) += t_b1 - t_b0;
      std::get<4>(r) += t_b2 - t_b1;
      std::get<5>(r) += sum_b;
      
      std::get<6>(r) += t_c1 - t_c0;
      std::get<7>(r) += t_c2 - t_c1;
      std::get<8>(r) += sum_c;
    }
  }

  for (auto & [ n, d ] : results) {
    double da0 = std::get<0>(d) / runs;
    double da1 = std::get<1>(d) / runs;
    double Sa = std::get<2>(d) / runs;

    double db0 = std::get<3>(d) / runs;
    double db1 = std::get<4>(d) / runs;
    double Sb = std::get<5>(d) / runs;

    double dc0 = std::get<6>(d) / runs;
    double dc1 = std::get<7>(d) / runs;
    double Sc = std::get<8>(d) / runs;
    
    std::cout << n << ";" << da0 << ";" << da1 << ";" << db0 << ";" << db1 << ";" << dc0 << ";" << dc1 << ";" << Sa << ";" << Sb << ";" << Sc << "\n";
  }

  return 0;
}
