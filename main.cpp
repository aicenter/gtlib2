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
  shared_ptr<Domain> d = make_shared<PursuitDomain>(1);
//  shared_ptr<Domain> d2 = make_shared<PursuitDomainChance>(2);
  reward = vector<double>(d->GetMaxPlayers());
  NormalFormLP nor(d);
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
