// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <map>
#include <tuple>
#include <functional>
#include <algorithm>
#include <cassert>

using namespace std;

vector<array<int, 4>> program;

struct CPU;

int const nop = 16;
int const nreg = 4;

using encoded = array<int, 4>;
using op = function<void(CPU &cpu, encoded const &iabc)>;
using regfile = array<int, nreg>;
using observation = tuple<regfile, encoded, regfile>;
using possibilities = array<array<bool, nop>, nop>;

struct CPU {
  // Contents of the registers
  regfile registers;
  // Functions to do the various types of instructions
  vector<op> instructions;
  // Tests for deducing the opcode => instructions mapping
  vector<observation> tests;
  // The program to run
  vector<encoded> program;

  // Construct from stdin
  CPU();

  int reg(int r) const { return registers[r]; }
  int &reg(int r) { return registers[r]; }

  // Is an instruction consistent with the before/after contents of
  // registers?
  bool consistent(op const &instr, observation const &test);
  // How many instructions are consistent with the given test?
  int how_many(observation const &test);

  // Find encoding and reorder the instructions to match
  void deduce_opcodes();
  // Run the program
  int execute();
};

// For conciseness...
#define addop(code)                                                     \
 instructions.push_back([](CPU &cpu, encoded const &iabc) {             \
                          int a = iabc[1], b = iabc[2], c = iabc[3];    \
                          code; })

CPU::CPU() {
  auto readreg =
    [](string const &s) {
      auto pos = s.find('[');
      assert(pos != string::npos);
      stringstream ss(s.substr(pos + 1));
      char _;
      regfile reg;
      ss >> reg[0] >> _ >> reg[1] >> _ >> reg[2] >> _ >> reg[3];
      return reg;
    };
  auto readencoded =
    [](string const &s) {
      stringstream ss(s);
      encoded enc;
      ss >> enc[0] >> enc[1] >> enc[2] >> enc[3];
      assert(enc[0] < nop && enc[3] < nreg);
      return enc;
    };
  string line;
  while (getline(cin, line)) {
    if (line.empty())
      break;
    regfile reg_start = readreg(line);
    getline(cin, line);
    encoded enc = readencoded(line);
    getline(cin, line);
    regfile reg_end = readreg(line);
    getline(cin, line);
    assert(line.empty());
    tests.emplace_back(reg_start, enc, reg_end);
  }
  while (getline(cin, line))
    if (!line.empty())
      program.push_back(readencoded(line));
  // addr
  addop(cpu.reg(c) = cpu.reg(a) + cpu.reg(b));
  // addi
  addop(cpu.reg(c) = cpu.reg(a) + b);
  // mulr
  addop(cpu.reg(c) = cpu.reg(a) * cpu.reg(b));
  // muli
  addop(cpu.reg(c) = cpu.reg(a) * b);
  // banr
  addop(cpu.reg(c) = cpu.reg(a) & cpu.reg(b));
  // bani
  addop(cpu.reg(c) = cpu.reg(a) & b);
  // borr
  addop(cpu.reg(c) = cpu.reg(a) | cpu.reg(b));
  // bori
  addop(cpu.reg(c) = cpu.reg(a) | b);
  // setr
  addop((void)b; cpu.reg(c) = cpu.reg(a));
  // seti
  addop((void)b; cpu.reg(c) = a);
  // gtir
  addop(cpu.reg(c) = a > cpu.reg(b) ? 1 : 0);
  // gtri
  addop(cpu.reg(c) = cpu.reg(a) > b ? 1 : 0);
  // gtrr
  addop(cpu.reg(c) = cpu.reg(a) > cpu.reg(b) ? 1 : 0);
  // eqir
  addop(cpu.reg(c) = a == cpu.reg(b) ? 1 : 0);
  // eqri
  addop(cpu.reg(c) = cpu.reg(a) == b ? 1 : 0);
  // eqrr
  addop(cpu.reg(c) = cpu.reg(a) == cpu.reg(b) ? 1 : 0);
  assert(int(instructions.size()) == nop);
}

#undef addop

bool CPU::consistent(op const &instr, observation const &test) {
  auto const & [reg_start, enc, reg_end] = test;
  registers = reg_start;
  instr(*this, enc);
  return registers == reg_end;
}

int CPU::how_many(observation const &test) {
  int could_be = 0;
  for (auto const &instr : instructions)
    if (consistent(instr, test))
      ++could_be;
  return could_be;
}

void CPU::deduce_opcodes() {
  // Find the possibility matrix: poss[instr][opcode] = is instr
  // consistent with being opcode?
  possibilities poss;
  for (int instr_num = 0; instr_num < nop; ++instr_num) {
    auto const &instr = instructions[instr_num];
    auto &this_poss = poss[instr_num];
    for (int opcode = 0; opcode < nop; ++opcode)
      this_poss[opcode] = true;
    for (auto const &test : tests) {
      auto const &enc = get<1>(test);
      this_poss[enc[0]] = this_poss[enc[0]] && consistent(instr, test);
    }
  }
  // Find the assignment.  The input is apparently set up so that a
  // single choice is available at each step and there's no need for
  // backtracking .  I had originally written a full recursive search,
  // but the generality isn't needed.
  auto choices = [&](int i) -> int {
                   return count(poss[i].begin(), poss[i].end(), true);
                 };
  map<int, int> assignment;
  for (int _ = 0; _ < nop; ++_) {
    optional<int> to_assign;
    for (int i = 0; i < nop; ++i)
      if (assignment.find(i) == assignment.end() && choices(i) == 1) {
        to_assign = i;
        break;
      }
    assert(to_assign);
    auto &passign = poss[*to_assign];
    int choice = find(passign.begin(), passign.end(), true) - passign.begin();
    assignment[*to_assign] = choice;
    for (int i = 0; i < nop; ++i)
      poss[i][choice] = false;
  }
  // Reorder instructions to match the encoding
  vector<op> new_instructions(16);
  for (int i = 0; i < 16; ++i)
    new_instructions[assignment[i]] = instructions[i];
  instructions = new_instructions;
}

int CPU::execute() {
  for (int i = 0; i < nreg; ++i)
    registers[i] = 0;
  for (auto const &enc : program)
    instructions[enc[0]](*this, enc);
  return registers[0];
}

void part1() {
  CPU cpu;
  int ans = 0;
  for (auto const &test : cpu.tests)
    if (cpu.how_many(test) >= 3)
      ++ans;
  cout << ans << '\n';
}

void part2() {
  CPU cpu;
  cpu.deduce_opcodes();
  cout << cpu.execute() << '\n';
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
