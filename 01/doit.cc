// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <set>

using namespace std;

int freq = 0;
unsigned next_pos = 0;
vector<int> deltas;

void read() {
  int delta;
  while (cin >> delta)
    deltas.push_back(delta);
}

bool next() {
  freq += deltas[next_pos++];
  if (next_pos == deltas.size())
    next_pos = 0;
  return next_pos != 0;
}

void part1() {
  read();
  while (next())
    ;
  cout << freq << '\n';
}

void part2() {
  read();
  set<int> prev;
  while (!prev.count(freq)) {
    prev.insert(freq);
    next();
  }
  cout << freq << '\n';
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
