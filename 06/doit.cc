// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

using namespace std;

// I had originally done part 1 with a marking approach, but I don't
// see any way to do part 2 without a brute-force loop.  So given
// that, may as well use the same brute-force for part 1, since it's
// more transparent.

using coords = pair<int, int>;

coords operator-(coords const &c1, coords const &c2) {
  return { c1.first - c2.first, c1.second - c2.second };
}

coords min(coords const &c1, coords const &c2) {
  return { min(c1.first, c2.first), min(c1.second, c2.second) };
}

coords max(coords const &c1, coords const &c2) {
  return { max(c1.first, c2.first), max(c1.second, c2.second) };
}

int manhattan(coords const &c1, coords const &c2) {
  return abs(c1.first - c2.first) + abs(c1.second - c2.second);
}

enum status { boundary = -1, closest };

struct grid {
  // Original points
  vector<coords> coordinates;
  // Bounding box
  coords ll;
  coords ur;
  // dist_info[y][x] (with indexes offset by ll) is
  // { closest point (or boundary), total distance to all points }
  vector<vector<pair<int, int>>> dist_info;

  // Construct from stdin, compute dist_info
  grid();

  pair<int, int> distances(coords const &c) const;

  int part1() const;
  int part2() const;
};

grid::grid() {
  char comma;
  int x, y;
  while (cin >> x >> comma >> y)
    coordinates.emplace_back(x, y);
  assert(!coordinates.empty());
  ll = coordinates.front();
  ur = ll;
  for (auto const &c : coordinates) {
    ll = min(ll, c);
    ur = max(ur, c);
  }
  coords dxy = ur - ll;
  int w = dxy.first + 1;
  int h = dxy.second + 1;
  dist_info.resize(h, vector<pair<int, int>>(w, { 0, 0 }));
  for (int y = ll.second; y <= ur.second; ++y) {
    for (int x = ll.first; x <= ur.first; ++x) {
      coords xy{ x, y };
      int total_dist = manhattan(xy, coordinates[0]);
      int status = closest + 0;
      int min_dist = total_dist;
      for (size_t i = 1; i < coordinates.size(); ++i) {
        int disti = manhattan(xy, coordinates[i]);
        total_dist += disti;
        if (disti < min_dist) {
          min_dist = disti;
          status = closest + i;
        } else if (disti == min_dist)
          status = boundary;
      }
      dist_info[y - ll.second][x - ll.first] = { status, total_dist };
    }
  }
}

pair<int, int> grid::distances(coords const &c) const {
  coords c1 = c - ll;
  return dist_info[c1.second][c1.first];
}

int grid::part1() const {
  // Points whose index is on an edge have unbounded regions
  vector<int> areas(coordinates.size(), 0);
  for (int y = ll.second; y <= ur.second; ++y)
    for (int x = ll.first; x <= ur.first; ++x) {
      int index = distances({ x, y }).first;
      if (index == boundary)
        continue;
      assert(index >= closest);
      index -= closest;
      if (x == ll.first || x == ur.first || y == ll.second || y == ur.second)
        // Reached edge
        areas[index] = -1;
      if (areas[index] < 0)
        // Reached edge
        continue;
      ++areas[index];
    }
  return *max_element(areas.begin(), areas.end());
}

int grid::part2() const {
  int result = 0;
  for (int y = ll.second; y <= ur.second; ++y)
    for (int x = ll.first; x <= ur.first; ++x)
      if (distances({ x, y }).second < 10000)
        ++result;
  return result;
}

void part1() { cout << grid().part1() << '\n'; }
void part2() { cout << grid().part2() << '\n'; }

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
