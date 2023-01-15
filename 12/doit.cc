// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <set>
#include <cassert>

using namespace std;

struct pots {
  // Number of the leftmost represented pot
  int leftmost{0};
  // What's growing in the represented pots
  string state;
  // Rules that produce plants
  set<string> fertile;

  // Construct from stdin
  pots();

  // Run one generation.  Return true if a repeat of the state is
  // detected.  (The value of leftmost may change however, i.e., the
  // state can be shifting along.)
  bool grow();

  // Do the growing pot number sum thing
  int plant_pots() const;
};

pots::pots() {
  string _;
  cin >> _ >> _ >> state;
  string pattern;
  char next;
  while (cin >> pattern >> _ >> next) {
    assert(pattern.length() == 5);
    assert(next == '#' || next == '.');
    if (next == '#')
      fertile.insert(pattern);
  }
  assert(!fertile.count("....."));
}

bool pots::grow() {
  // Make sure there's room to check the rules
  string extended = "..." + state + "...";
  leftmost -= 3;
  // next is overwritten with the new state
  string next = extended;
  for (size_t i = 2; i + 2 < next.length(); ++i)
    // Apply rules
    next[i] = fertile.count(extended.substr(i - 2, 5)) ? '#' : '.';
  // Trim as much as possible
  while (!next.empty() && next.back() == '.')
    next.pop_back();
  auto pos = next.find('#');
  assert(pos != string::npos);
  next = next.substr(pos);
  // Adjust leftmost and note if the state is unchanged
  bool repeating = state == next;
  state = next;
  leftmost += pos;
  return repeating;
}

int pots::plant_pots() const {
  int result = 0;
  for (size_t i = 0; i < state.length(); ++i)
    if (state[i] == '#')
      result += i + leftmost;
  return result;
}

void part1() {
  pots p;
  for (int _ = 0; _ < 20; ++_)
    p.grow();
  cout << p.plant_pots() << '\n';
}

void part2() {
  pots p;
  // Get to a repeating state
  long remaining = 50000000000;
  while (remaining-- > 0 && !p.grow())
    ;
  assert(remaining > 0);
  // Do one more grow cycle to see how plant_pots() changes
  long pots1 = p.plant_pots();
  --remaining;
  p.grow();
  long pots2 = p.plant_pots();
  // Scale to extend for the remaining generations
  cout << pots2 + (pots2 - pots1) * remaining << '\n';
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
