// -*- C++ -*-
// g++ -std=c++17 -Wall -g -o doit doit.cc
// ./doit 1 < input  # part 1
// ./doit 2 < input  # part 2

#include <iostream>
#include <vector>
#include <optional>
#include <numeric>
#include <cassert>

using namespace std;

struct node {
  vector<node> children;
  vector<int> metadata;

  node();

  int total_metadata() const;
  int value() const;
};

node::node() {
  int num_children, num_metadata;
  cin >> num_children >> num_metadata;
  for (int _ = 0; _ < num_children; ++_)
    children.emplace_back();
  for (int _ = 0; _ < num_metadata; ++_) {
    int data;
    cin >> data;
    metadata.push_back(data);
  }
}

int node::total_metadata() const {
  int result = accumulate(metadata.begin(), metadata.end(), 0);
  for (auto const &child : children)
    result += child.total_metadata();
  return result;
}

int node::value() const {
  if (children.empty())
    return accumulate(metadata.begin(), metadata.end(), 0);
  vector<optional<int>> children_values(children.size());
  int result = 0;
  for (int index : metadata) {
    --index;
    if (index < 0 || index >= int(children.size()))
      continue;
    if (!children_values[index])
      children_values[index] = children[index].value();
    result += *children_values[index];
  }
  return result;
}

void part1() { cout << node().total_metadata() << '\n'; }
void part2() { cout << node().value() << '\n'; }

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
