//
// Created by rozliv on 02.08.2017.
//

//  #include <gtest/gtest.h>

#include "efg.h"


int main(int argc, char* argv[]) {
  srand(static_cast<unsigned int>(time(nullptr)));
  clock_t begin = clock();
  PursuitDomain::width_ = 3;
  countStates = 0;
  PursuitDomain::height_ = 3;
  mapa = unordered_map<size_t, vector<EFGNode>>();
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(2);
  // TODO: dostat tam MMPursuitState
//  unique_ptr<Domain> d2 = MakeUnique<PursuitDomainChance>(1);
  reward = vector<double>(d->GetMaxPlayers());
  EFGTreewalkStart(d);
//  TreewalkStart(d2);
//  PursuitStart(d2);
  for (double i : reward) {
    cout << i << " ";
  }
  cout << countStates << '\n';
  cout << mapa.size() << '\n';
  clock_t end = clock();
  double elapsed_secs = static_cast<double>(end - begin) / CLOCKS_PER_SEC;
  cout << "hotovo: time " << elapsed_secs << "s" << '\n';

//  getchar();
//  testing::InitGoogleTest(&argc, argv);
//  RUN_ALL_TESTS();
  return 0;
}
