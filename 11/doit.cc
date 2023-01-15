// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <array>
#include <cassert>

using namespace std;

int const n = 300;

struct grid {
  // Computed power levels
  array<array<int, n>, n> power;
  // Partial sums of power[][]; accum[i][j] is the sum of
  // power[0...i][0...j]
  array<array<int, n>, n> accum;

  // Reads serial number from stdin
  grid();

  // Power in a square of size sz with corner at (x, y).  NB: x and y
  // are normal 0-indexed
  int square_power(int x, int y, int sz) const;

  // Find the largest total power of any square of size sz.  Return
  // the 1-indexed (x, y) of the square's corner and the square's
  // power..
  pair<pair<int, int>, int> largest_square(int sz) const;
};

grid::grid() {
  int serial_num;
  cin >> serial_num;
  for (int x = 1; x <= n; ++x)
    for (int y = 1; y <= n; ++y) {
      int rack_id = x + 10;
      int power_level = rack_id * y;
      power_level += serial_num;
      power_level *= rack_id;
      power_level %= 1000;
      power_level /= 100;
      power_level -= 5;
      power[x - 1][y - 1] = power_level;
    }
  for (int i=0; i< n; ++i)  for (int j=0; j< n; ++j) accum[i][j]=-9;
  for (int i = 0; i < n; ++i) {
    int jsum = power[i][0];
    accum[i][0] = (i > 0 ? accum[i - 1][0] : 0) + jsum;
    for (int j = 1; j < n; ++j) {
      jsum += power[i][j];
      accum[i][j] = (i > 0 ? accum[i - 1][j] : 0) + jsum;
    }
  }
}

// To find the sum in the square S, start with the sum from accum at
// the corner C.  If the top rectangle with corner A is nonempty,
// subtract off that.  If the left rectangle with corner B is
// nonempty, subtract off that too.  If both rectangles are nonempty,
// we've then subtracted the rectangle ending at (x, y) twice, so add
// it back once.
//
// +-------+--------------+
// |   +   |      -       |
// +----(x, y)------------A
// |       |              |
// |       |              |
// |   -   |     S = +   sz
// |       |              |
// |       |              |
// |       |              |
// +-------B------sz------C
//
// Note: 0-indexed coordinates here
int grid::square_power(int x, int y, int sz) const {
  int total = accum[x + (sz - 1)][y + (sz - 1)];
  if (x > 0)
    total -= accum[x - 1][y + (sz - 1)];
  if (y > 0)
    total -= accum[x + (sz - 1)][y - 1];
  if (x > 0 && y > 0)
    total += accum[x - 1][y - 1];
  return total;
}

pair<pair<int, int>, int> grid::largest_square(int sz) const {
  assert(sz <= n);
  pair<int, int> result{ 0, 0 };
  int largest = square_power(result.first, result.second, sz);
  for (int x = 0; x + sz - 1 < n; ++x)
    for (int y = 0; y + sz - 1 < n; ++y) {
      int this_power = square_power(x, y, sz);
      if (largest < this_power) {
        largest = this_power;
        result = { x, y };
      }
    }
  // Return 1-indexed coordinates to match the problem output
  return { { result.first + 1, result.second + 1 }, largest };
}

void part1() {
  auto [x, y] = grid().largest_square(3).first;
  cout << x << ',' << y << '\n';
}

void part2() {
  grid g;
  auto [best_xy, best_power] = g.largest_square(1);
  int best_sz = 1;
  for (int sz = 2; sz <= n; ++sz) {
    auto [this_xy, this_power] = g.largest_square(sz);
    if (best_power < this_power) {
      best_power = this_power;
      best_xy = this_xy;
      best_sz = sz;
    }
  }
  auto [x, y] = best_xy;
  cout << x << ',' << y << ',' << best_sz << '\n';
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
