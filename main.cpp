//
// Created by rozliv on 02.08.2017.
//
#include "pursuit.h"
//#include <gtest/gtest.h>
//#include <gmock/gmock.h>




int main(int argc,char* argv[])
{
    PursuitDomain::width_ = 3;
    count = 0;
    PursuitDomain::height_ = 3;
    vector<Pos> loc = {{0, 0},{PursuitDomain::height_-1, PursuitDomain::width_-1}};
    rewards = std::vector<double>(loc.size());
    std::unique_ptr<State> s {new PursuitState(loc)};
    PursuitDomain d = PursuitDomain(2,s);
    pursuit(d,s, d.getMaxDepth());
//    PursuitAction  a = PursuitAction(1);
//    std::vector<Action> list2 = std::vector<Action>();
//    std::vector<std::unique_ptr<Action>> list = std::vector<std::unique_ptr<Action>>();
//    list.emplace_back(new PursuitAction(1));
//    list2.push_back(a);
//    cout<<list2.back().getInfo() << " ";
//    cout<< list.back()->getInfo();
    for (double i : rewards) {
        cout << i << " ";
    }
    cout << count << '\n';
   // testing::InitGoogleTest(&argc, argv);
   // RUN_ALL_TESTS();

    return 0;
}