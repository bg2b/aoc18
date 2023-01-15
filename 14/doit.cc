// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <functional>
#include <cassert>

using namespace std;

vector<char> scoreboard{ 3, 7 };

void cook(function<bool(void)> stop) {
  size_t elf1 = 0;
  size_t elf2 = 1;
  while (!stop()) {
    char sum = scoreboard[elf1] + scoreboard[elf2];
    if (sum >= 10) {
      scoreboard.push_back(1);
      sum -= 10;
    }
    scoreboard.push_back(sum);
    elf1 += scoreboard[elf1] + 1;
    elf1 %= scoreboard.size();
    elf2 += scoreboard[elf2] + 1;
    elf2 %= scoreboard.size();
  }
}

void part1() {
  unsigned ten_after;
  cin >> ten_after;
  cook([=] { return ten_after + 9 < scoreboard.size(); });
  for (unsigned i = 0; i < 10; ++i)
    cout << char(scoreboard[ten_after + i] + '0');
  cout << '\n';
}

void part2() {
  vector<char> wanted;
  char c;
  while (cin >> c) {
    assert(c >= '0' && c <= '9');
    wanted.push_back(c - '0');
  }
  // Careful here; the scoreboard may have increased in size by more
  // than 1, so it may be necessary to check multiple places.  The
  // scoreboard starting at offset is the next thing that should be
  // checked.
  size_t offset = 0;
  // Utility function to check for a match starting at offset
  auto match = [&] {
                 for (size_t i = 0; i < wanted.size(); ++i)
                   if (scoreboard[i + offset] != wanted[i])
                     return false;
                 return true;
               };
  cook([&] {
         if (scoreboard.size() < wanted.size())
           return false;
         size_t max_offset = scoreboard.size() - wanted.size();
         bool matched;
         while (offset <= max_offset && !(matched = match()))
           ++offset;
         return matched;
       });
  cout << offset << '\n';
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
