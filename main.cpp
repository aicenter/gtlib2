//
// Created by rozliv on 02.08.2017.
//
#include "pursuit.h"
// #include <gtest/gtest.h>
// #include <gmock/gmock.h>


int main(int argc, char* argv[]) {
    srand(static_cast<unsigned int>(time(nullptr)));
    clock_t begin = clock();
    PursuitDomain::width_ = 3;
    count = 0;
    PursuitDomain::height_ = 3;
    vector<Pos> loc = {{0, 0}, {PursuitDomain::height_-1, PursuitDomain::width_-1}};
    rewards = vector<double>(loc.size());
    unique_ptr<State>s = make_unique<PursuitState>(loc);
    PursuitDomain d = PursuitDomain(2, s, 2);
    pursuit(d, d.getRoot(), d.getMaxDepth());
    for (double i : rewards) {
        cout << i << " ";
    }
    cout << count << '\n';
    clock_t end = clock();
    double elapsed_secs = static_cast<double>(end - begin) / CLOCKS_PER_SEC;
    cout << "hotovo: time " << elapsed_secs << "s" << '\n';
    // testing::InitGoogleTest(&argc, argv);
    // RUN_ALL_TESTS();

    return 0;
}
