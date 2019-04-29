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

#include "base/base.h"
#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/strategy.h"
#include "algorithms/tree.h"
#include "algorithms/utility.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"
#include "tests/domainsTest.h"
#include <boost/test/unit_test.hpp>


namespace GTLib2::algorithms {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::MatchingPenniesVariant::SimultaneousMoves;
using domains::MatchingPenniesVariant::AlternatingMoves;
using domains::GoofSpielDomain;
using domains::GoofSpielVariant::IncompleteObservations;
using domains::GoofSpielVariant::CompleteObservations;

BOOST_AUTO_TEST_SUITE(AlgorithmsTests)
BOOST_AUTO_TEST_SUITE(CFR)

BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInSmallDomain) {
    MatchingPenniesDomain domain(AlternatingMoves);
    auto settings = CFRSettings();
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, data, Player(0), settings);
    data.buildForest();
    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    BOOST_CHECK(!rootData.fixRMStrategy);
    BOOST_CHECK(!childData.fixRMStrategy);
    BOOST_CHECK(!rootData.fixAvgStrategy);
    BOOST_CHECK(!childData.fixAvgStrategy);
    BOOST_CHECK(rootData.regretUpdates.empty());
    BOOST_CHECK(childData.regretUpdates.empty());

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(0));
    BOOST_CHECK(cfvInfoset == 0.0);
    BOOST_CHECK(rootData.regrets[0] == 0.0);
    BOOST_CHECK(rootData.regrets[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 0.5);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 0.5);
    // does not change regrets / acc for player 1
    BOOST_CHECK(childData.regrets[0] == 0.0);
    BOOST_CHECK(childData.regrets[1] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[0] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 0.0);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(1));
    // does not change regrets / acc for player 0 but it does for player 1!!
    BOOST_CHECK(rootData.regrets[0] == 0.0);
    BOOST_CHECK(rootData.regrets[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 0.5);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 0.5);
    BOOST_CHECK(childData.regrets[0] == 0.5);
    BOOST_CHECK(childData.regrets[1] == 0.5);
    // acc is not two equal numbers because first
    // the left node is traversed and then the right node of the infoset
    BOOST_CHECK(childData.avgStratAccumulator[0] == 0.5);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 1.5);
    // this is also why cfv is not zero
    BOOST_CHECK(cfvInfoset == -0.5);
}


BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInSmallDomainForInfosetUpdatingCFR) {
    MatchingPenniesDomain domain(AlternatingMoves);

    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, data, Player(0), settings);

    data.buildForest();
    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    BOOST_CHECK(!rootData.fixRMStrategy);
    BOOST_CHECK(!childData.fixRMStrategy);
    BOOST_CHECK(!rootData.fixAvgStrategy);
    BOOST_CHECK(!childData.fixAvgStrategy);
    BOOST_CHECK(rootData.regretUpdates.size() == 2);
    BOOST_CHECK(childData.regretUpdates.size() == 2);

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(0));
    cfr.delayedApplyRegretUpdates();
    BOOST_CHECK(cfvInfoset == 0.0);
    BOOST_CHECK(rootData.regrets[0] == 0.0);
    BOOST_CHECK(rootData.regrets[1] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[0] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 0.5);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 0.5);
    // does not change regrets / acc for player 1
    BOOST_CHECK(childData.regrets[0] == 0.0);
    BOOST_CHECK(childData.regrets[1] == 0.0);
    BOOST_CHECK(childData.regretUpdates[0] == 0.0);
    BOOST_CHECK(childData.regretUpdates[1] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[0] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 0.0);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(1));
    cfr.delayedApplyRegretUpdates();
    // does not change regrets / acc for player 0
    BOOST_CHECK(rootData.regrets[0] == 0.0);
    BOOST_CHECK(rootData.regrets[1] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[0] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 0.5);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 0.5);
    // for player 1 it does not change regrets but changes acc
    BOOST_CHECK(childData.regrets[0] == 0.0);
    BOOST_CHECK(childData.regrets[1] == 0.0);
    BOOST_CHECK(childData.regretUpdates[0] == 0.0);
    BOOST_CHECK(childData.regretUpdates[1] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[0] == 1.0);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 1.0);
    BOOST_CHECK(cfvInfoset == 0.0);

    // ------ many iterations ------
    cfr.runIterations(100);
    BOOST_CHECK(rootData.regrets[0] == 0.0);
    BOOST_CHECK(rootData.regrets[1] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[0] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 50.5);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 50.5);
    // for player 1 it does not change regrets but changes acc
    BOOST_CHECK(childData.regrets[0] == 0.0);
    BOOST_CHECK(childData.regrets[1] == 0.0);
    BOOST_CHECK(childData.regretUpdates[0] == 0.0);
    BOOST_CHECK(childData.regretUpdates[1] == 0.0);
    // Accumulators are not the same in root/child, because player1 histories
    // have been updated more times. This is ok, since we will calculate weighted
    // sum of these to get the average strategy. These increments are always added
    // by a constant number of times more (by each history in infoset) and they
    // will cancel out.
    BOOST_CHECK(childData.avgStratAccumulator[0] == 101.0);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 101.0);
}


BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInSmallDomainFixStrategy) {
    MatchingPenniesDomain domain(AlternatingMoves);
    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, data, Player(0), settings);
    data.buildForest();
    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    rootData.regrets[0] = 0.75;
    rootData.regrets[1] = 0.25;
    childData.avgStratAccumulator[0] = 0.125;
    childData.avgStratAccumulator[1] = 0.875;
    rootData.fixRMStrategy = true;
    childData.fixAvgStrategy = true;
    BOOST_CHECK(rootData.fixRMStrategy);
    BOOST_CHECK(!childData.fixRMStrategy);
    BOOST_CHECK(!rootData.fixAvgStrategy);
    BOOST_CHECK(childData.fixAvgStrategy);
    BOOST_CHECK(rootData.regretUpdates.size() == 2);
    BOOST_CHECK(childData.regretUpdates.size() == 2);

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(0));
    cfr.delayedApplyRegretUpdates();
    BOOST_CHECK(cfvInfoset == 0.0);
    BOOST_CHECK(rootData.regrets[0] == 0.75);
    BOOST_CHECK(rootData.regrets[1] == 0.25);
    BOOST_CHECK(rootData.regretUpdates[0] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 0.75);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 0.25);
    BOOST_CHECK(childData.regrets[0] == 0.0);
    BOOST_CHECK(childData.regrets[1] == 0.0);
    BOOST_CHECK(childData.regretUpdates[0] == 0.0);
    BOOST_CHECK(childData.regretUpdates[1] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[0] == 0.125);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 0.875);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, std::array<double, 3>{1., 1., 1.}, Player(1));
    cfr.delayedApplyRegretUpdates();
    BOOST_CHECK(rootData.regrets[0] == 0.75);
    BOOST_CHECK(rootData.regrets[1] == 0.25);
    BOOST_CHECK(rootData.regretUpdates[0] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 0.75);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 0.25);
    BOOST_CHECK(childData.regrets[0] == -0.5);
    BOOST_CHECK(childData.regrets[1] == 0.5);
    BOOST_CHECK(childData.regretUpdates[0] == 0.0);
    BOOST_CHECK(childData.regretUpdates[1] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[0] == 0.125);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 0.875);
    BOOST_CHECK(cfvInfoset == 0.0);

    // ------ many iterations ------
    cfr.runIterations(100);
    BOOST_CHECK(rootData.regrets[0] == 0.75);
    BOOST_CHECK(rootData.regrets[1] == 0.25);
    BOOST_CHECK(rootData.regretUpdates[0] == 0.0);
    BOOST_CHECK(rootData.regretUpdates[1] == 0.0);
    BOOST_CHECK(rootData.avgStratAccumulator[0] == 75.75);
    BOOST_CHECK(rootData.avgStratAccumulator[1] == 25.25);
    BOOST_CHECK(childData.regrets[0] == -100.5);
    BOOST_CHECK(childData.regrets[1] == 0.5);
    BOOST_CHECK(childData.regretUpdates[0] == 0.0);
    BOOST_CHECK(childData.regretUpdates[1] == 0.0);
    BOOST_CHECK(childData.avgStratAccumulator[0] == 0.125);
    BOOST_CHECK(childData.avgStratAccumulator[1] == 0.875);
}


BOOST_AUTO_TEST_CASE(CalcUtilities) {
    MatchingPenniesDomain domain(AlternatingMoves);
    CFRData data(domain, InfosetsUpdating);
    data.buildForest();

    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    rootData.regrets[0] = 0.75;
    rootData.regrets[1] = 0.25;
    rootData.avgStratAccumulator[0] = 0.125;
    rootData.avgStratAccumulator[1] = 0.875;
    childData.regrets[0] = 0.2;
    childData.regrets[1] = 0.8;
    childData.avgStratAccumulator[0] = 0.1;
    childData.avgStratAccumulator[1] = 0.9;

    auto actualRootUtils = calcExpectedUtility(data, rootNode, Player(0));
    auto actualChildUtils = calcExpectedUtility(data, childNode, Player(0));

    BOOST_CHECK(fabs(actualRootUtils.rmUtility - -0.3) < 1e-9);
    BOOST_CHECK(fabs(actualRootUtils.avgUtility - 0.6) < 1e-9);
    BOOST_CHECK(fabs(actualChildUtils.rmUtility - -0.6) < 1e-9);
    BOOST_CHECK(fabs(actualChildUtils.avgUtility - -0.8) < 1e-9);
}


BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInGS2) {
    GoofSpielDomain domain({variant:  IncompleteObservations,
                               numCards: 2,
                               fixChanceCards: true,
                               chanceCards: {2, 1},
                               binaryTerminalRewards: false});
    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    settings.accumulatorWeighting = UniformAccWeighting;
    settings.regretMatching = RegretMatchingNormal;
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, data, Player(0), settings);
    cfr.runIterations(1000);

    auto profile = algorithms::getAverageStrategy(cfr.getCache());
    auto bestResp0 = algorithms::bestResponseTo(profile[0], Player(0), Player(1), domain).second;
    auto bestResp1 = algorithms::bestResponseTo(profile[1], Player(1), Player(0), domain).second;
    double utility = algorithms::computeUtilityTwoPlayersGame(
        domain, profile[0], profile[1], Player(0), Player(1)).first;

    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = data.getInfosetFor(rootNode);
    auto stratPlayer = profile[0].at(rootInfoset);
    auto rootAction = rootNode->availableActions()[0];

    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = data.getInfosetFor(childNode);
    auto stratOpponent = profile[1].at(childInfoset);
    auto childAction = childNode->availableActions()[0];

    BOOST_CHECK(std::abs(utility - 0) <= 0.0001);
    BOOST_CHECK(std::abs(bestResp0 - 0) <= 0.001);
    BOOST_CHECK(std::abs(bestResp1 - 0) <= 0.001);
    BOOST_CHECK(stratPlayer.at(rootAction) == stratOpponent.at(childAction));
}

BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInGS3) {
    GoofSpielDomain domain({variant:  IncompleteObservations,
                               numCards: 3,
                               fixChanceCards: true,
                               chanceCards: {3, 2, 1}});
    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, data, Player(0), settings);
    cfr.runIterations(1000);

    auto profile = algorithms::getAverageStrategy(data);
    auto bestResp0 = algorithms::bestResponseTo(profile[0], Player(0), Player(1), domain).second;
    auto bestResp1 = algorithms::bestResponseTo(profile[1], Player(1), Player(0), domain).second;
    double utility = algorithms::computeUtilityTwoPlayersGame(
        domain, profile[0], profile[1], Player(0), Player(1)).first;

    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = data.getInfosetFor(rootNode);
    auto stratPlayer = profile[0].at(rootInfoset);
    auto rootAction = rootNode->availableActions()[0];

    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = data.getInfosetFor(childNode);
    auto stratOpponent = profile[1].at(childInfoset);
    auto childAction = childNode->availableActions()[0];

    BOOST_CHECK(std::abs(utility - 0) <= 0.0002);
    BOOST_CHECK(std::abs(bestResp0 - 0) <= 0.0015);
    BOOST_CHECK(std::abs(bestResp1 - 0) <= 0.0015);
}

BOOST_AUTO_TEST_CASE(CheckConvergenceInSmallDomain) {
    GoofSpielDomain domain
        ({variant: IncompleteObservations, numCards: 3, fixChanceCards: false, chanceCards: {}});
    auto settings = CFRSettings();
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, data, Player(0), settings);

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
        auto profile = algorithms::getAverageStrategy(data);
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
BOOST_AUTO_TEST_SUITE_END()

}  // namespace GTLib2
