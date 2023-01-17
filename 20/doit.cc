// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <variant>
#include <functional>
#include <cctype>
#include <cassert>

using namespace std;

using coords = pair<int, int>;

coords operator+(coords const &c1, coords const &c2) {
  return { c1.first + c2.first, c1.second + c2.second };
}

map<char, coords> dirs{
  { 'N', { 0, 1 } }, { 'S', { 0, -1 } }, { 'E', { 1, 0 } }, { 'W', { -1, 0 } }
};

struct rooms {
  // Which rooms connect to which
  map<coords, set<coords>> doors;

  // Record that there's a door between a room and an adjacent room in
  // direction dir.  Return the coordinates of the adjacent room
  coords walk(coords const &room, char dir);

  // Is there a door between these rooms?
  bool door(coords const &room1, coords const &room2) const;

  // Generic breadth-first search from { 0, 0 }; call visit_room in
  // each room
  void bfs(function<void(coords const &, int)> visit_room) const;

  // C++ is very restricted in identifier names :-(
  int f_lpar_a_or_u_rpar_rthest() const;

  // How many rooms are at least threshold away?
  int count_at_least(int threshold) const;
};

coords rooms::walk(coords const &c, char dir) {
  auto p = dirs.find(dir);
  assert(p != dirs.end());
  coords adj = c + p->second;
  doors[c].insert(adj);
  doors[adj].insert(c);
  return adj;
}

bool rooms::door(coords const &room1, coords const &room2) const {
  auto p = doors.find(room1);
  assert(p != doors.end());
  return p->second.count(room2);
}

void rooms::bfs(function<void(coords const &, int)> visit_room) const {
  set<coords> visited;
  list<pair<coords, int>> frontier;
  auto visit = [&](coords const &c, int depth) {
                 if (visited.count(c))
                   return;
                 visited.insert(c);
                 frontier.emplace_back(c, depth);
               };
  visit({ 0, 0 }, 0);
  while (!frontier.empty()) {
    auto [c, depth] = frontier.front();
    frontier.pop_front();
    visit_room(c, depth);
    for (auto dir : dirs) {
      auto adj = c + dir.second;
      if (door(c, adj))
        visit(adj, depth + 1);
    }
  }
}

int rooms::f_lpar_a_or_u_rpar_rthest() const {
  int max_depth = 0;
  bfs([&](coords const &_, int depth) { max_depth = max(max_depth, depth); });
  return max_depth;
}

int rooms::count_at_least(int threshold) const {
  int total = 0;
  bfs([&](coords const &_, int depth) {
        if (depth >= threshold)
          ++total;
      });
  return total;
}

// A regex is...
struct regex: variant<char, vector<regex>, set<regex>> {
  // a single character; or
  regex(char c) : variant(c) {}
  // a sequence of regexs that must match in order; or
  regex(vector<regex> const &sequence) : variant(sequence) {}
  // an alternative, where one of the regexs must match
  regex(set<regex> const &alternatives) : variant(alternatives) {}
};

// Parse a regex for a sequence from a string(stream) representation.
// Leaves the stream positioned for reading whatever is next
regex parse_regex(stringstream &ss) {
  vector<regex> sequence;
  while (true) {
    if (isalpha(ss.peek())) {
      sequence.emplace_back(ss.get());
      continue;
    }
    if (ss.eof() || ss.peek() == '|' || ss.peek() == ')')
      // At the end of something
      break;
    // The only alternative is an alternative ;-)
    assert(ss.peek() == '(');
    ss.ignore(1);
    set<regex> alternatives;
    while (true) {
      alternatives.insert(parse_regex(ss));
      if (ss.peek() == ')') {
        ss.ignore(1);
        break;
      }
      assert(ss.peek() == '|');
      ss.ignore(1);
    }
    sequence.emplace_back(alternatives);
  }
  return regex(sequence);
}

// Starting from any room in cs, record that it's possible to walk a
// path corresponding to the regex.  Return the set of rooms it's
// possible to wind up in.
set<coords> walk_regex(set<coords> const &cs, regex const &re, rooms &maze) {
  set<coords> next;
  switch (re.index()) {
  case 0:
    // A simple direction, step from each room to an adjacent room
    for (auto current : cs)
      next.insert(maze.walk(current, get<0>(re)));
    break;
  case 1:
    // A sequence; walk each element in turn
    next = cs;
    for (auto const &re1 : get<1>(re))
      next = walk_regex(next, re1, maze);
    break;
  default:
    // An alternative; walk each element and collect all the
    // possible places we could wind up
    for (auto const &re1 : get<2>(re)) {
      auto alt = walk_regex(cs, re1, maze);
      next.insert(alt.begin(), alt.end());
    }
    break;
  }
  return next;
}

rooms read() {
  string s;
  cin >> s;
  assert(!s.empty() && s.front() == '^' && s.back() == '$');
  s.pop_back();
  s = s.substr(1);
  stringstream ss(s);
  regex re = parse_regex(ss);
  assert(ss.eof());
  rooms maze;
  coords start{ 0, 0 };
  walk_regex({ start }, re, maze);
  return maze;
}

void part1() { cout << read().f_lpar_a_or_u_rpar_rthest() << '\n'; }
void part2() { cout << read().count_at_least(1000) << '\n'; }

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
