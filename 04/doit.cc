// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

using namespace std;

using sleeping = vector<bool>;
using nights = vector<sleeping>;
map<int, nights> guards;

void read() {
  vector<string> log;
  string line;
  while (getline(cin, line))
    log.push_back(line);
  sort(log.begin(), log.end());
  for (size_t i = 0; i < log.size(); ) {
    auto const &entry = log[i++];
    string gn = "Guard #";
    auto pos = entry.find(gn);
    assert(pos != string::npos);
    int num = stoi(entry.substr(pos + gn.size()));
    auto &guard = guards[num];
    sleeping night(60, false);
    auto event = [](string const &entry) {
                   auto pos = entry.find(']');
                   assert(pos != string::npos && pos >= 2);
                   int time = stoi(entry.substr(pos - 2, 2));
                   assert(pos + 2 < entry.length());
                   char event = entry[pos + 2];
                   return make_pair(time, event);
                 };
    while (i < log.size() && log[i].find(gn) == string::npos) {
      assert(i + 1 < log.size());
      auto [time1, event1] = event(log[i]);
      auto [time2, event2] = event(log[i + 1]);
      assert(event1 == 'f' && event2 == 'w');
      assert(0 <= time1 && time1 < time2 && time2 < 60);
      fill(night.begin() + time1, night.begin() + time2, true);
      i += 2;
    }
    guard.push_back(night);
  }
}

// Summarize sleepiness for one guard.  Returns:
// { total sleep, max sleep over all minutes, minute for max sleep }
array<int, 3> sleepiness(nights const &all_nights) {
  int sleeping = 0;
  vector<int> per_night(60, 0);
  for (auto const &night : all_nights)
    for (int i = 0; i < 60; ++i)
      if (night[i]) {
        ++sleeping;
        ++per_night[i];
      }
  auto p = max_element(per_night.begin(), per_night.end());
  int sleepiest_minute = p - per_night.begin();
  return { sleeping, per_night[sleepiest_minute], sleepiest_minute };
}

// Chooses the guard based on sleepiness(...)[best_choice]
void solve(unsigned best_choice) {
  read();
  optional<int> sleepiest;
  int max_sleep = 0;
  for (auto const & [num, all_nights] : guards) {
    int sleep = sleepiness(all_nights)[best_choice];
    if (sleep >= max_sleep) {
      sleepiest = num;
      max_sleep = sleep;
    }
  }
  assert(sleepiest);
  auto const &all_nights = guards[*sleepiest];
  int sleepiest_minute = sleepiness(all_nights)[2];
  cout << *sleepiest * sleepiest_minute << '\n';
}

void part1() { solve(0); }
void part2() { solve(1); }

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
