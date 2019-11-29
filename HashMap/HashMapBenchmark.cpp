#include <iostream>
#include <chrono>
#include <random>
#include <unordered_map>
#include <fstream>

#include "HashMap.h"

using namespace std::chrono;

constexpr size_t count = 1000;
constexpr size_t iters = 10001;


template <class T>
void insertTimestamp(const char *n, T &m, std::ofstream &file) {
	auto start = high_resolution_clock::now();
	for (size_t i = 1; i < iters; ++i) {

		m.insert({ i, i });
	}
	auto stop = high_resolution_clock::now();
	auto duration = stop - start;

	file << n << ": "
		<< duration_cast<nanoseconds>(duration).count() / iters << " insert ns/iter" << std::endl;
}

template <class T>
void findTimestamp(const char *n, T &m, std::ofstream &file) {
	auto start = high_resolution_clock::now();
	for (size_t i = 1; i < iters; ++i) {
		m.find(i);
	}
	auto stop = high_resolution_clock::now();
	auto duration = stop - start;

	file << n << ": "
		<< duration_cast<nanoseconds>(duration).count() / iters << " find ns/iter" << std::endl;
}

template <class T>
void eraseTimestamp(const char *n, T &m, std::ofstream &file) {
	auto start = high_resolution_clock::now();

	for (size_t i = 1; i < iters; ++i) {
		m.erase(i);
	}

	auto stop = high_resolution_clock::now();
	auto duration = stop - start;

	file << n << ": "
		<< duration_cast<nanoseconds>(duration).count() / iters << " erase ns/iter" << std::endl;
}


int main(int argc, char *argv[]) {
  std::ofstream profile("profile.txt");

  {
    HashMap<int, int> hm(iters, 0);
	insertTimestamp<HashMap<int, int>>("HashMap", hm, profile);
	findTimestamp<HashMap<int, int>>("HashMap", hm, profile);
	eraseTimestamp<HashMap<int, int>>("HashMap", hm, profile);
  }


  {
    std::unordered_map<int, int> hm(iters);
	insertTimestamp<std::unordered_map<int, int>>("std::unordered_map", hm, profile);
	findTimestamp<std::unordered_map<int, int>>("std::unordered_map", hm, profile);
	eraseTimestamp<std::unordered_map<int, int>>("std::unordered_map", hm, profile);
  }

  profile.close();

  return 0;
}
