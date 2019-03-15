//
// Created by rozliv on 02.08.2017.
//
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "base/efg.h"


using std::string;
using std::cout;
using std::vector;
using std::move;

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
    arrIS.push_back(std::make_shared<AOH>(node->GetPlayer(),
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
        locallist.insert(locallist.begin(), std::make_shared<Action>(NoA));
        ++actionssize;
      }
      while (domain->GetMaxPlayers() > locallist.size()) {
        locallist.push_back(std::make_shared<Action>(NoA));
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

std::vector<int> initialization() {
  srand(static_cast<unsigned int>(time(nullptr)));
  graph = std::vector<string>();
  pole = std::vector<int>();
  arrid = std::vector<int>();
  PursuitDomain::width_ = 3;
  count = 0;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  reward = vector<double>(loc.size());
  arrIS = vector<shared_ptr<InfSet>>();
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(2, loc.size(), loc);
  auto node = MakeUnique<EFGNode>(0, std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
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

std::vector<string> graph2() {
    return graph;
}

std::vector<int> array2() {
  return arrid;
}

std::vector<int> par2() {
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
