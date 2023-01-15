// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <cassert>

using namespace std;

using coords = pair<int, int>;

coords operator+(coords const &c1, coords const &c2) {
  return { c1.first + c2.first, c1.second + c2.second };
}

coords operator-(coords const &c) { return { -c.first, -c.second }; }

coords rotate(coords dir, int l_or_r) {
  assert(abs(l_or_r) <= 1);
  if (l_or_r == 0)
    return dir;
  if (l_or_r == -1)
    l_or_r = 3;
  while (l_or_r-- > 0)
    dir = { -dir.second, dir.first };
  return dir;
}

struct cart {
  // For distinguishing carts
  unsigned id;
  // Where the cart is
  coords xy;
  // The direction it's moving
  coords dir;
  // Turn it makes at the next intersection, left = -1, straight = 0,
  // right = +1
  int next_turn{-1};
  // Has the cart crashed into some other?
  bool crashed{false};

  cart(unsigned id_, coords const &xy_, coords const &dir_) :
    id(id_), xy(xy_), dir(dir_) {}

  void forward() { xy = xy + dir; }
  void turn(char track);

  char print() const;
};

void cart::turn(char track) {
  if (crashed)
    return;
  // Driving straight
  if (track == '-') {
    assert(abs(dir.second) == 0);
    return;
  }
  if (track == '|') {
    assert(abs(dir.first) == 0);
    return;
  }
  // Going around a curve
  if (track == '/') {
    swap(dir.first, dir.second);
    dir = -dir;
    return;
  }
  if (track == '\\') {
    swap(dir.first, dir.second);
    return;
  }
  // At an intersection
  assert(track == '+');
  dir = rotate(dir, next_turn);
  if (++next_turn == +2)
    next_turn = -1;
}

struct racetrack {
  // The track (with all carts removed)
  vector<string> track;
  // The carts (reordered for each tick)
  vector<cart> carts;

  // Construct from stdin
  racetrack();

  // The stretch of track is at the given coordinates
  char at(coords const &c) const;

  // Do one tick, return either the location of the first crash, or
  // the location of the last surviving cart (or null if there's no
  // crash and multiple carts running)
  optional<coords> tick(bool first_crash);
};

racetrack::racetrack() {
  auto place_carts =
    [&](char c, string &s, coords const &dir) {
      while (auto pos = s.find(c)) {
        if (pos == string::npos)
          return;
        coords xy{ pos, track.size() };
        carts.emplace_back(carts.size(), xy, dir);
        s[pos] = dir.second == 0 ? '-' : '|';
      }
    };
  string row;
  while (getline(cin, row)) {
    place_carts('<', row, { -1, 0 });
    place_carts('>', row, { +1, 0 });
    place_carts('^', row, { 0, -1 });
    place_carts('v', row, { 0, +1 });
    track.push_back(row);
    assert(track.front().length() == row.length());
  }
  assert(!track.empty());
}

char racetrack::at(coords const &c) const {
  assert(c.first >= 0 && c.first < int(track.front().length()));
  assert(c.second >= 0 && c.second < int(track.size()));
  return track[c.second][c.first];
}

optional<coords> racetrack::tick(bool first_crash) {
  sort(carts.begin(), carts.end(),
       [](auto const &c1, auto const &c2) {
         if (c1.xy.second != c2.xy.second)
           return c1.xy.second < c2.xy.second;
         return c1.xy.first < c2.xy.first;
       });
  for (auto &c : carts) {
    if (c.crashed)
      // Already crashed into some other cart
      continue;
    c.forward();
    for (auto &other : carts) {
      if (!other.crashed && c.id != other.id && c.xy == other.xy) {
        c.crashed = true;
        other.crashed = true;
        if (first_crash)
          return c.xy;
      }
    }
    c.turn(at(c.xy));
  }
  optional<coords> result;
  if (!first_crash)
    // See if there's a lone survivor
    for (auto const &c : carts)
      if (!c.crashed) {
        if (result)
          // Multiple surviving carts
          return nullopt;
        result = c.xy;
      }
  return result;
}

void race(bool first_crash) {
  racetrack track;
  optional<coords> ans;
  while (true) {
    ans = track.tick(first_crash);
    if (ans)
      break;
  }
  cout << ans->first << ',' << ans->second << '\n';
}

void part1() { race(true); }
void part2() { race(false); }

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
