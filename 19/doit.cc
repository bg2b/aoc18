// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <functional>
#include <cassert>

using namespace std;

struct CPU;

int const nop = 16;
int const nreg = 6;

using encoded = array<int, 4>;
using op = function<void(CPU &cpu, encoded const &iabc)>;
using regfile = array<int, nreg>;

struct CPU {
  // Contents of the registers
  regfile registers;
  // IP binding
  int ipreg;
  // Functions to do the various types of instructions
  vector<op> instructions;
  // The program to run
  vector<encoded> program;

  // Construct from stdin
  CPU();

  int reg(int r) const { return registers[r]; }
  int &reg(int r) { return registers[r]; }

  // Run the program with r0 as the initial value of register 0, for
  // up to 10 million instructions.  Returns whatever is in register 0
  // at the end.
  int execute(int r0);
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
  addop("eqrr", cpu.reg(c) = cpu.reg(a) == cpu.reg(b) ? 1 : 0);
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

int CPU::execute(int r0) {
  registers[0] = r0;
  for (int i = 1; i < nreg; ++i)
    registers[i] = 0;
  int &ip = registers[ipreg];
  int max_exec = 10000000;
  while (ip >= 0 && ip < int(program.size()) && max_exec-- > 0) {
    auto const &enc = program[ip];
    instructions[enc[0]](*this, enc);
    ++ip;
  }
  return registers[0];
}

void part1() { cout << CPU().execute(0) << '\n'; }

void part2() {
  // From some experimentation and inspection, it appears that the
  // program is just computing the sum of all factors of whatever is
  // in register 4 (after an initialization phase).
  CPU cpu;
  // This won't run to completion, but register 4 will be stable
  (void)cpu.execute(1);
  // After register 4 is initialized, it doesn't change, so just grab
  // whatever is there and add up the factors
  int n = cpu.registers[4];
  int sum_factors = 0;
  for (int i = 1; i * i <= n; ++i)
    if (n % i == 0)
      sum_factors += i + n / i;
  cout << sum_factors << '\n';
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
