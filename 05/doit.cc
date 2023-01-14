// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <cctype>
#include <cassert>

using namespace std;

bool react(char u1, char u2) {
  return u1 != u2 && tolower(u1) == tolower(u2);
}

string react(string const &polymer) {
  string result;
  for (char u : polymer)
    if (!result.empty() && react(result.back(), u))
      result.pop_back();
    else
      result.push_back(u);
  return result;
}

string strip(string const &polymer, char drop) {
  string result;
  for (char u : polymer)
    if (tolower(u) != drop)
      result.push_back(u);
  return result;
}

void part1() {
  string polymer;
  cin >> polymer;
  cout << react(polymer).size() << '\n';
}

void part2() {
  string polymer;
  cin >> polymer;
  size_t min_length = polymer.length();
  for (char c = 'a'; c <= 'z'; ++c)
    min_length = min(min_length, react(strip(polymer, c)).length());
  cout << min_length << '\n';
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
