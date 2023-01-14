// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <optional>
#include <algorithm>
#include <cassert>

using namespace std;

using coords = pair<int, int>;
using point = pair<coords, coords>;

coords operator-(coords const &c1, coords const &c2) {
  return { c1.first - c2.first, c1.second - c2.second };
}

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

vector<point> read() {
  auto erase = [](string const &s, string const &pattern) {
                 auto pos = s.find(pattern);
                 assert(pos != string::npos);
                 return s.substr(0, pos) + s.substr(pos + pattern.length());
               };
  vector<point> result;
  string line;
  while (getline(cin, line)) {
    line = erase(line, "position=<");
    line = erase(line, "velocity=<");
    stringstream ss(line);
    char _;
    int x, y, vx, vy;
    ss >> x >> _ >> y >> _ >> vx >> _ >> vy;
    result.emplace_back(make_pair(x, y), make_pair(vx, vy));
  }
  return result;
}

// The idea is that when alignment occurs, points are grouped together
// to make characters.  Some pairs of points will become horizontally
// adjacent.  candidate_time see if there's a t where pt1 and pt2 are
// side-by-side (with pt1 on the left).  The function is called with
// both orders of pt1 and pt2, so if the two points will ever align
// side-by-side, that time will be found.
optional<int> candidate_time(point const &pt1, point const &pt2) {
  auto [p1, v1] = pt1;
  auto [p2, v2] = pt2;
  // How much x displacement has to occur?
  int dx = p2.first - (p1.first + 1);
  // How fast is the x distance changing?
  int vx = v2.first - v1.first;
  if (vx == 0)
    // Distance unchanging
    return nullopt;
  int t = -dx / vx;
  if (t < 0)
    // In the past
    return nullopt;
  if (p1 + t * v1 + coords({ 1, 0 }) != p2 + t * v2)
    // Not exactly aligned (i.e., vx didn't evenly divide dx)
    return nullopt;
  return t;
}

// See if the points make a good constellation at time t, and
// optionally display the constellation.
bool check_alignment(vector<point> const &pts, int t, bool print) {
  // Where are the points at time t?
  set<coords> pts_at_t;
  for (auto const &pt : pts) {
    auto pt_at_t = pt.first + t * pt.second;
    pts_at_t.insert(pt_at_t);
  }
  // Check for isolated points at that time
  for (auto const &pt_at_t : pts_at_t) {
    int neighbors = 0;
    for (int dx = -1; dx <= 1; ++dx)
      for (int dy = -1; dy <= 1; ++dy)
        if (pts_at_t.count(pt_at_t + coords({ dx, dy })))
          ++neighbors;
    if (neighbors < 2)
      // The point itself is counted, so non-isolated is 2+
      return false;
  }
  if (print) {
    coords ll = *pts_at_t.begin();
    coords ur = ll;
    for (auto const &pt_at_t : pts_at_t) {
      ll = min(ll, pt_at_t);
      ur = max(ur, pt_at_t);
    }
    for (int y = ll.second; y <= ur.second; ++y) {
      for (int x = ll.first; x <= ur.first; ++x)
        cout << (pts_at_t.count({ x, y }) ? "@@" : "  ");
      cout << '\n';
    }
  }
  // Alignment found
  return true;
}

int align(bool print) {
  auto pts = read();
  // Find candidate alignment times, count how many times each
  // candidate is found
  map<int, int> candidates;
  for (size_t i = 0; i < pts.size(); ++i)
    for (size_t j = 0; j < pts.size(); ++j)
      if (i != j) {
        auto t = candidate_time(pts[i], pts[j]);
        if (t)
          ++candidates[*t];
      }
  // Most likely alignment times are those that are found the most
  vector<pair<int, int>> by_occurrences(candidates.begin(), candidates.end());
  sort(by_occurrences.begin(), by_occurrences.end(),
       [](auto const &i1, auto const &i2) {
         return i1.second < i2.second;
       });
  // Check in order of most likely
  optional<int> t_aligned;
  while (!by_occurrences.empty()) {
    int t = by_occurrences.back().first;
    by_occurrences.pop_back();
    if (check_alignment(pts, t, print)) {
      t_aligned = t;
      break;
    }
  }
  assert(t_aligned);
  return *t_aligned;
}

void part1() { align(true); }
void part2() { cout << align(false) << '\n'; }

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
