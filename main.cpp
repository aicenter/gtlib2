//
// Created by rozliv on 02.08.2017.
//
#include "pursuit.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>




int main(int argc,char* argv[])
{
    int width = 3;
    count = 0;
    int height = 3;
    vector<Pos> loc = {{0, 0},{height-1, width-1}};
    rewards = std::vector<int>(loc.size());
    PursuitState s = PursuitState(loc);
    PursuitDomain d = PursuitDomain(height,width,2,s);
    pursuit(d,s, d.getMaxDepth());

    for (int i : rewards) {
        cout << i << " ";
    }
    cout << count << '\n';
   // testing::InitGoogleTest(&argc, argv);
   // RUN_ALL_TESTS();

    return 0;
}