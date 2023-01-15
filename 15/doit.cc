// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <cassert>

using namespace std;

using coords = pair<int, int>;

coords operator+(coords const &c1, coords const &c2) {
  return { c1.first + c2.first, c1.second + c2.second };
}

bool operator<(coords const &c1, coords const &c2) {
  if (c1.second != c2.second)
    return c1.second < c2.second;
  return c1.first < c2.first;
}

int manhattan(coords const &c1, coords const &c2) {
  return abs(c1.first - c2.first) + abs(c1.second - c2.second);
}

// These are ordered so that adding them to some coordinate gives the
// neighbors in reading_order, though I don't believe it matters...
vector<coords> adjacent{
  { 0, -1 }, { -1, 0 }, { +1, 0 }, { 0, +1 }
};

struct unit {
  bool elf;
  coords xy;
  int hp{200};
  int attack_power{3};

  unit(bool elf_, coords const &xy_) : elf(elf_), xy(xy_) {}

  bool dead() const { return hp <= 0; }
};

struct arena {
  // The map (sans elves or goblins)
  vector<string> caves;
  // The combatants
  vector<unit> units;
  // The number of full rounds that have been completed
  unsigned full_rounds{0};
  // The number of elves that have died
  unsigned dead_elves{0};

  // Construct from stdin
  arena();

  char at(coords const &c) const;
  // All the squares around c that are valid (not considering unit
  // positions)
  vector<coords> around(coords const &c) const;

  // Set the attack power of all the elves
  void elf_power(int power);

  // Fight the battle; if dead_elves_ok is true, gives the final
  // outcome.  If dead_elves_ok is false and an elf dies, bails out
  // early with -1
  int fight(bool dead_elves_ok);

  // One round of the battle
  int round();

  // The active unit looks for targets; returns 0 if any are found,
  // otherwise gives the battle outcome (total hit points of the
  // survivors times full_rounds)
  int no_targets(unit const &active) const;
  // Decide how to move the active unit
  void move(unit &active);
  // The active unit tries to attack something nearby
  void attack(unit const &active);
};

arena::arena() {
  auto add_units = [&](char c, string &s) {
                     while (true) {
                       auto pos = s.find(c);
                       if (pos == string::npos)
                         return;
                       coords xy{ pos, caves.size() };
                       units.emplace_back(c == 'E', xy);
                       s[pos] = '.';
                     }
                   };
  string row;
  while (getline(cin, row)) {
    add_units('E', row);
    add_units('G', row);
    caves.push_back(row);
    assert(row.length() == caves.front().length());
  }
  assert(!caves.empty());
  assert(!units.empty());
}

char arena::at(coords const &c) const {
  assert(c.first >= 0 && c.first < int(caves.front().length()));
  assert(c.second >= 0 && c.second < int(caves.size()));
  return caves[c.second][c.first];
}

vector<coords> arena::around(coords const &c) const {
  vector<coords> result;
  result.reserve(adjacent.size());
  for (auto const &adj : adjacent)
    if (at(c + adj) == '.')
      result.push_back(c + adj);
  return result;
}

void arena::elf_power(int power) {
  for (auto &u : units)
    if (u.elf)
      u.attack_power = power;
}

int arena::fight(bool dead_elves_ok) {
  int ans;
  while ((ans = round()) == 0 && (dead_elves_ok || !dead_elves))
    ;
  if (!dead_elves_ok && dead_elves)
    return -1;
  return ans;
}

int arena::round() {
  // Movement order
  sort(units.begin(), units.end(), [](unit const &u1, unit const &u2) {
                                     return u1.xy < u2.xy;
                                   });
  // Attack!
  for (auto &u : units)
    if (!u.dead()) {
      if (int outcome = no_targets(u))
        return outcome;
      move(u);
      attack(u);
    }
  ++full_rounds;
  // Bring out your dead
  for (size_t i = 0; i < units.size(); )
    if (units[i].dead())
      units.erase(units.begin() + i);
    else
      ++i;
  return 0;
}

int arena::no_targets(unit const &active) const {
  bool potential_targets = false;
  unsigned total_hp = 0;
  for (auto const &u : units)
    if (!u.dead()) {
      total_hp += u.hp;
      potential_targets = potential_targets || active.elf != u.elf;
    }
  return potential_targets ? 0 : total_hp * full_rounds;
}

// Yes, this is a too long, but I think breaking it up doesn't really
// make it any clearer.  Just take it in sections (marked by =====)
void arena::move(unit &active) {
  assert(!active.dead());
  // ===== Fast return if there's already an adjacent target
  for (auto const &u : units)
    if (active.elf != u.elf && !u.dead() && manhattan(active.xy, u.xy) == 1)
      return;
  // ===== Real movement starts here
  // Where are the other units?
  set<coords> occupied;
  for (auto const &u : units)
    if (!u.dead())
      occupied.insert(u.xy);
  occupied.erase(active.xy);
  // ===== Find the in-range squares
  set<coords> in_range;
  for (auto const &u : units)
    if (active.elf != u.elf && !u.dead())
      for (auto const &c : around(u.xy))
        if (!occupied.count(c))
          in_range.insert(c);
  if (in_range.empty())
    // Even if there are live enemies, this can happen if all squares
    // adjacent to the enemies are already occupied
    return;
  // ===== Search for closest in-range
  // Normal breadth-first search.  Initialize the visited set with
  // where units are so that those locations won't be searched.  (Note
  // that active.xy is not in occupied...)
  set<coords> visited = occupied;
  list<pair<coords, int>> frontier;
  auto visit = [&](coords const &c, int depth) {
                 if (visited.count(c))
                   return;
                 visited.insert(c);
                 frontier.emplace_back(c, depth);
               };
  visit(active.xy, 0);
  assert(!frontier.empty());
  optional<coords> chosen;
  optional<int> closest_depth;
  while (!frontier.empty()) {
    auto [c, depth] = frontier.front();
    frontier.pop_front();
    if (closest_depth && depth > *closest_depth)
      // Any other in-range are going to be farther away
      break;
    if (in_range.count(c)) {
      if (!chosen || c < *chosen) {
        // Either first in-range found, or c is better in the order
        chosen = c;
        closest_depth = depth;
      }
      // Still look for other things on the frontier that could be
      // in-range and at the same depth, but there's no point in
      // considering successors from this location since those would
      // have greater depth
      continue;
    }
    for (auto const &adj : around(c))
      visit(adj, depth + 1);
  }
  if (!chosen)
    // Nothing in-range is reachable
    return;
  // ===== Search backwards to find the initial step
  // Could do this more efficiently by restricting to the
  // previously-visited things (even stratifying by depth), but
  // whatevs...
  visited = occupied;
  frontier.clear();
  visit(*chosen, 0);
  optional<coords> first_step;
  while (true) {
    assert(!frontier.empty());
    auto [c, depth] = frontier.front();
    frontier.pop_front();
    if (c == active.xy)
      // Found a path back to active.  At this point, any adjacent
      // square to active that was reachable at the same depth as
      // *first_step will have been expanded.
      break;
    for (auto const &adj : around(c)) {
      if (adj == active.xy && (!first_step || c < *first_step))
        // c is adjacent to active and so is a candidate for the first
        // step.  There could be another such square that's better in
        // the ordering though, so it's not possible to stop searching
        // just yet.
        first_step = c;
      visit(adj, depth + 1);
    }
  }
  assert(first_step);
  active.xy = *first_step;
}

void arena::attack(unit const &active) {
  unit *target = nullptr;
  auto better_target = [&](unit &u) {
                         // Must be a different type
                         if (active.elf == u.elf) return false;
                         // Can't be dead
                         if (u.dead()) return false;
                         // Must be adjacent
                         if (manhattan(active.xy, u.xy) != 1) return false;
                         // No target so far?
                         if (!target) return true;
                         // Lower on hp?
                         if (u.hp != target->hp) return u.hp < target->hp;
                         // Same hp, better in the order?
                         return u.xy < target->xy;
                       };
  for (auto &u : units)
    if (better_target(u))
      target = &u;
  if (target) {
    target->hp -= active.attack_power;
    if (target->dead() && target->elf)
      ++dead_elves;
  }
}

void part1() { cout << arena().fight(true) << '\n'; }

void part2() {
  arena caverns;
  int ans;
  for (int power = 3; ; ++power) {
    arena trial_by_combat(caverns);
    trial_by_combat.elf_power(power);
    if ((ans = trial_by_combat.fight(false)) > 0)
      break;
  }
  cout << ans << '\n';
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
