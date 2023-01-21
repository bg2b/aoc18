// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <array>
#include <cassert>

using namespace std;

using point = array<int, 4>;

int manhattan(point const &p1, point const &p2) {
  return (abs(p1[0] - p2[0]) + abs(p1[1] - p2[1]) +
          abs(p1[2] - p2[2]) + abs(p1[3] - p2[3]));
}

vector<point> read() {
  vector<point> result;
  char _;
  point p;
  while (cin >> p[0] >> _ >> p[1] >> _ >> p[2] >> _ >> p[3])
    result.push_back(p);
  return result;
}

void part1() {
  auto pts = read();
  int n = pts.size();
  // Standard union/find approach
  vector<int> uf(n);
  for (int i = 0; i < n; ++i)
    uf[i] = i;
  auto find = [&](int element) {
                int rep = element;
                while (uf[rep] != rep)
                  rep = uf[rep];
                uf[element] = rep;
                return rep;
              };
  auto onion = [&](int element1, int element2) {
                 int rep1 = find(element1);
                 int rep2 = find(element2);
                 if (rep1 <= rep2)
                   uf[rep2] = rep1;
                 else
                   uf[rep1] = rep2;
               };
  for (int i = 0; i < n; ++i)
    for (int j = i + 1; j < n; ++j)
      if (manhattan(pts[i], pts[j]) <= 3)
        onion(i, j);
  int num_constellations = 0;
  for (int i = 0; i < n; ++i)
    if (find(i) == i)
      ++num_constellations;
  cout << num_constellations << '\n';
}

void part2() {
  cout << "Trigger the Underflow!\n";
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
