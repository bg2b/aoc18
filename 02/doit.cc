// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

bool exactly(string s, unsigned repeats) {
  sort(s.begin(), s.end());
  for (size_t i = 0; i < s.length(); ) {
    size_t first = i;
    char c = s[i++];
    while (i < s.length() && c == s[i])
      ++i;
    if (i - first == repeats)
      return true;
  }
  return false;
}

void part1() {
  string id;
  int exactly_twice = 0;
  int exactly_thrice = 0;
  while (cin >> id) {
    if (exactly(id, 2))
      ++exactly_twice;
    if (exactly(id, 3))
      ++exactly_thrice;
  }
  cout << exactly_twice * exactly_thrice << '\n';
}

string common(string const &s1, string const &s2) {
  string result;
  if (s1.length() == s2.length())
    for (size_t i = 0; i < s1.length(); ++i)
      if (s1[i] == s2[i])
        result.push_back(s1[i]);
  return result;
}

void part2() {
  vector<string> ids;
  string id;
  while (cin >> id) {
    for (auto const &other : ids) {
      auto overlap = common(id, other);
      if (overlap.length() + 1 == id.length()) {
        cout << overlap << '\n';
        return;
      }
    }
    ids.push_back(id);
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "usage: " << argv[0] << " partnum < input\n";
    exit(1);
  }
  if (*argv[1] == '1')
    part1();
  else
    part2();
  return 0;
}
