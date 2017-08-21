//
// Created by rozliv on 02.08.2017.
//

#include "efg.h"
// #include <gtest/gtest.h>
// #include <gmock/gmock.h>

int main(int argc, char* argv[]) {
  srand(static_cast<unsigned int>(time(nullptr)));
  clock_t begin = clock();
  PursuitDomain::width_ = 3;
  count = 0;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  reward = vector<double>(loc.size());
  arrIS = vector<shared_ptr<AOH>>();
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 2);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0,
                                                 std::make_shared<PursuitState>(loc),
                                                 vector<double>(loc.size()));
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
//  Treewalk(d, d->GetRoot().get(), d->GetMaxDepth(), d->GetMaxPlayers());
//  Pursuit(d, d->GetRoot().get(), d->GetMaxDepth(), d->GetMaxPlayers());
  for (double i : reward) {
    cout << i << " ";
  }
  cout << count << '\n';
  cout << arrIS.size() << '\n';
  clock_t end = clock();
  double elapsed_secs = static_cast<double>(end - begin) / CLOCKS_PER_SEC;
  cout << "hotovo: time " << elapsed_secs << "s" << '\n';

  // testing::InitGoogleTest(&argc, argv);
  // RUN_ALL_TESTS();
  return 0;
}
