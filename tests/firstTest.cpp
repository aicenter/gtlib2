//
// Created by Jacob on 03.08.2017.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../efg.h"

using testing::Eq;

class FTest : public testing::Test {
 public:
  FTest() {
    reward = std::vector<double>(2);
    countStates = 0;
    mapa = unordered_map<size_t, vector<EFGNode>>();
  }
};

TEST_F(FTest, Depth2PROP33) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  shared_ptr<Domain> d = make_shared<PursuitDomain>(2, loc.size(), loc);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<PursuitState>(loc, 1),
                                                 vector<double>(loc.size()), nullptr);

  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 6);
  ASSERT_EQ(reward[1], -6);
  ASSERT_EQ(countStates, 416);
  ASSERT_EQ(mapa.size(), 14);
}

TEST_F(FTest, Depth3PROP44) {
  PursuitDomain::width_ = 4;
  PursuitDomain::height_ = 4;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  shared_ptr<Domain> d = make_shared<PursuitDomain>(3, loc.size(), loc);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<PursuitState>(loc, 1),
                                                 vector<double>(loc.size()), nullptr);

  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 20);
  ASSERT_EQ(reward[1], -20);
  ASSERT_EQ(countStates, 12960);
  ASSERT_EQ(mapa.size(), 58);
}

TEST_F(FTest, Depth3PROP33) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  shared_ptr<Domain> d = make_shared<PursuitDomain>(3, loc.size(), loc);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<PursuitState>(loc, 1),
                                                 vector<double>(loc.size()), nullptr);
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 1064);
  ASSERT_EQ(reward[1], -1064);
  ASSERT_EQ(countStates, 12080);
  ASSERT_EQ(mapa.size(), 218);
}

TEST_F(FTest, MMDepth2PROP33) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  shared_ptr<Domain> d = make_shared<PursuitDomain>(2, loc.size(), loc);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
                                                 vector<double>(loc.size()), nullptr);
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 0);
  ASSERT_EQ(reward[1], 0);
  /*  MMPursuitState::PerformAction: count = 2;
   * each player has one move, then plays the second one
   * it is a turn based game */
  ASSERT_EQ(countStates, 24);
  ASSERT_EQ(mapa.size(), 4);
}


TEST_F(FTest, MMDepth3PROP44) {
  PursuitDomain::width_ = 4;
  PursuitDomain::height_ = 4;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  shared_ptr<Domain> d = make_shared<PursuitDomain>(3, loc.size(), loc);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
                                                 vector<double>(loc.size()), nullptr);
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 0);
  ASSERT_EQ(reward[1], 0);
  /*  MMPursuitState::PerformAction: count = 2;
   * each player has one move, then plays the second one
   * it is a turn based game */
  ASSERT_EQ(countStates, 132);
  ASSERT_EQ(mapa.size(), 15);
}

TEST_F(FTest, MMDepth4PROP44) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  shared_ptr<Domain> d = make_shared<PursuitDomain>(4, loc.size(), loc);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
                                                 vector<double>(loc.size()), nullptr);
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 96);
  ASSERT_EQ(reward[1], -96);
  /*  MMPursuitState::PerformAction: count = 2;
   * each player has one move, then plays the second one
   * it is a turn based game */
  ASSERT_EQ(countStates, 696);
  ASSERT_EQ(mapa.size(), 102);
}
