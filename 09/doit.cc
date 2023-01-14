// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

using num = long;
using iter = list<num>::iterator;

list<num> circle;
iter current;

iter clockwise(iter i) {
  if (++i == circle.end())
    i = circle.begin();
  return i;
}

iter countercw(iter i) {
  if (i == circle.begin())
    i = circle.end();
  return --i;
}

num move(num i) {
  if (i % 23 != 0) {
    current = circle.insert(clockwise(clockwise(current)), i);
    return 0;
  } else {
    for (int _ = 0; _ < 6; ++_)
      current = countercw(current);
    iter to_remove = countercw(current);
    num score = i + *to_remove;
    circle.erase(to_remove);
    return score;
  }
}

void play(bool x100) {
  string _;
  int num_players, last_marble;
  cin >> num_players >> _ >> _ >> _ >> _ >> _ >> last_marble >> _;
  if (x100)
    last_marble *= 100;
  circle.clear();
  circle.push_back(0);
  current = circle.begin();
  vector<num> scores(num_players, 0);
  for (num i = 0; i < last_marble; ++i)
    scores[i % num_players] += move(i + 1);
  cout << *max_element(scores.begin(), scores.end()) << '\n';
}

void part1() { play(false); }
void part2() { play(true); }

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
