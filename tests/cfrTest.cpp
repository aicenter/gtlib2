/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#include "algorithms/cfr.h"
#include "algorithms/strategy.h"
#include "domains/matching_pennies.h"
#include "tests/domainsTest.h"
#include <boost/test/unit_test.hpp>
#include <algorithms/utility.h>
#include "algorithms/tree.h"
#include "domains/goofSpiel.h"
#include "algorithms/bestResponse.h"


namespace GTLib2 {
namespace algorithms {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using algorithms::CFRData;
using algorithms::bestResponseTo;


BOOST_AUTO_TEST_SUITE(CFRTest)

BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInSmallDomain) {
    MatchingPenniesDomain domain;
    CFRAlgorithm cfr(domain, Player(0), CFRSettings());
    auto& data = cfr.getCache();
    data.buildForest();
    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = childNode->getAOHInfSet();

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(0));
    auto&[regRoot, accRoot, regUpdRoot] = data.infosetData.at(rootInfoset);
    auto&[regChild, accChild, regUpdChild] = data.infosetData.at(childInfoset);
    BOOST_CHECK(cfvInfoset == 0.0);
    BOOST_CHECK(regRoot[0] == 0.0);
    BOOST_CHECK(regRoot[1] == 0.0);
    BOOST_CHECK(accRoot[0] == 0.5);
    BOOST_CHECK(accRoot[1] == 0.5);
    // does not change regrets / acc for player 1
    BOOST_CHECK(regChild[0] == 0.0);
    BOOST_CHECK(regChild[1] == 0.0);
    BOOST_CHECK(accChild[0] == 0.0);
    BOOST_CHECK(accChild[1] == 0.0);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(1));
    auto &[regRoot2, accRoot2, regUpdRoot2] = data.infosetData.at(rootInfoset);
    auto &[regChild2, accChild2, regUpdChild2] = data.infosetData.at(childInfoset);
    // does not change regrets / acc for player 0
    BOOST_CHECK(regRoot2[0] == 0.0);
    BOOST_CHECK(regRoot2[1] == 0.0);
    BOOST_CHECK(accRoot2[0] == 0.5);
    BOOST_CHECK(accRoot2[1] == 0.5);
    BOOST_CHECK(regChild2[0] == 0.5);
    BOOST_CHECK(regChild2[1] == 0.5);
    // acc is not two equal numbers because first
    // the left node is traversed and then the right node of the infoset
    BOOST_CHECK(accChild2[0] == 0.5);
    BOOST_CHECK(accChild2[1] == 1.5);
    // this is also why cfv is not zero
    BOOST_CHECK(cfvInfoset == -0.5);
}


BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInSmallDomainForInfosetUpdatingCFR) {
    MatchingPenniesDomain domain;
    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    CFRAlgorithm cfr(domain, Player(0), settings);
    auto& data = cfr.getCache();
    data.buildForest();
    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = childNode->getAOHInfSet();

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(0));
    cfr.delayedApplyRegretUpdates();
    auto&[regRoot, accRoot, regUpdRoot] = data.infosetData.at(rootInfoset);
    auto&[regChild, accChild, regUpdChild] = data.infosetData.at(childInfoset);
    BOOST_CHECK(cfvInfoset == 0.0);
    BOOST_CHECK(regRoot[0] == 0.0);
    BOOST_CHECK(regRoot[1] == 0.0);
    BOOST_CHECK(accRoot[0] == 0.5);
    BOOST_CHECK(accRoot[1] == 0.5);
    // does not change regrets / acc for player 1
    BOOST_CHECK(regChild[0] == 0.0);
    BOOST_CHECK(regChild[1] == 0.0);
    BOOST_CHECK(accChild[0] == 0.0);
    BOOST_CHECK(accChild[1] == 0.0);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(1));
    cfr.delayedApplyRegretUpdates();
    auto &[regRoot2, accRoot2, regUpdRoot2] = data.infosetData.at(rootInfoset);
    auto &[regChild2, accChild2, regUpdChild2] = data.infosetData.at(childInfoset);
    // does not change regrets / acc for player 0
    BOOST_CHECK(regRoot2[0] == 0.0);
    BOOST_CHECK(regRoot2[1] == 0.0);
    BOOST_CHECK(accRoot2[0] == 0.5);
    BOOST_CHECK(accRoot2[1] == 0.5);
    BOOST_CHECK(regChild2[0] == 0.0);
    BOOST_CHECK(regChild2[1] == 0.0);
    BOOST_CHECK(accChild2[0] == 1.0);
    BOOST_CHECK(accChild2[1] == 1.0);
    BOOST_CHECK(cfvInfoset == 0.0);

    cfr.runIterations(100);
    auto &[regRoot100, accRoot100, regUpdRoot100] = data.infosetData.at(rootInfoset);
    auto &[regChild100, accChild100, regUpdChild100] = data.infosetData.at(childInfoset);
    BOOST_CHECK(regRoot100[0] == 0.0);
    BOOST_CHECK(regRoot100[1] == 0.0);
    BOOST_CHECK(accRoot100[0] == 50.5);
    BOOST_CHECK(accRoot100[1] == 50.5);
    BOOST_CHECK(regChild100[0] == 0.0);
    BOOST_CHECK(regChild100[1] == 0.0);
    // Accumulators are not the same in root/child, because player1 histories
    // have been updated more times. This is ok, since we will calculate weighted
    // sum of these to get the average strategy. These increments are always added
    // by a constant number of times more (by each history in infoset) and they
    // will cancel out.
    BOOST_CHECK(accChild100[0] == 101.0);
    BOOST_CHECK(accChild100[1] == 101.0);
}


BOOST_AUTO_TEST_CASE(CheckConvergenceInSmallDomain) {
    domains::IIGoofSpielDomain domain(3, 3, nullopt);
    CFRAlgorithm cfr(domain, Player(0), CFRSettings());
    auto& data = cfr.getCache();

    double expectedUtilities[] =
        {0.00467926, 0.00251501, 0.00171567, 0.00130139, 0.00104813, 0.000877345, 0.000754399,
         0.000661667, 0.000589232, 0.00053109};
    double expectedBestResp0[] =
        {0.0353826, 0.0176913, 0.0117942, 0.00884565, 0.00707652, 0.0058971, 0.00505466, 0.00442283,
         0.0039314, 0.00353826};
    double expectedBestResp1[] =
        {0.0334574, 0.016742, 0.0111643, 0.00837431, 0.00669999, 0.00558362, 0.00478614, 0.00418799,
         0.00372274, 0.00335053};


    for (int i = 0; i < 10; ++i) {
        cfr.runIterations(50);
        auto profile = algorithms::getAverageStrategy(&data);
        auto bestResp0 = algorithms::bestResponseTo(
            profile[0], Player(0), Player(1), domain).second;
        auto bestResp1 = algorithms::bestResponseTo(
            profile[1], Player(1), Player(0), domain).second;
        double utility = algorithms::computeUtilityTwoPlayersGame(
            domain, profile[0], profile[1], Player(0), Player(1)).first;

        BOOST_CHECK(std::abs(utility - expectedUtilities[i]) <= 0.0001);
        BOOST_CHECK(std::abs(bestResp0 - expectedBestResp0[i]) <= 0.0001);
        BOOST_CHECK(std::abs(bestResp1 - expectedBestResp1[i]) <= 0.0001);
    }
}


BOOST_AUTO_TEST_SUITE_END()

}
}  // namespace GTLib2
