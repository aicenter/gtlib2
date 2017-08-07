//
// Created by Jacob on 03.08.2017.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../pursuit.h"

using testing::Eq;

namespace {
    class FTest : public testing::Test {

    public:
        std::vector<Pos> loc = {{0, 0},{5, 5}};
        std::vector<int> rewards = std::vector<int>(loc.size());
        PursuitState s = PursuitState(loc);
        PursuitDomain d = PursuitDomain(6,6,5,s);

        void pursuit(PursuitDomain& domain,const PursuitState& state, int depth)
        {
            std::vector<Pos> m = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}};
            vector<int> array = vector<int>();
            vector<Action> actions = vector<Action>();
            vector<Action> actions2 = vector<Action>();
            if(depth == 0){return;}

            state.getActions(actions,1, domain.getHeight(), domain.getWidth());
            state.getActions(actions2,2, domain.getHeight(), domain.getWidth());

            for (Action &action : actions) {
                for (Action &j : actions2) {
                    vector<Action> base = {action, j};
                    PursuitOutcome o = state.PerformAction(base);
                    for (int i = 0; i < rewards.size(); ++i) {
                        rewards[i] += o.getReward()[i];
                    }
                    pursuit(domain, o.getState(), depth-1);
                }
            }
        }
        FTest() {
            pursuit(d,s,d.getMaxDepth());
        }
    };
}

TEST_F(FTest, FirstTest) {
    ASSERT_EQ(rewards[0], -3);
}

TEST_F(FTest, SecondTest) {
   // pursuit(d,s,d.getMaxDepth());
    ASSERT_EQ(rewards[1], -6);
}