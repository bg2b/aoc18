// -*- C++ -*-
// Optimization useful for part 2
// g++ -std=c++17 -Wall -g -O -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <optional>
#include <functional>
#include <cassert>

using namespace std;

struct CPU;

int const nop = 16;
int const nreg = 6;

using encoded = array<int, 4>;
using op = function<void(CPU &cpu, encoded const &iabc)>;
using regfile = array<int, nreg>;

// After some experimenting, it turns out that the program halts based
// on the "eqrr 5 0 1" near the end.  Basically it's generating
// random-looking numbers in register 5 and comparing them with
// whatever the initial value of register 0 is, and halting if there's
// equality.  The generated numbers have some initial prefix and then
// start cycling.  So the first number where the program will halt
// comes from the value in register 5 the first time it gets to the
// comparison.  The number that makes it run the longest is the last
// number in the cycle just before it loops back again.

struct CPU {
  // Contents of the registers
  regfile registers;
  // IP binding
  int ipreg;
  // Functions to do the various types of instructions
  vector<op> instructions;
  // The program to run
  vector<encoded> program;
  // Collects the number compared to r0 in the critical comparison
  optional<int> compared;

  // Construct from stdin
  CPU();

  int reg(int r) const { return registers[r]; }
  int &reg(int r) { return registers[r]; }

  // Run the program and monitor the critical comparison.  If
  // before_cycling is false, stops on the first comparison and
  // returns that (part 1).  If true, runs until it sees a comparison
  // about to be repeated, then returns the comparison just before
  // that (part 2).
  int execute(bool before_cycling);
};

// For conciseness...
#define addop(mnemonic, code)                                           \
 mnemonics.emplace(mnemonic, mnemonics.size());                         \
 instructions.push_back([](CPU &cpu, encoded const &iabc) {             \
                          int a = iabc[1], b = iabc[2], c = iabc[3];    \
                          code; })

CPU::CPU() {
  string _;
  cin >> _ >> ipreg;
  map<string, int> mnemonics;
  addop("addr", cpu.reg(c) = cpu.reg(a) + cpu.reg(b));
  addop("addi", cpu.reg(c) = cpu.reg(a) + b);
  addop("mulr", cpu.reg(c) = cpu.reg(a) * cpu.reg(b));
  addop("muli", cpu.reg(c) = cpu.reg(a) * b);
  addop("banr", cpu.reg(c) = cpu.reg(a) & cpu.reg(b));
  addop("bani", cpu.reg(c) = cpu.reg(a) & b);
  addop("borr", cpu.reg(c) = cpu.reg(a) | cpu.reg(b));
  addop("bori", cpu.reg(c) = cpu.reg(a) | b);
  addop("setr", (void)b; cpu.reg(c) = cpu.reg(a));
  addop("seti", (void)b; cpu.reg(c) = a);
  addop("gtir", cpu.reg(c) = a > cpu.reg(b) ? 1 : 0);
  addop("gtri", cpu.reg(c) = cpu.reg(a) > b ? 1 : 0);
  addop("gtrr", cpu.reg(c) = cpu.reg(a) > cpu.reg(b) ? 1 : 0);
  addop("eqir", cpu.reg(c) = a == cpu.reg(b) ? 1 : 0);
  addop("eqri", cpu.reg(c) = cpu.reg(a) == b ? 1 : 0);
  addop("eqrr", cpu.compared = cpu.reg(a);
        cpu.reg(c) = cpu.reg(a) == cpu.reg(b) ? 1 : 0);
  assert(int(instructions.size()) == nop);
  auto readencoded =
    [&](string const &s) {
      stringstream ss(s);
      encoded enc;
      string mnemonic;
      ss >> mnemonic >> enc[1] >> enc[2] >> enc[3];
      auto p = mnemonics.find(mnemonic);
      assert(p != mnemonics.end());
      enc[0] = p->second;
      assert(enc[0] < nop && enc[3] < nreg);
      return enc;
    };
  string line;
  while (getline(cin, line))
    if (!line.empty())
      program.push_back(readencoded(line));
}

#undef addop

int CPU::execute(bool before_cycling) {
  int prev_compared = 0;
  set<int> all_compared;
  for (int i = 0; i < nreg; ++i)
    registers[i] = 0;
  int &ip = registers[ipreg];
  while (true) {
    assert(ip >= 0 && ip < int(program.size()));
    auto const &enc = program[ip];
    instructions[enc[0]](*this, enc);
    ++ip;
    if (compared) {
      if (!before_cycling)
        // Part 1
        return *compared;
      if (all_compared.count(*compared))
        // About to cycle
        return prev_compared;
      all_compared.insert(*compared);
      prev_compared = *compared;
      compared.reset();
    }
  }
}

void part1() { cout << CPU().execute(false) << '\n'; }
void part2() { cout << CPU().execute(true) << '\n'; }

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
