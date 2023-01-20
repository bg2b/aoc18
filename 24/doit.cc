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
#include <algorithm>
#include <cctype>
#include <cassert>

using namespace std;

enum modifier { none = 0, weak, immune };

struct group {
  // true => infection, false => immune system
  bool hostile;
  // Units in the group
  int units;
  // Hitpoints per unit
  int hp;
  // Damage modifiers
  map<string, modifier> modifiers;
  // Attack type
  string attack;
  // Attack power
  int damage;
  // Attack initiative
  int initiative;

  // Construct from string description
  group(bool hostile_, string const &s);

  int effective_power() const { return units * damage; }

  // Damage, accounting for modifiers
  int damage_from(string const &attack_type, int attack_dmg) const;
  // Deal damage, reduce number of units, return units killed
  int take_damage(string const &attack_type, int attack_dmg);

  bool dead() const { return units == 0; }

  void boost(int amount) { if (!hostile) damage += amount; }
};

group::group(bool hostile_, string const &s) : hostile(hostile_) {
  stringstream ss(s);
  string _;
  //      #    units each with   #  hit points
  ss >> units >> _ >> _ >> _ >> hp >> _ >> _;
  string with;
  ss >> with;
  if (with[0] == '(') {
    // Optional damage modifiers
    auto read_mods =
      [&](string const &mod_type) {
        // Read a mod like "weak to fire, radiation"
        // Return true if there are more modifiers to read
        assert(mod_type == "weak" || mod_type == "immune");
        modifier mod = mod_type == "weak" ? weak : immune;
        //   to
        ss >> _;
        // After each attack type, there's an ending
        // , => more attack types
        // ; => another modifier type
        // ) => done with modifiers
        while (true) {
          string type;
          ss >> type;
          char ending = type.back();
          type.pop_back();
          modifiers.emplace(type, mod);
          if (ending != ',') {
            assert(ending == ';' || ending == ')');
            return ending == ';';
          }
        }
      };
    string mod_type = with.substr(1);
    while (read_mods(mod_type))
      ss >> mod_type;
    ss >> with;
    assert(with == "with");
  }
  //   an attack that does     #      <type>   dmg at initiatv     #
  ss >> _ >> _ >> _ >> _ >> damage >> attack >> _ >> _ >> _ >> initiative;
}

int group::damage_from(string const &attack_type, int attack_dmg) const {
  auto p = modifiers.find(attack_type);
  if (p == modifiers.end())
    return attack_dmg;
  if (p->second == immune)
    return 0;
  assert(p->second == weak);
  return 2 * attack_dmg;
}

int group::take_damage(string const &attack_type, int attack_dmg) {
  int effective_dmg = damage_from(attack_type, attack_dmg);
  int units_killed = min(effective_dmg / hp, units);
  units -= units_killed;
  return units_killed;
}

vector<group> read() {
  vector<group> result;
  bool hostile = false;
  string line;
  while (getline(cin, line)) {
    if (line.empty())
      continue;
    if (line == "Immune System:") {
      hostile = false;
      continue;
    }
    if (line == "Infection:") {
      hostile = true;
      continue;
    }
    assert(isdigit(line[0]));
    result.emplace_back(hostile, line);
  }
  return result;
}

bool better_attack(group const &current, group const &potential,
                   group const &attacker) {
  int incoming = attacker.effective_power();
  int current_dmg = current.damage_from(attacker.attack, incoming);
  int potential_dmg = potential.damage_from(attacker.attack, incoming);
  if (current_dmg != potential_dmg)
    return current_dmg < potential_dmg;
  if (current.effective_power() != potential.effective_power())
    return current.effective_power() < potential.effective_power();
  return current.initiative < potential.initiative;
}

bool fight(vector<group> &groups) {
  vector<group const *> order;
  for (auto const &g : groups)
    order.push_back(&g);
  // Target selection order
  sort(order.begin(), order.end(),
       [](group const *g1, group const *g2) {
         int power1 = g1->effective_power();
         int power2 = g2->effective_power();
         if (power1 != power2)
           return power1 > power2;
         return g1->initiative > g2->initiative;
       });
  // Groups choose targets
  map<group const *, group *> targets;
  set<group const *> targeted;
  for (auto attacker : order) {
    int damage = attacker->effective_power();
    for (auto &defender : groups) {
      if (targeted.count(&defender))
        // Some else already attacked this group
        continue;
      if (defender.hostile == attacker->hostile)
        // Don't attack friendlies
        continue;
      int effective_dmg = defender.damage_from(attacker->attack, damage);
      if (effective_dmg == 0)
        // Defending group is immune to the attack
        continue;
      auto p = targets.find(attacker);
      if (p == targets.end() || better_attack(*p->second, defender, *attacker))
        targets.insert_or_assign(attacker, &defender);
    }
    auto p = targets.find(attacker);
    if (p != targets.end())
      targeted.insert(p->second);
  }
  if (targeted.empty())
    // No one can attack (all remaining groups are immune to the
    // available attacks)
    return false;
  // Attack order
  sort(order.begin(), order.end(),
       [](group const *g1, group const *g2) {
         return g1->initiative > g2->initiative;
       });
  // Attack
  for (auto attacker : order) {
    if (attacker->dead())
      // Already killed
      continue;
    auto p = targets.find(attacker);
    if (p == targets.end())
      // No one to target
      continue;
    auto &defender = *p->second;
    defender.take_damage(attacker->attack, attacker->effective_power());
  }
  // Eliminate dead groups
  groups.erase(remove_if(groups.begin(), groups.end(),
                         [](group const &g) { return g.dead(); }),
               groups.end());
  // Still any fighting left to do?
  return (any_of(groups.begin(), groups.end(),
                 [](group const &g) { return !g.hostile; }) &&
          any_of(groups.begin(), groups.end(),
                 [](group const &g) { return g.hostile; }));
}

pair<int, int> to_the_death(vector<group> groups) {
  while (fight(groups))
    ;
  int immune = 0, infection = 0;
  for (auto const &g : groups)
    (g.hostile ? infection : immune) += g.units;
  // Note that it is possible to get into stalemates, in which case
  // both immune and infection will be nonzero
  return { immune, infection };
}

void part1() {
  auto [immune, infection] = to_the_death(read());
  cout << immune + infection << '\n';
}

void part2() {
  auto groups = read();
  auto immune_left =
    [&](int amount) {
      auto trial_groups = groups;
      for (auto &g : trial_groups)
        g.boost(amount);
      auto [immune, infection] = to_the_death(trial_groups);
      // Stalemate counts as failure
      return infection == 0 ? immune : 0;
    };
  // Initial bracketing
  int bad_boost = 0;
  int good_boost = 1;
  while (immune_left(good_boost) == 0) {
    bad_boost = good_boost;
    good_boost *= 2;
  }
  // Bisection to get minimum boost
  while (good_boost > bad_boost + 1) {
    int mid = (good_boost + bad_boost) / 2;
    if (immune_left(mid))
      good_boost = mid;
    else
      bad_boost = mid;
  }
  cout << immune_left(good_boost) << '\n';
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
