//
// Created by rozliv on 02.08.2017.
//
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "../efg.h"


using std::string;
using std::cout;
using std::vector;
using std::move;


std::vector<int> initialization() {
  srand(static_cast<unsigned int>(time(nullptr)));
  graph = std::vector<string>();
  pole = std::vector<int>();
  arrid = std::vector<int>();
  playarr = std::vector<int>();
  PursuitDomain::width_ = 3;
  count = 0;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  reward = vector<double>(loc.size());
  arrIS = vector<shared_ptr<AOH>>();
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 2);
  auto node = MakeUnique<EFGNode>(0, std::make_shared<PursuitState>(loc),
                                  vector<double>(loc.size()));
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
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

std::vector<int> play() {
  return playarr;
}

namespace py = pybind11;

PYBIND11_PLUGIN(example) {
    py::module m("example", "pybind11 example plugin");
    m.def("initialization", &initialization, py::return_value_policy::copy);
    m.def("play", &play, py::return_value_policy::copy);
    m.def("graph2", &graph2, py::return_value_policy::copy);
    m.def("array2", &array2, py::return_value_policy::copy);
    py::object world = py::cast(count);
    m.attr("what") = world;
    return m.ptr();
};
