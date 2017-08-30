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
    count = 0;
    arrIS = vector<shared_ptr<InfSet>>();
  }
};

TEST_F(FTest, Depth2PROP33) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 2);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<PursuitState>(loc, 1),
                                                 vector<double>(loc.size()));

  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 6);
  ASSERT_EQ(reward[1], -6);
  ASSERT_EQ(count, 416);
  ASSERT_EQ(arrIS.size(), 14);
}

TEST_F(FTest, Depth3PROP44) {
  PursuitDomain::width_ = 4;
  PursuitDomain::height_ = 4;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 3);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<PursuitState>(loc, 1),
                                                 vector<double>(loc.size()));

  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 20);
  ASSERT_EQ(reward[1], -20);
  ASSERT_EQ(count, 12960);
  ASSERT_EQ(arrIS.size(), 58);
}

TEST_F(FTest, Depth3PROP33) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 3);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<PursuitState>(loc, 1),
                                                 vector<double>(loc.size()));
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 1064);
  ASSERT_EQ(reward[1], -1064);
  ASSERT_EQ(count, 12080);
  ASSERT_EQ(arrIS.size(), 206);
}

TEST_F(FTest, MMDepth2PROP33) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 2);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
                                                 vector<double>(loc.size()));
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 0);
  ASSERT_EQ(reward[1], 0);
  ASSERT_EQ(count, 24);  //  MMPursuitState::PerformAction: count = 2; each player has one move, then plays the second one
  ASSERT_EQ(arrIS.size(),4);
}


TEST_F(FTest, MMDepth3PROP44) {
  PursuitDomain::width_ = 4;
  PursuitDomain::height_ = 4;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 3);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
                                                 vector<double>(loc.size()));
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 0);
  ASSERT_EQ(reward[1], 0);
  ASSERT_EQ(count, 132);  //  MMPursuitState::PerformAction: count = 2; each player has one move, then plays the second one
  ASSERT_EQ(arrIS.size(),15);
}

TEST_F(FTest, MMDepth4PROP44) {
  PursuitDomain::width_ = 3;
  PursuitDomain::height_ = 3;
  vector<Pos> loc = {{0, 0},
                     {PursuitDomain::height_ - 1, PursuitDomain::width_ - 1}};
  unique_ptr<Domain> d = MakeUnique<PursuitDomain>(loc, loc.size(), 4);
  unique_ptr<EFGNode> node = MakeUnique<EFGNode>(0, std::make_shared<MMPursuitState>(loc, vector<bool>({true, false}), 1),
                                                 vector<double>(loc.size()));
  EFGTreewalk(d, node.get(), d->GetMaxDepth(), 1, {});
  ASSERT_EQ(reward[0], 96);
  ASSERT_EQ(reward[1], -96);
  ASSERT_EQ(count, 696);  //  MMPursuitState::PerformAction: count = 2; each player has one move, then plays the second one
  ASSERT_EQ(arrIS.size(),74);
}