//
// Created by rozliv on 02.08.2017.
//
#include "pursuit.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>




int main(int argc,char* argv[])
{
    PursuitDomain::width = 3;
    count = 0;
    PursuitDomain::height = 3;
    vector<Pos> loc = {{0, 0},{PursuitDomain::height-1, PursuitDomain::width-1}};
    rewards = std::vector<double>(loc.size());
    PursuitState s = PursuitState(loc);
    PursuitDomain d = PursuitDomain(2,s);
    pursuit(d,s, d.getMaxDepth());

    for (double i : rewards) {
        cout << i << " ";
    }
    cout << count << '\n';
   // testing::InitGoogleTest(&argc, argv);
   // RUN_ALL_TESTS();

    return 0;
}