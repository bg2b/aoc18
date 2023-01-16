// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <map>
#include <algorithm>
#include <cassert>

using namespace std;

using coords = pair<int, int>;

coords operator+(coords const &c1, coords const &c2) {
  return { c1.first + c2.first, c1.second + c2.second };
}

coords operator*(int sc, coords const &c) {
  return { sc * c.first, sc * c.second };
}

coords min(coords const &c1, coords const &c2) {
  return { min(c1.first, c2.first), min(c1.second, c2.second) };
}

coords max(coords const &c1, coords const &c2) {
  return { max(c1.first, c2.first), max(c1.second, c2.second) };
}

struct scan {
  // Non-sand
  map<coords, char> ground;
  // Bounds
  coords ll;
  coords ur;

  // Construct from stdin
  scan();

  char at(coords const &xy) const;

  // Fill with water starting at xy, return true if the water managed
  // to reach below the bottom (infinity), false if this area filled
  bool fill(coords const &xy);

  // Count water, if flowing then | and ~, else just ~
  size_t water(bool flowing) const;
};

scan::scan() {
  char d1, d2, eq, dot, cm;
  int v1, v2a, v2b;
  while (cin >> d1 >> eq >> v1 >> cm >> d2 >> eq >> v2a >> dot >> dot >> v2b) {
    assert((d1 == 'x' && d2 == 'y') || (d1 == 'y' && d2 == 'x'));
    if (v2a > v2b)
      swap(v2a, v2b);
    for (int v2 = v2a; v2 <= v2b; ++v2) {
      coords xy;
      if (d1 == 'x')
        xy = { v1, v2 };
      else
        xy = { v2, v1 };
      if (ground.empty()) {
        ll = xy;
        ur = xy;
      } else {
        ll = min(ll, xy);
        ur = max(ur, xy);
      }
      ground.emplace(xy, '#');
    }
  }
}

char scan::at(coords const &xy) const {
  auto p = ground.find(xy);
  if (p != ground.end())
    return p->second;
  return '.';
}

bool scan::fill(coords const &xy) {
  if (xy.second > ur.second)
    // Reached infinity
    return true;
  ground[xy] = '|';
  auto below = xy + coords{ 0, 1 };
  char c = at(below);
  if (c == '|')
    // Reached some other water that reached infinity
    return true;
  if (c == '.' && fill(below))
    // Reached infinity
    return true;
  // Either below is clay, or what was below filled up; scan
  // left/right
  assert(at(below) == '~' || at(below) == '#');
  bool reached_inf = false;
  // left_right scans for an edge of the horizontal span of flow,
  // returns dx for the edge location
  auto left_right =
    [&](int dir) {
      for (int dx = dir; ; dx += dir) {
        auto horiz = xy + coords{ dx, 0 };
        char hc = at(horiz);
        if (hc == '|') {
          // Reached some other water that reached infinity
          reached_inf = true;
          return dx - dir;
        }
        if (hc == '#')
          // You shall not pass!
          return dx - dir;
        assert(hc == '.');
        // The water might be able to go down to infinity here, in
        // which case it won't go horizontally anymore
        auto below_horiz = horiz + coords{ 0, 1 };
        if (at(below_horiz) == '.') {
          // Try to fill the thing below...
          if (fill(below_horiz)) {
            reached_inf = true;
            // Water stops flowing horizontally here
            return dx;
          }
        }
        // Else keep scanning for the edge
      }
    };
  int left = left_right(-1);
  int right = left_right(+1);
  // Everything from dx = left to right is the same, either | or ~
  char mark = reached_inf ? '|' : '~';
  for (int dx = left; dx <= right; ++dx)
    ground[xy + coords{ dx, 0 }] = mark;
  return reached_inf;
}

size_t scan::water(bool flowing) const {
  return count_if(ground.begin(), ground.end(),
                  [=](auto const &p) {
                    return !flowing ? (p.second == '~') : (p.second != '#');
                  });
}

void solve(bool flowing) {
  scan sc;
  sc.fill({ 500, sc.ll.second });
  cout << sc.water(flowing) << '\n';
}

void part1() { solve(true); }
void part2() { solve(false); }

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
