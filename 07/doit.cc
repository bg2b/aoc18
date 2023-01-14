// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <map>
#include <set>
#include <cassert>

using namespace std;

map<char, set<char>> needs;
set<char> steps;

void read() {
  string _;
  char before, after;
  while (cin >> _ >> before >> _ >> _ >> _ >> _ >> _ >> after >> _ >> _) {
    assert(before >= 'A' && before <= 'Z');
    assert(after >= 'A' && after <= 'Z');
    needs[after].insert(before);
    steps.insert(before);
    steps.insert(after);
  }
}

pair<string, int> assemble(int workers, int base_time) {
  read();
  // Time to do a given step
  auto time_for = [=](char step) { return base_time + step - 'A' + 1; };
  // When a step is completed, what steps might it enable?
  map<char, set<char>> enables;
  for (auto const & [step2, requirements] : needs)
    for (char step1 : requirements)
      enables[step1].insert(step2);
  // The steps that are ready
  set<char> ready = steps;
  for (auto const & [step, _] : needs)
    ready.erase(step);
  // Remaining steps needed
  auto remaining_needs = needs;
  // When workers will finish their current steps
  map<int, set<char>> finishing;
  // Does anyone know what time it is?
  int t = 0;
  // The order of finishing steps
  string order;
  while (!ready.empty() || !finishing.empty()) {
    if (workers > 0 && !ready.empty()) {
      // Start a step
      char step = *ready.begin();
      ready.erase(ready.begin());
      --workers;
      finishing[t + time_for(step)].insert(step);
      continue;
    }
    // Nothing is ready, or all workers are busy
    assert(!finishing.empty());
    auto [tnext, done] = *finishing.begin();
    finishing.erase(finishing.begin());
    // Time advances
    t = tnext;
    // Workers become free again
    workers += done.size();
    for (char step1 : done) {
      // Note steps as completed
      order.push_back(step1);
      // Enable next steps
      for (char step2 : enables[step1]) {
        remaining_needs[step2].erase(step1);
        if (remaining_needs[step2].empty())
          ready.insert(step2);
      }
    }
  }
  return { order, t };
}

void part1() { cout << assemble(1, 0).first << '\n'; }
void part2() { cout << assemble(5, 60).second << '\n'; }

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
