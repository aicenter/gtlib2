//
// Created by rozliv on 02.08.2017.
//

//#include <gtest/gtest.h>

#include "efg.h"
#include "normalFormLP.h"

int main(int argc, char* argv[]) {
  srand(static_cast<unsigned int>(time(nullptr)));
  clock_t begin = clock();
  PursuitDomain::width_ = 2;
  countStates = 0;
  PursuitDomain::height_ = 2;
  mapa = unordered_map<size_t, vector<EFGNode>>();
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  auto st = std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1);
  shared_ptr<Domain> d = make_shared<PursuitDomain>(2, loc.size(), st);
//  shared_ptr<Domain> d2 = make_shared<PursuitDomainChance>(1);
  reward = vector<double>(d->GetMaxPlayers());
  NormalFormLP nor(d);
  cout << "vysledek hry: " << nor.SolveGame() << "\n";
//  EFGTreewalkStart(d);
//  TreewalkStart(d);
//  PursuitStart(d);
  for (double i : reward) {
    cout << i << " ";
  }
  cout << countStates << '\n';
  cout << mapa.size() << '\n';
  int s = 0;
  for (auto &i : mapa) {
    s+= i.second.size();
  }
  cout << s <<"\n";
  clock_t end = clock();
  double elapsed_secs = static_cast<double>(end - begin) / CLOCKS_PER_SEC;
  cout << "hotovo: time " << elapsed_secs << "s" << '\n';

//  getchar();
//  testing::InitGoogleTest(&argc, argv);
//  cout << RUN_ALL_TESTS();
  return 0;
}
