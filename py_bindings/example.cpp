/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "base/efg.h"


using std::string;
using cout;
using vector;
using move;

vector<int> parents = vector<int>();  // temporary for python graphs
vector<string> graph = vector<string>();  // temporary for python graphs
vector<int> pole = vector<int>();  // temporary for python graphs
vector<int> arrid = vector<int>();  // temporary for python graphs


/* Domain independent extensive form game treewalk algorithm,
 * adjusted to graphs made by python script. */
void EFGTreewalkG(const unique_ptr<Domain>& domain, EFGNode *node,
                  int depth, int players,
                  const vector<shared_ptr<Action>>& list, int parent) {
  if (node == nullptr) {
    throw("Node is NULL");
  }
  parents.push_back(parent);  // temporary for python graphs
  pole.push_back(depth);  // temporary for python graphs

  if (depth == 0) {
    node->IS = -1;
    arrid.push_back(node->IS);  // temporary for python graphs
    graph.emplace_back("leaf");  // temporary for python graphs
    return;
  }
  unsigned int l = 0;
  vector<shared_ptr<Action>> actions = node->GetAction();
  for (l = 0; l < arrIS.size(); ++l) {
    if (*arrIS[l] == *node->GetIS()) {
      node->IS = l;
      break;
    }
  }
  if (arrIS.empty() || l == arrIS.size()) {
    arrIS.push_back(make_shared<AOH>(node->GetPlayer(),
                                          node->GetState()->GetAOH()[node->GetPlayer()]));
    node->IS = arrIS.size() - 1;
  }

  arrid.push_back(node->IS);  // temporary for python graphs
  // temporary for python graphs
  graph.push_back("InfSet id: " + to_string(node->IS)+
                  "  " +node->GetState()->ToString(node->GetPlayer()));
  int x = arrid.size()-1;
  for (auto &i : actions) {
    unique_ptr<EFGNode> n = node->PerformAction(i);
    vector<shared_ptr<Action>> locallist = list;
    locallist.push_back(i);
    int actionssize = locallist.size();
    if (players == node->GetState()->GetNumPlayers()) {
      while (node->GetPlayer() >= actionssize) {
        locallist.insert(locallist.begin(), make_shared<Action>(NoA));
        ++actionssize;
      }
      while (domain->GetMaxPlayers() > locallist.size()) {
        locallist.push_back(make_shared<Action>(NoA));
      }
      // if all players play in this turn, returns a ProbDistribution
      ProbDistribution prob = n->GetState()->PerformAction(locallist);
      ChanceNode chan(&prob, locallist, n);
      parents.push_back(x);   // temporary for python graphs
      pole.push_back(depth);  // temporary for python graphs
      arrid.push_back(-1);  // temporary for python graphs
      graph.emplace_back("ChanceNode");  // temporary for python graphs
      vector<unique_ptr<EFGNode>> vec = chan.GetAll();
      int y = arrid.size();
      for (auto &j : vec) {
        count++;
        for (unsigned int k = 0; k < reward.size(); ++k) {
          reward[k] += j->GetRewards()[k];
        }
        EFGTreewalkG(domain, j.get(), depth - 1, 1, {}, y-1);
      }
    } else {
      EFGTreewalkG(domain, n.get(), depth, players + 1, locallist, x);
    }
  }
}


void EFGTreewalkGStart(const unique_ptr<Domain>& domain, int depth = 0) {
  if (depth == 0)
    depth = domain->GetMaxDepth();
  ChanceNode chan(domain->GetRoot().get());
  parents.push_back(-1);   // temporary for python graphs
  pole.push_back(depth);  // temporary for python graphs
  arrid.push_back(-1);  // temporary for python graphs
  graph.emplace_back("ChanceNode");  // temporary for python graphs
  vector<unique_ptr<EFGNode>> vec = chan.GetAll();
  for (auto &j : vec) {
    count++;
    for (unsigned int k = 0; k < reward.size(); ++k) {
      reward[k] += j->GetRewards()[k];
    }
    EFGTreewalkG(domain, j.get(), depth, 1, {}, 0);
  }
}

vector<int> initialization() {
  srand(static_cast<unsigned int>(time(nullptr)));
  graph = vector<string>();
  pole = vector<int>();
  arrid = vector<int>();
  PursuitDomain::width_ = 3;
  count = 0;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  reward = vector<double>(loc.size());
  arrIS = vector<shared_ptr<InfSet>>();
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(2, loc.size(), loc);
  auto node = MakeUnique<EFGNode>(0, make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
                                  vector<double>(loc.size()));
  unique_ptr<Domain> d2 = MakeUnique<PursuitDomainChance>(1);
//  EFGTreewalkGStart(d2);
  EFGTreewalkG(d, node.get(), d->GetMaxDepth(), 1, {}, -1);
//  Treewalk(d, d->GetRoot().get(), d->GetMaxDepth(), d->GetMaxPlayers());
//  Pursuit(d, d->GetRoot().get(), d->GetMaxDepth(), d->GetMaxPlayers());
  for (double i : reward) {
    cout << i << " ";
  }
  cout << count << '\n';
  cout << arrIS.size() << '\n';
  return pole;
}

vector<string> graph2() {
    return graph;
}

vector<int> array2() {
  return arrid;
}

vector<int> par2() {
  return parents;
}

namespace py = pybind11;

PYBIND11_PLUGIN(example) {
    py::module m("example", "pybind11 example plugin");
    m.def("initialization", &initialization, py::return_value_policy::copy);
    m.def("graph2", &graph2, py::return_value_policy::copy);
    m.def("array2", &array2, py::return_value_policy::copy);
    m.def("par2", &par2, py::return_value_policy::copy);
    py::object world = py::cast(count);
    m.attr("what") = world;
    return m.ptr();
};
