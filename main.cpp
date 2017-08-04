//
// Created by rozliv on 02.08.2017.
//
#include "pursuit.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using std::string;
using std::cout;
using std::endl;
using std::vector;
using std::move;

std::vector<int> rewards;
int count = 0;
void pursuit(PursuitDomain& domain,PursuitState state, int depth)
{
    std::vector<Pos> m = {{0,0}, {1,0}, {0,1}, {-1,0}, {0,-1}};
    vector<int> array = vector<int>();
    vector<Action> actions = vector<Action>();
    vector<Action> actions2 = vector<Action>();
//    cout << "hloubka je: " << depth << "   ";
//    cout<< "1. " << state.getPlace()[0].x << " " << state.getPlace()[0].y;
//    cout<< "    2. " << state.getPlace()[1].x << " " << state.getPlace()[1].y << endl;
    ++count;
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
            pursuit(domain, o.getState(), depth - 1);
        }
    }
}

int main(int argc,char* argv[])
{
    srand(static_cast<unsigned int>(time(nullptr)));
    vector<Pos> loc = {{0, 0},{2, 2}};
    rewards = std::vector<int>(loc.size());
    PursuitState s = PursuitState(loc);
    PursuitDomain d = PursuitDomain(3,3,2,s);
    pursuit(d,s, d.getMaxDepth());
    for(auto i:rewards){
        cout<< i << " ";
    }
    cout << count << endl;
  //  testing::InitGoogleTest(&argc, argv);
  //  RUN_ALL_TESTS();
    return 0;
}