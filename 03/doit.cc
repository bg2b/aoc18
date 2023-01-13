// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <cassert>

using namespace std;

pair<int, int> solve() {
  map<pair<int, int>, pair<int, int>> claimed;
  set<int> uncontested_claims;
  int contested = 0;
  char _;
  int num, left, top, wdth, hght;
  while (cin >> _ >> num >> _ >> left >> _ >> top >> _ >> wdth >> _ >> hght) {
    bool any_contested = false;
    for (int i = left; i < left + wdth; ++i)
      for (int j = top; j < top + hght; ++j) {
        auto & [total_claims, first_claim] = claimed[{ i, j }];
        if (++total_claims == 2) {
          uncontested_claims.erase(first_claim);
          ++contested;
        }
        if (total_claims == 1)
          first_claim = num;
        else
          any_contested = true;
      }
    if (!any_contested)
      uncontested_claims.insert(num);
  }
  assert(uncontested_claims.size() == 1);
  return { contested, *uncontested_claims.begin() };
}

void part1() { cout << solve().first << '\n'; }
void part2() { cout << solve().second << '\n'; }

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
