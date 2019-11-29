#include <iostream>
#include <string>
#include "HashMap.h"


int main(int argc, char *argv[]) {
  // Stupid hash for using std::string as lookup key
  struct Hash {
    size_t operator()(int v) { return v + 7; }
	size_t operator()(const std::string &v) { return std::stoi(v) + 7; }
  };

  // Simple equal comparison for using std::string as lookup key
  struct Equal {
    bool operator()(int lhs, int rhs) { return lhs == rhs; }
    bool operator()(int lhs, const std::string &rhs) {
      return lhs == std::stoi(rhs);
    }
  };

  HashMap<int, int, Hash, Equal> hashMap(1, 0);
  hashMap.emplace(1, 1);
  hashMap.emplace(8, 2);
  hashMap[3] = 3;

  // Iterate and print key-value pairs
  for (const auto &el : hashMap) {
    std::cout << el.first << " = " << el.second << "\n";
  }

  // Lookup using std::string
  std::cout << hashMap.at("1") << "\n";

  // Erase entry
  hashMap.erase(1);

  return 0;
}
