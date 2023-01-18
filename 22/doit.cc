// -*- C++ -*-
// Could possibly turn optimization on if desired; it's a over a
// second otherwise for part 2...
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <cassert>

using namespace std;

using coords = pair<int, int>;

coords operator+(coords const &c1, coords const &c2) {
  return { c1.first + c2.first, c1.second + c2.second };
}

int manhattan(coords const &c1, coords const &c2) {
  return abs(c1.first - c2.first) + abs(c1.second - c2.second);
}

// Region numbers correspond to risk levels (and to erosion level % 3)
enum region { rocky = 0, wet, narrow };
// Tool numbers chosen so that legal situations are type != tool
enum tool { neither = 0, torch, climbing };

struct cave {
  // Cave depth for erosion computations
  int depth;
  // Where the friend is
  coords target;
  // Cached erosion levels.  An unordered map is a bit faster, but
  // having to declare hashers for pairs is annoying.  Mutable since
  // conceptually things are const but this may be added to
  mutable map<coords, int> erosion;

  cave();

  // Get the erosion level at a location
  int erosion_level(coords const &c) const;
  // What's the region at a location?
  region type(coords const &c) const { return region(erosion_level(c) % 3); }

  // Sum of risks to target
  int risk_level() const;
  // Minimal path length to target
  int search() const;
};

cave::cave() {
  string _;
  cin >> _ >> depth;
  char comma;
  cin >> _ >> target.first >> comma >> target.second;
}

int cave::erosion_level(coords const &c) const {
  auto p = erosion.find(c);
  if (p == erosion.end()) {
    int geologic_index;
    if (c == coords{ 0, 0 } || c == target)
      geologic_index = 0;
    else if (c.second == 0)
      geologic_index = 16807 * c.first;
    else if (c.first == 0)
      geologic_index = 48271 * c.second;
    else
      geologic_index = (erosion_level(c + coords{ 0, -1 }) *
                        erosion_level(c + coords{ -1, 0 }));
    p = erosion.emplace(c, (geologic_index + depth) % 20183).first;
  }
  return p->second;
}

int cave::risk_level() const {
  int total_risk = 0;
  int nx = target.first + 1;
  int ny = target.second + 1;
  for (int x = 0; x < nx; ++x)
    for (int y = 0; y < ny; ++y)
      total_risk += type(coords{ x, y });
  return total_risk;
}

vector<coords> dirs{
  { +1, 0 }, { 0, +1 }, { -1, 0 }, { 0, -1 }
};

int cave::search() const {
  using state = pair<coords, tool>;
  auto bound = [&](state const &st) {
                 // If not holding the torch, have to do at least one
                 // switch.  Best case movement requires no other
                 // switches
                 int result = manhattan(st.first, target);
                 if (st.second != torch)
                   result += 7;
                 return result;
               };
  auto legal = [&](coords const &c, int holding) {
                 return type(c) != holding;
               };
  // Visited holds the minimum minutes to reach a state, and whether
  // such a state has actually been searched yet
  map<state, pair<int, bool>> visited;
  // Searching pairs (minutes so far, state)
  using t_st = pair<int, state>;
  auto comp_t_st = [&](t_st const &ts1, t_st const &ts2) {
                     int h1 = ts1.first + bound(ts1.second);
                     int h2 = ts2.first + bound(ts2.second);
                     // Swap order since priority_queue gives max
                     return h2 < h1;
                   };
  priority_queue<t_st, vector<t_st>, decltype(comp_t_st)> q(comp_t_st);
  auto visit = [&](t_st const &ts) {
                 auto p = visited.find(ts.second);
                 if (p != visited.end() && p->second.first <= ts.first)
                   // Already reached by a path at least as quick
                   return;
                 // New state or reached quicker, not yet searched
                 visited.insert_or_assign(ts.second, make_pair(ts.first, false));
                 q.push(ts);
               };
  visit(t_st(0, { coords{ 0, 0 }, torch }));
  state goal{ target, torch };
  optional<int> min_t;
  // Have to search until there's nothing in q, since the queue
  // ordering is only heuristic
  while (!q.empty()) {
    auto [t, st] = q.top();
    q.pop();
    auto p = visited.find(st);
    auto & [tvis, searched] = p->second;
    if ((!searched && t > tvis) || (searched && t >= tvis))
      // Either this state has been enqueued with a strictly quicker
      // time, or it's been searched with the same time
      continue;
    searched = true;
    if (st == goal) {
      // Goal reached; t has to be smaller than any previous time
      min_t = t;
      continue;
    }
    if (min_t && t + bound(st) >= *min_t)
      // Cut off based on current best solution
      continue;
    // Consider movement
    auto [c, holding] = st;
    for (auto const &dir : dirs) {
      auto next = c + dir;
      if (next.first >= 0 && next.second >= 0 && legal(next, holding))
        // In bounds and holding a valid tool
        visit({ t + 1, { next, holding } });
    }
    // Consider different tools
    for (auto next_holding : { neither, torch, climbing })
      if (next_holding != holding && legal(c, next_holding))
        visit({ t + 7, { c, next_holding } });
  }
  assert(min_t);
  return *min_t;
}

void part1() { cout << cave().risk_level() << '\n'; }

void part2() { cout << cave().search() << '\n'; }

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
