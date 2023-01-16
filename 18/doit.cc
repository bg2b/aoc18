// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <string>
#include <cassert>

using namespace std;

struct landscape {
  vector<string> acres;

  // Construct from stdin
  landscape();

  // Count number of neighbors of a given type
  int neighboring(int i, int j, char type) const;

  // Run one step of the evolution
  void evolve();

  // Compute the resource value
  int summary() const;
};

landscape::landscape() {
  string row;
  while (getline(cin, row)) {
    acres.push_back(row);
    assert(row.length() == acres.front().length());
  }
  assert(!acres.empty());
  assert(acres.size() == acres.front().size());
}

int landscape::neighboring(int i, int j, char type) const {
  int n = acres.size();
  int result = 0;
  for (int ii = i - 1; ii <= i + 1; ++ii)
    for (int jj = j - 1; jj <= j + 1; ++jj) {
      if (ii == i && jj == j) continue;
      if (ii < 0 || ii >= n || jj < 0 || jj >= n) continue;
      result += acres[ii][jj] == type ? 1 : 0;
    }
  return result;
}

void landscape::evolve() {
  auto next_acres = acres;
  int n = acres.size();
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      char c = acres[i][j];
      char next = c;
      if (c == '.' && neighboring(i, j, '|') >= 3)
        next = '|';
      if (c == '|' && neighboring(i, j, '#') >= 3)
        next = '#';
      if (c == '#' && !(neighboring(i, j, '#') >= 1 &&
                        neighboring(i, j, '|') >= 1))
        next = '.';
      next_acres[i][j] = next;
    }
  acres = next_acres;
}

int landscape::summary() const {
  int num_wooded = 0;
  int num_lumberyards = 0;
  int n = acres.size();
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      char c = acres[i][j];
      if (c == '|') ++num_wooded;
      if (c == '#') ++num_lumberyards;
    }
  return num_wooded * num_lumberyards;
}

void part1() {
  landscape area;
  for (int _ = 0; _ < 10; ++_)
    area.evolve();
  cout << area.summary() << '\n';
}

void part2() {
  landscape slow;
  auto fast = slow;
  int const total_t = 1000000000;
  int t = 0;
  // fast evolves at twice the rate of slow
  while (t < total_t) {
    slow.evolve();
    fast.evolve();
    fast.evolve();
    ++t;
    if (slow.acres == fast.acres)
      break;
  }
  // This isn't generally the minimal cycle if there's an initial
  // transient, but it is some cycle
  int cycle_length = 2 * t - t;
  int whole_cycles = (total_t - t) / cycle_length;
  t += whole_cycles * cycle_length;
  while (t < total_t) {
    slow.evolve();
    ++t;
  }
  cout << slow.summary() << '\n';
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
