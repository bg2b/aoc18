// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <queue>
#include <algorithm>
#include <cassert>

using namespace std;

using coords = array<int, 3>;

coords operator-(coords const &c1, coords const &c2) {
  return { c1[0] - c2[0], c1[1] - c2[1], c1[2] - c2[2] };
}

coords min(coords const &c1, coords const &c2) {
  return { min(c1[0], c2[0]), min(c1[1], c2[1]), min(c1[2], c2[2]) };
}

coords max(coords const &c1, coords const &c2) {
  return { max(c1[0], c2[0]), max(c1[1], c2[1]), max(c1[2], c2[2]) };
}

int manhattan(coords const &c1, coords const &c2) {
  return abs(c1[0] - c2[0]) + abs(c1[1] - c2[1]) + abs(c1[2] - c2[2]);
}

struct bbox {
  coords ll, ur;

  bbox(coords const &ll_, coords const &ur_) : ll(ll_), ur(ur_) {}

  // Shift origin to some other coordinate
  bbox shift(coords const &c) const { return bbox(ll - c, ur - c); }

  // Manhattan distance of closest point in box to origin
  int closest_to_origin() const;
};

int bbox::closest_to_origin() const {
  int result = 0;
  for (int i = 0; i < 3; ++i)
    if (0 < ll[i])
      result += ll[i];
    else if (ur[i] < 0)
      result -= ur[i];
  return result;
}

bbox merge(bbox const &bb1, bbox const &bb2) {
  return bbox(min(bb1.ll, bb2.ll), max(bb1.ur, bb2.ur));
}

enum relationship { disjoint = 0, overlaps, contains };

struct nanobot {
  coords c;
  int radius;

  nanobot(string s);

  bool in_range(coords const &c1) const { return manhattan(c, c1) <= radius; }
  bool in_range(nanobot const &other) const { return in_range(other.c); }

  // Bounding box of points in range
  bbox bounds() const;

  // How do the points in bb relate to the bot?
  // disjoint => no points in the box are in range
  // contains => all points in the box are in range
  // overlaps => some but not all are in range
  relationship interaction(bbox const &bb) const;
};

nanobot::nanobot(string s) {
  auto erase = [](string const &s, string const &pattern) {
                 auto pos = s.find(pattern);
                 assert(pos != string::npos);
                 return s.substr(0, pos) + s.substr(pos + pattern.length());
               };
  s = erase(s, "pos=<");
  s = erase(s, ", r=");
  stringstream ss(s);
  char _;
  ss >> c[0] >> _ >> c[1] >> _ >> c[2] >> _ >> radius;
}

bbox nanobot::bounds() const {
  return bbox({ c[0] - radius, c[1] - radius, c[2] - radius },
              { c[0] + radius, c[1] + radius, c[2] + radius });
}

relationship nanobot::interaction(bbox const &bb) const {
  if (bb.shift(c).closest_to_origin() > radius)
    return disjoint;
  // There's some overlap; then all points are in iff the corners are
  // all in
  int num_in = 0;
  for (int i = 0; i < 8; ++i) {
    coords v;
    v[0] = (i & 1) ? bb.ll[0] : bb.ur[0];
    v[1] = (i & 2) ? bb.ll[1] : bb.ur[1];
    v[2] = (i & 4) ? bb.ll[2] : bb.ur[2];
    if (in_range(v))
      ++num_in;
  }
  if (num_in == 8)
    return contains;
  return overlaps;
}

vector<nanobot> read() {
  vector<nanobot> result;
  string line;
  while (getline(cin, line))
    result.emplace_back(line);
  return result;
}

// Lower and upper bounds on the number of bots that will be in range
// for the points in the box.  I.e., for every point in the box, that
// point is in range of between lb and ub bots
pair<int, int> interactions(vector<nanobot> const &bots, bbox const &bb) {
  int lower_bound = 0;
  int upper_bound = 0;
  for (auto const &bot : bots)
    switch (bot.interaction(bb)) {
    case contains: ++lower_bound;  // Fall through
    case overlaps: ++upper_bound;  // Fall through
    case disjoint: ;
    }
  return { lower_bound, upper_bound };
}

// Recursively split search space, searching in priority order:
// 1. Upper bound on number of bots in range
// 2. For equal upper bounds, boxes with points closer to the origin
//
// Splitting boxes tends to bring upper and lower bounds closer.
// (When a box is known to be entirely in or entirely out of the range
// of each bot, the bounds are equal.)  Upon splitting, if a greater
// lower bound for bots in range is found, that becomes the new best
// solution.  Or if the lower bound is equal, then take minimum
// distance to origin.
int search(vector<nanobot> const &bots) {
  bbox all = bots.front().bounds();
  for (auto const &bot : bots)
    all = merge(all, bot.bounds());
  // bbb = bounding-box bounds
  using bbb = pair<bbox, pair<int, int>>;
  auto comp_bounds =
    [](bbb const &bbb1, bbb const &bbb2) {
      auto [bb1, bounds1] = bbb1;
      auto [bb2, bounds2] = bbb2;
      auto [lb1, ub1] = bounds1;
      auto [lb2, ub2] = bounds2;
      if (ub1 != ub2)
        // Prefer larger upper bounds
        return ub1 < ub2;
      // If upper bounds match, prefer boxes that have minimal
      // distance to origin (> because max queue)
      return bb1.closest_to_origin() > bb2.closest_to_origin();
    };
  priority_queue<bbb, vector<bbb>, decltype(comp_bounds)> q(comp_bounds);
  int best_lb = 0;
  int best_dist = 0;
  auto add =
    [&](bbox const &bb) {
      auto bounds = interactions(bots, bb);
      auto [lb, ub] = bounds;
      // Go ahead and set the new best case as soon as a box is
      // enqueued
      if (best_lb < lb) {
        best_lb = lb;
        best_dist = bb.closest_to_origin();
      } else if (best_lb == lb)
        best_dist = min(best_dist, bb.closest_to_origin());
      q.push({ bb, bounds });
    };
  add(all);
  while (!q.empty()) {
    auto [bb, bounds] = q.top();
    q.pop();
    auto [lb, ub] = bounds;
    if (ub <= best_lb)
      // Can't do any better than what's already found
      continue;
    // Split along longest dimension
    coords dims = bb.ur - bb.ll;
    int split = 0;
    if (dims[1] > dims[0])
      split = 1;
    if (dims[2] > dims[split])
      split = 2;
    int mid = bb.ll[split] + dims[split] / 2;
    bbox bb1(bb);
    bbox bb2(bb);
    bb1.ur[split] = mid;
    bb2.ll[split] = mid + 1;
    add(bb1);
    add(bb2);
  }
  return best_dist;
}

void part1() {
  auto bots = read();
  auto const &strongest =
    *max_element(bots.begin(), bots.end(),
                 [](nanobot const &bot1, nanobot const &bot2) {
                   return bot1.radius < bot2.radius;
                 });
  int ans = 0;
  for (auto const &other : bots)
    if (strongest.in_range(other))
      ++ans;
  cout << ans << '\n';
}

void part2() { cout << search(read()) << '\n'; }

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
