//
// Created by Matej Sestak on 11.3.19.
//


#include "algorithms/treeWalk.h"
#include "algorithms/utility.h"
#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"


#include <boost/test/unit_test.hpp>
#include "domains/oshiZumo.h"

using namespace GTLib2;


BOOST_AUTO_TEST_SUITE(OshiZumo)

BOOST_AUTO_TEST_CASE(numberOfNodes1031) {
    domains::OshiZumoDomain domain(10, 3, 1);
    auto nodes = algorithms::countNodes(domain);
    BOOST_CHECK(nodes == 621405);
}

BOOST_AUTO_TEST_CASE(numberOfInformationSets1572){
    domains::OshiZumoDomain domain(15, 7, 2);
    int p1 = domain.getPlayers()[0];
    int p2 = domain.getPlayers()[1];

    auto p1InfoSetAndActions = algorithms::generateInformationSetsAndAvailableActions(domain, p1);
    auto p2InfoSetAndActions = algorithms::generateInformationSetsAndAvailableActions(domain, p2);

    auto numInfoSetsP1 = p1InfoSetAndActions.size();
    auto numInfoSetsP2 = p2InfoSetAndActions.size();
    BOOST_CHECK(numInfoSetsP1 == 219565 && numInfoSetsP2 == 219565);
}

BOOST_AUTO_TEST_CASE(CFR100iter831){
    domains::OshiZumoDomain domain(8, 3, 1);
        auto regrets = algorithms::CFRiterations(domain, 100);
        auto strat1 = algorithms::getStrategyFor(domain, domain.getPlayers()[0], regrets);
        auto strat2 = algorithms::getStrategyFor(domain, domain.getPlayers()[1], regrets);
        auto bestResp1 = algorithms::bestResponseTo(strat2, domain.getPlayers()[1],
                                                    domain.getPlayers()[0], domain).second;
        auto bestResp2 =  algorithms::bestResponseTo(strat1, domain.getPlayers()[0],
                                                     domain.getPlayers()[1], domain).second;
        double utility =  algorithms::computeUtilityTwoPlayersGame(domain, strat1, strat2,
                                                                   domain.getPlayers()[0], domain.getPlayers()[1]).first;

        BOOST_CHECK(std::abs(utility + 0.010035) <= 0.0001);
        BOOST_CHECK(std::abs(bestResp2 - 0.047110) <= 0.0001);
        BOOST_CHECK(std::abs(bestResp1 - 0.011856) <= 0.0001);
}


BOOST_AUTO_TEST_SUITE_END()