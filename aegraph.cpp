// Copyright 2019 Luca Istrate, Danut Matei
#include "./aegraph.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

std::string strip(std::string s) {
  // deletes whitespace from the beginning and end of the string
  s.erase(0, s.find_first_not_of(" \n\r\t"));
  s.erase(s.find_last_not_of(" \n\r\t") + 1);
  return s;
}

std::pair<std::string, std::string> split_first(std::string s,
                                                char delimiter = ',') {
  // returns a pair that contains: <first_cut, rest_of_graph>

  int numOpen = 0;

  int len = s.size();
  for (int i = 0; i < len; i++) {
    char c = s[i];
    if (c == delimiter && numOpen == 0)
      return std::make_pair(strip(s.substr(0, i)), strip(s.substr(i + 1, len)));
    if (c == '[')
      numOpen += 1;
    if (c == ']')
      numOpen -= 1;
  }

  return {strip(s), std::string()};
}

std::vector<std::string> split_level(std::string s, char delimiter = ',') {
  // splits 's' into separate entities (atoms, subgraphs)

  std::vector<std::string> result;
  auto snd = s;
  while (true) {
    auto p = split_first(snd, delimiter);
    auto fst = p.first;
    snd = p.second;

    result.push_back(fst);

    if (snd.empty())
      return result;
  }
}

int AEGraph::num_subgraphs() const { return subgraphs.size(); }

int AEGraph::num_atoms() const { return atoms.size(); }

int AEGraph::size() const { return num_atoms() + num_subgraphs(); }

bool AEGraph::operator<(const AEGraph &other) const {
  return this->repr() < other.repr();
}

bool AEGraph::operator==(const AEGraph &other) const {
  return this->repr() == other.repr();
}

bool AEGraph::operator!=(const AEGraph &other) const {
  return this->repr() != other.repr();
}

AEGraph AEGraph::operator[](const int index) const {
  // offers an easier way of accessing the nested graphs
  if (index < num_subgraphs()) {
    return subgraphs[index];
  }

  if (index < num_subgraphs() + num_atoms()) {
    return AEGraph('(' + atoms[index - num_subgraphs()] + ')');
  }

  return AEGraph("()");
}

std::ostream &operator<<(std::ostream &out, const AEGraph &g) {
  out << g.repr();
  return out;
}

AEGraph::AEGraph(std::string representation) {
  // constructor that creates an AEGraph structure from a
  // serialized representation
  char left_sep = representation[0];
  char right_sep = representation[representation.size() - 1];

  assert((left_sep == '(' && right_sep == ')') ||
         (left_sep == '[' && right_sep == ']'));

  // if the left separator is '(' then the AEGraph is the entire
  // sheet of assertion
  if (left_sep == '(') {
    is_SA = true;
  } else {
    is_SA = false;
  }

  // eliminate the first pair of [] or ()
  representation = representation.substr(1, representation.size() - 2);

  // split the graph into separate elements
  auto v = split_level(representation);
  // add them to the corresponding vector
  for (auto s : v) {
    if (s[0] != '[') {
      atoms.push_back(s);
    } else {
      subgraphs.push_back(AEGraph(s));
    }
  }

  // also internally sort the new graph
  this->sort();
}

std::string AEGraph::repr() const {
  // returns the serialized representation of the AEGraph

  std::string left, right;
  if (is_SA) {
    left = '(';
    right = ')';
  } else {
    left = '[';
    right = ']';
  }

  std::string result;
  for (auto subgraph : subgraphs) {
    result += subgraph.repr() + ", ";
  }

  int len = atoms.size();
  if (len != 0) {
    for (int i = 0; i < len - 1; i++) {
      result += atoms[i] + ", ";
    }
    result += atoms[len - 1];
  } else {
    if (subgraphs.size() != 0)
      return left + result.substr(0, result.size() - 2) + right;
  }

  return left + result + right;
}

void AEGraph::sort() {
  std::sort(atoms.begin(), atoms.end());

  for (auto &sg : subgraphs) {
    sg.sort();
  }

  std::sort(subgraphs.begin(), subgraphs.end());
}

bool AEGraph::contains(const std::string other) const {
  // checks if an atom is in a graph
  if (find(atoms.begin(), atoms.end(), other) != atoms.end())
    return true;

  for (const auto &sg : subgraphs)
    if (sg.contains(other))
      return true;

  return false;
}

bool AEGraph::contains(const AEGraph &other) const {
  // checks if a subgraph is in a graph
  if (find(subgraphs.begin(), subgraphs.end(), other) != subgraphs.end())
    return true;

  for (const auto &sg : subgraphs)
    if (sg.contains(other))
      return true;

  return false;
}

std::vector<std::vector<int>>
AEGraph::get_paths_to(const std::string other) const {
  // returns all paths in the tree that lead to an atom like <other>
  std::vector<std::vector<int>> paths;

  int len_atoms = num_atoms();
  int len_subgraphs = num_subgraphs();

  for (int i = 0; i < len_atoms; i++) {
    if (atoms[i] == other && size() > 1) {
      paths.push_back({i + len_subgraphs});
    }
  }

  for (int i = 0; i < len_subgraphs; i++) {
    if (subgraphs[i].contains(other)) {
      auto r = subgraphs[i].get_paths_to(other);
      for (auto &v : r)
        v.insert(v.begin(), i);
      copy(r.begin(), r.end(), back_inserter(paths));
    }
  }

  return paths;
}

std::vector<std::vector<int>>
AEGraph::get_paths_to(const AEGraph &other) const {
  // returns all paths in the tree that lead to a subgraph like <other>
  std::vector<std::vector<int>> paths;
  int len_subgraphs = num_subgraphs();

  for (int i = 0; i < len_subgraphs; i++) {
    if (subgraphs[i] == other && size() > 1) {
      paths.push_back({i});
    } else {
      auto r = subgraphs[i].get_paths_to(other);
      for (auto &v : r)
        v.insert(v.begin(), i);
      copy(r.begin(), r.end(), back_inserter(paths));
    }
  }

  return paths;
}

std::vector<std::vector<int>> AEGraph::possible_double_cuts() const {
  std::vector<int> path;
  std::vector<std::vector<int>> res;

  possible_double_cuts_helper(*this, path, res);

  return res;
}

void AEGraph::possible_double_cuts_helper(
    const AEGraph &g, std::vector<int> &path,
    std::vector<std::vector<int>> &res) const {
  if (g.num_atoms() == 0 && !g.is_SA) {
    // possible double cut
    res.push_back(path);
  }

  if (g.num_subgraphs() == 0) {
    return;
  }

  for (int i = 0; i < g.num_subgraphs(); ++i) {
    path.push_back(i);
    possible_double_cuts_helper(g.subgraphs[i], path, res);
    path.pop_back();
  }
}

AEGraph AEGraph::double_cut(std::vector<int> where) const {
  AEGraph graphCopy = (*this);

  // we will use cutPosition to iterate trough the graph
  AEGraph cutPosition = graphCopy;

  for (unsigned int i = 0; i < where.size(); ++i) {
    cutPosition = cutPosition[where[i]];
  }

  // cut the extra parantheses
  int rightCut = cutPosition.repr().size() - 4;
  std::string afterCut = cutPosition.repr().substr(2, rightCut);

  // find position of double cut in string
  auto found = graphCopy.repr().find(cutPosition.repr());

  // do the double cut, generate a new graph and return it
  std::string newGraph =
      graphCopy.repr().replace(found, cutPosition.repr().size(), afterCut);

  return AEGraph(newGraph);
}

void AEGraph::possible_erasures_helper(const AEGraph &g, std::vector<int> &path,
                                       std::vector<std::vector<int>> &res,
                                       int brothers = 0,
                                       bool calledFromSA = true) const {
  if (path.size() % 2 == 1 &&
      (brothers >= 1 || (brothers == 0 && calledFromSA))) {
    res.push_back(path);
  }
  for (int i = 0; i < g.num_subgraphs(); ++i) {
    path.push_back(i);

    // nr de copii dev nr de frati ai unui copil
    int brothers = g.num_subgraphs() + g.num_atoms() - 1; 
    
    possible_erasures_helper(g.subgraphs[i], path, res, brothers, g.is_SA);
    path.pop_back();
  }
  for (int i = 0; i < g.num_atoms(); ++i) {
    path.push_back(i + g.num_subgraphs());
    if (path.size() % 2 == 1 && g.num_subgraphs() + g.num_atoms() > 1 &&
        !g.is_SA) {
      res.push_back(path);
    } else if (g.is_SA) {
      res.push_back(path);
    }
    path.pop_back();
  }
}

std::vector<std::vector<int>> AEGraph::possible_erasures(int level) const {
  std::vector<int> path;
  std::vector<std::vector<int>> res;

  if(this->repr() == "()") return res;

  possible_erasures_helper(*this, path, res);

  return res;
}

AEGraph AEGraph::erase(std::vector<int> where) const {
    AEGraph graphCopy = (*this);
    // we will use cutPosition to iterate trough the graph
    AEGraph cutPosition = graphCopy, parent("()");
    
    for(int i = 0; i < where.size(); ++i) {
        parent = cutPosition;
        cutPosition = cutPosition[where[i]];
    }

    // we search for the erasure part in our graf
    std::string toBeFound;
    if (cutPosition.repr()[0] == '(') toBeFound = cutPosition.repr().substr (1, cutPosition.repr().length() - 2);
    else toBeFound = cutPosition.repr();
    std::size_t found = (*this).repr().find(toBeFound);

    // we split the graf into one left and one right part excepting erasured part
    std::string leftCut = (*this).repr().substr (0,found);
    std::string rightCut;
    if ((*this).repr()[found + 1] != ',') rightCut = (*this).repr().substr (found + toBeFound.length(), (*this).repr().length() - 1);
    else rightCut = (*this).repr().substr (found + toBeFound.length() + 1, (*this).repr().length() - 1);

    // we fix remaining errors like consecutive two commas 
    if (leftCut[leftCut.size()-1] == ' ' && rightCut[0] == ' ') {
        leftCut.pop_back();
    } 
    else if (leftCut[leftCut.size()-1] == ' ' && rightCut[0] == ',') {
            rightCut.erase(0,2);
         }
         else if (leftCut[leftCut.size()-1] == ' ' && rightCut[0] == ')') {
                leftCut.pop_back();
                leftCut.pop_back();
              }
              else if (leftCut[leftCut.size()-1] == '(' && rightCut[0] == ',') {
                           rightCut.erase(0,2); 
                   }   
    std::string extracted = leftCut + rightCut;
    return AEGraph(extracted);
    //return this->deiterate(where);
}

std::vector<std::vector<int>> AEGraph::possible_deiterations() const {
  std::vector<std::vector<int>> res;

  for (int i = 0; i < num_atoms() + num_subgraphs(); ++i) {
    // check all subgraphs/atoms except the one we are currently at
    for (int j = 0; j < num_subgraphs(); ++j) {
      if (subgraphs[j] != (*this)[i]) { 
        
        // treat subgraph case
        if (i < num_subgraphs() && subgraphs[j].contains((*this)[i])) {

          // get all paths to the subgraph
          std::vector<std::vector<int>> paths =
              subgraphs[j].get_paths_to((*this)[i]);

          // add all paths to the results
          for (auto auxPath : paths) {
            // recalculate full path
            std::vector<int> fullPath;
            fullPath.push_back(j);
            fullPath.insert(fullPath.end(), auxPath.begin(), auxPath.end());
            res.push_back(fullPath);
          }
        }

        // treat atom case
        if (i >= num_subgraphs() && i < num_atoms() + num_subgraphs() &&
            subgraphs[j].contains(atoms[i - num_subgraphs()])) {

          // get all paths to the atom
          std::vector<std::vector<int>> paths =
              subgraphs[j].get_paths_to(atoms[i - num_subgraphs()]);

          // add all paths to the results
          for (auto auxPath : paths) {
            // recalculate full path
            std::vector<int> fullPath;
            fullPath.push_back(j);
            fullPath.insert(fullPath.end(), auxPath.begin(), auxPath.end());
            res.push_back(fullPath);
          }
        }
      }
    }
  }

  return res;
}

AEGraph AEGraph::deiterate(std::vector<int> where) const {

  AEGraph graphCopy = (*this);
  // we will use cutPosition to iterate trough the graph
  AEGraph cutPosition = graphCopy, parent("()");
  int paddingLeft = 0, paddingRight = 0;

  for(int i = 0; i < where.size(); ++i) {
      parent = cutPosition;
      cutPosition = cutPosition[where[i]];
  }

  std::string toBeFound;
  if (cutPosition.repr()[0] == '(') toBeFound = cutPosition.repr().substr (1, cutPosition.repr().length() - 2);
  else toBeFound = cutPosition.repr();
  
  auto found = parent.repr().find(toBeFound);  

  if(parent.repr()[found-1] == ' ') {
    paddingLeft = 1;
  }

  if(parent.repr()[found+toBeFound.length()] == ',') {
    paddingRight = 2;
  }

  std::string newParent = parent.repr().replace(found-paddingLeft, paddingLeft+toBeFound.length()+paddingRight,"");
  
  auto foundParent = graphCopy.repr().find(parent.repr());
  std::string newGraph = graphCopy.repr().replace(foundParent, parent.repr().length(), newParent);

  return AEGraph(newGraph);
}
