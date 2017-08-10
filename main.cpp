//
// Created by rozliv on 02.08.2017.
//
#include "pursuit.h"
//#include <gtest/gtest.h>
//#include <gmock/gmock.h>
#include <list>



int main(int argc,char* argv[])
{
    clock_t begin = clock();
    PursuitDomain::width_ = 3;
    count = 0;
    PursuitDomain::height_ = 3;
//    for (int k = 0; k < 16; ++k) {
//        for (int j = 3; j >= 0; --j) {
//            cout << ((k >> j) & 1) << " ";
//        }
//        cout << "\n";
//    }
    vector<Pos> loc = {{0, 0},{PursuitDomain::height_-1, PursuitDomain::width_-1}};
    rewards = std::vector<double>(loc.size());
    std::unique_ptr<State>s = make_unique<PursuitState>(loc);
    PursuitDomain d = PursuitDomain(2,s);
    pursuit(d,d.getRoot(), d.getMaxDepth());
//    PursuitAction  a = PursuitAction(1,1);
//    std::list<Action> list = std::list<Action>();
//    list.emplace_back(9);
//    list.back() = a;
//    list.push_back(PursuitAction(1,1));

  //  cout << s.use_count()<<'\n';
//    PursuitAction  a = PursuitAction(1);
//    std::vector<Action> list2 = std::vector<Action>();
//    list2.push_back(a);
//    std::vector<std::unique_ptr<Action>> list = std::vector<std::unique_ptr<Action>>();
//    list.emplace_back(new PursuitAction(1));
//
//    cout<<list2.back().getInfo() << " ";
//    cout<< list.back()->getInfo() << '\n';
//
//    PursuitAction b = PursuitAction(2);
//    Action c = static_cast<Action> (b);
//    cout<< c.getInfo() << '\n';
    for (double i : rewards) {
        cout << i << " ";
    }
    cout << count << '\n';
    clock_t end = clock();
    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    std::cout << "hotovo: time " << elapsed_secs << "s" << std::endl;
    // testing::InitGoogleTest(&argc, argv);
    // RUN_ALL_TESTS();

    return 0;
}