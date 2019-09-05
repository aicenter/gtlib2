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

#include "algorithms/utility.h"
#include "algorithms/evaluation.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"

#include "tests/domainsTest.h"

#include "gtest/gtest.h"


namespace GTLib2::algorithms {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using domains::MatchingPenniesVariant::SimultaneousMoves;
using domains::MatchingPenniesVariant::AlternatingMoves;
using domains::GoofSpielDomain;
using domains::GoofSpielVariant::IncompleteObservations;
using domains::GoofSpielVariant::CompleteObservations;

TEST(CFR, CheckRegretsAndAccInSmallDomain) {
    MatchingPenniesDomain domain(AlternatingMoves);
    auto settings = CFRSettings();
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, Player(0), data, settings);
    data.buildTree();
    auto rootNode = data.getRootNode();
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    EXPECT_TRUE(!rootData.fixRMStrategy);
    EXPECT_TRUE(!childData.fixRMStrategy);
    EXPECT_TRUE(!rootData.fixAvgStrategy);
    EXPECT_TRUE(!childData.fixAvgStrategy);
    EXPECT_TRUE(rootData.regretUpdates.empty());
    EXPECT_TRUE(childData.regretUpdates.empty());

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, array<double, 3>{1., 1., 1.}, Player(0));
    EXPECT_EQ(cfvInfoset, 0.0);
    EXPECT_EQ(rootData.regrets[0], 0.0);
    EXPECT_EQ(rootData.regrets[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 0.5);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 0.5);
    // does not change regrets / acc for player 1
    EXPECT_EQ(childData.regrets[0], 0.0);
    EXPECT_EQ(childData.regrets[1], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[0], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[1], 0.0);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, array<double, 3>{1., 1., 1.}, Player(1));
    // does not change regrets / acc for player 0 but it does for player 1!!
    EXPECT_EQ(rootData.regrets[0], 0.0);
    EXPECT_EQ(rootData.regrets[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 0.5);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 0.5);
    EXPECT_EQ(childData.regrets[0], 0.5);
    EXPECT_EQ(childData.regrets[1], 0.5);
    // acc is not two equal numbers because first
    // the left node is traversed and then the right node of the infoset
    EXPECT_EQ(childData.avgStratAccumulator[0], 0.5);
    EXPECT_EQ(childData.avgStratAccumulator[1], 1.5);
    // this is also why cfv is not zero
    EXPECT_EQ(cfvInfoset, -0.5);
}

TEST(CFR, CheckRegretsAndAccInSmallDomainForInfosetUpdatingCFR) {
    MatchingPenniesDomain domain(AlternatingMoves);

    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, Player(0), data, settings);

    data.buildTree();
    auto rootNode = data.getRootNode();
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    auto profile = getAverageStrategy(data);
    auto bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
    auto bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
    EXPECT_TRUE(!rootData.fixRMStrategy);
    EXPECT_TRUE(!childData.fixRMStrategy);
    EXPECT_TRUE(!rootData.fixAvgStrategy);
    EXPECT_TRUE(!childData.fixAvgStrategy);
    EXPECT_EQ(rootData.regretUpdates.size(), 2);
    EXPECT_EQ(childData.regretUpdates.size(), 2);
    EXPECT_EQ(bestResp0, 0.0);
    EXPECT_EQ(bestResp1, 0.0);

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, array<double, 3>{1., 1., 1.}, Player(0));
    cfr.delayedApplyRegretUpdates();
    profile = getAverageStrategy(data);
    bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
    bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
    EXPECT_EQ(cfvInfoset, 0.0);
    EXPECT_EQ(rootData.regrets[0], 0.0);
    EXPECT_EQ(rootData.regrets[1], 0.0);
    EXPECT_EQ(rootData.regretUpdates[0], 0.0);
    EXPECT_EQ(rootData.regretUpdates[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 0.5);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 0.5);
    // does not change regrets / acc for player 1
    EXPECT_EQ(childData.regrets[0], 0.0);
    EXPECT_EQ(childData.regrets[1], 0.0);
    EXPECT_EQ(childData.regretUpdates[0], 0.0);
    EXPECT_EQ(childData.regretUpdates[1], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[0], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[1], 0.0);
    EXPECT_EQ(bestResp0, 0.0);
    EXPECT_EQ(bestResp1, 0.0);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, array<double, 3>{1., 1., 1.}, Player(1));
    cfr.delayedApplyRegretUpdates();
    profile = getAverageStrategy(data);
    bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
    bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
    // does not change regrets / acc for player 0
    EXPECT_EQ(rootData.regrets[0], 0.0);
    EXPECT_EQ(rootData.regrets[1], 0.0);
    EXPECT_EQ(rootData.regretUpdates[0], 0.0);
    EXPECT_EQ(rootData.regretUpdates[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 0.5);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 0.5);
    // for player 1 it does not change regrets but changes acc
    EXPECT_EQ(childData.regrets[0], 0.0);
    EXPECT_EQ(childData.regrets[1], 0.0);
    EXPECT_EQ(childData.regretUpdates[0], 0.0);
    EXPECT_EQ(childData.regretUpdates[1], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[0], 1.0);
    EXPECT_EQ(childData.avgStratAccumulator[1], 1.0);
    EXPECT_EQ(cfvInfoset, 0.0);
    EXPECT_EQ(bestResp0, 0.0);
    EXPECT_EQ(bestResp1, 0.0);

    // ------ many iterations ------
    cfr.runIterations(100);
    profile = getAverageStrategy(data);
    bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
    bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
    EXPECT_EQ(rootData.regrets[0], 0.0);
    EXPECT_EQ(rootData.regrets[1], 0.0);
    EXPECT_EQ(rootData.regretUpdates[0], 0.0);
    EXPECT_EQ(rootData.regretUpdates[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 50.5);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 50.5);
    // for player 1 it does not change regrets but changes acc
    EXPECT_EQ(childData.regrets[0], 0.0);
    EXPECT_EQ(childData.regrets[1], 0.0);
    EXPECT_EQ(childData.regretUpdates[0], 0.0);
    EXPECT_EQ(childData.regretUpdates[1], 0.0);
    // Accumulators are not the same in root/child, because player1 histories
    // have been updated more times. This is ok, since we will calculate weighted
    // sum of these to get the average strategy. These increments are always added
    // by a constant number of times more (by each history in infoset) and they
    // will cancel out.
    EXPECT_EQ(childData.avgStratAccumulator[0], 101.0);
    EXPECT_EQ(childData.avgStratAccumulator[1], 101.0);
    EXPECT_EQ(bestResp0, 0.0);
    EXPECT_EQ(bestResp1, 0.0);
}

TEST(CFR, CheckRegretsAndAccInSmallDomainFixStrategy) {
    MatchingPenniesDomain domain(AlternatingMoves);
    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, Player(0), data, settings);
    data.buildTree();
    auto rootNode = data.getRootNode();
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);
    rootData.regrets[0] = 0.75;
    rootData.regrets[1] = 0.25;
    childData.avgStratAccumulator[0] = 0.125;
    childData.avgStratAccumulator[1] = 0.875;
    rootData.fixRMStrategy = true;
    childData.fixAvgStrategy = true;
    EXPECT_TRUE(rootData.fixRMStrategy);
    EXPECT_TRUE(!childData.fixRMStrategy);
    EXPECT_TRUE(!rootData.fixAvgStrategy);
    EXPECT_TRUE(childData.fixAvgStrategy);
    EXPECT_EQ(rootData.regretUpdates.size(), 2);
    EXPECT_EQ(childData.regretUpdates.size(), 2);

    // ------ iteration player 0 ------
    double cfvInfoset = cfr.runIteration(rootNode, array<double, 3>{1., 1., 1.}, Player(0));
    cfr.delayedApplyRegretUpdates();
    EXPECT_EQ(cfvInfoset, 0.0);
    EXPECT_EQ(rootData.regrets[0], 0.75);
    EXPECT_EQ(rootData.regrets[1], 0.25);
    EXPECT_EQ(rootData.regretUpdates[0], 0.0);
    EXPECT_EQ(rootData.regretUpdates[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 0.75);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 0.25);
    EXPECT_EQ(childData.regrets[0], 0.0);
    EXPECT_EQ(childData.regrets[1], 0.0);
    EXPECT_EQ(childData.regretUpdates[0], 0.0);
    EXPECT_EQ(childData.regretUpdates[1], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[0], 0.125);
    EXPECT_EQ(childData.avgStratAccumulator[1], 0.875);

    // ------ iteration player 1 ------
    cfvInfoset = cfr.runIteration(rootNode, array<double, 3>{1., 1., 1.}, Player(1));
    cfr.delayedApplyRegretUpdates();
    EXPECT_EQ(rootData.regrets[0], 0.75);
    EXPECT_EQ(rootData.regrets[1], 0.25);
    EXPECT_EQ(rootData.regretUpdates[0], 0.0);
    EXPECT_EQ(rootData.regretUpdates[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 0.75);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 0.25);
    EXPECT_EQ(childData.regrets[0], -0.5);
    EXPECT_EQ(childData.regrets[1], 0.5);
    EXPECT_EQ(childData.regretUpdates[0], 0.0);
    EXPECT_EQ(childData.regretUpdates[1], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[0], 0.125);
    EXPECT_EQ(childData.avgStratAccumulator[1], 0.875);
    EXPECT_EQ(cfvInfoset, 0.0);

    // ------ many iterations ------
    cfr.runIterations(100);
    EXPECT_EQ(rootData.regrets[0], 0.75);
    EXPECT_EQ(rootData.regrets[1], 0.25);
    EXPECT_EQ(rootData.regretUpdates[0], 0.0);
    EXPECT_EQ(rootData.regretUpdates[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 75.75);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 25.25);
    EXPECT_EQ(childData.regrets[0], -100.5);
    EXPECT_EQ(childData.regrets[1], 0.5);
    EXPECT_EQ(childData.regretUpdates[0], 0.0);
    EXPECT_EQ(childData.regretUpdates[1], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[0], 0.125);
    EXPECT_EQ(childData.avgStratAccumulator[1], 0.875);
}

TEST(CFR, CalcUtilities) {
    MatchingPenniesDomain domain(AlternatingMoves);
    CFRData data(domain, InfosetsUpdating);
    data.buildTree();

    auto rootNode = data.getRootNode();
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
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

    EXPECT_LE(fabs(actualRootUtils.rmUtility - -0.3), 1e-9);
    EXPECT_LE(fabs(actualRootUtils.avgUtility - 0.6), 1e-9);
    EXPECT_LE(fabs(actualChildUtils.rmUtility - -0.6), 1e-9);
    EXPECT_LE(fabs(actualChildUtils.avgUtility - -0.8), 1e-9);
}

TEST(CFR, CheckRegretsAndAccInGS2) {
    auto domain = GoofSpielDomain::IIGS(2);
    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    settings.accumulatorWeighting = UniformAccWeighting;
    settings.regretMatching = RegretMatchingNormal;
    auto data = CFRData(*domain, settings.cfrUpdating);
    CFRAlgorithm cfr(*domain, Player(0), data, settings);
    cfr.runIterations(1000);

    auto profile = getAverageStrategy(cfr.getCache());
    auto bestResp0 = bestResponseTo(profile[0], Player(1), *domain).value;
    auto bestResp1 = bestResponseTo(profile[1], Player(0), *domain).value;
    double utility = computeUtilitiesTwoPlayerGame(*domain, profile)[0];

    auto rootNode = data.getRootNode();
    auto rootInfoset = data.getInfosetFor(rootNode);
    auto stratPlayer = profile[0].at(rootInfoset);
    auto rootAction = rootNode->availableActions()[0];

    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    auto childInfoset = data.getInfosetFor(childNode);
    auto stratOpponent = profile[1].at(childInfoset);
    auto childAction = childNode->availableActions()[0];

    EXPECT_LE(std::abs(utility - 0), 0.0001);
    EXPECT_LE(std::abs(bestResp0 - 0), 0.001);
    EXPECT_LE(std::abs(bestResp1 - 0), 0.001);
    EXPECT_EQ(stratPlayer.at(rootAction), stratOpponent.at(childAction));
}

TEST(CFR, CheckRegretsAndAccInGS3) {
    auto domain = GoofSpielDomain::IIGS(3);

    auto settings = CFRSettings();
    settings.cfrUpdating = InfosetsUpdating;
    auto data = CFRData(*domain, settings.cfrUpdating);
    CFRAlgorithm cfr(*domain, Player(0), data, settings);
    cfr.runIterations(1000);

    auto profile = getAverageStrategy(data);
    auto bestResp0 = bestResponseTo(profile[0], Player(1), *domain).value;
    auto bestResp1 = bestResponseTo(profile[1], Player(0), *domain).value;
    double utility = computeUtilitiesTwoPlayerGame(*domain, profile)[0];

    auto rootNode = data.getRootNode();
    auto rootInfoset = data.getInfosetFor(rootNode);
    auto stratPlayer = profile[0].at(rootInfoset);
    auto rootAction = rootNode->availableActions()[0];

    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    auto childInfoset = data.getInfosetFor(childNode);
    auto stratOpponent = profile[1].at(childInfoset);
    auto childAction = childNode->availableActions()[0];

    EXPECT_LE(std::abs(utility - 0), 0.0002);
    EXPECT_LE(std::abs(bestResp0 - 0), 0.0015);
    EXPECT_LE(std::abs(bestResp1 - 0), 0.0015);
}

TEST(CFR, CheckConvergenceInSmallDomain) {
    GoofSpielDomain domain({
                               variant: IncompleteObservations,
                               numCards: 3,
                               fixChanceCards: false,
                               chanceCards: {}
                           });
    auto settings = CFRSettings();
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, Player(0), data, settings);

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
        auto profile = getAverageStrategy(data);
        auto bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
        auto bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
        double utility = computeUtilitiesTwoPlayerGame(domain, profile)[0];

        EXPECT_LE(std::fabs(utility - expectedUtilities[i]), 0.0001);
        EXPECT_LE(std::fabs(bestResp0 - expectedBestResp0[i]), 0.0001);
        EXPECT_LE(std::fabs(bestResp1 - expectedBestResp1[i]), 0.0001);
    }
}

TEST(CFR, CheckConvergenceSmallStepsInSmallDomain) {
    GoofSpielDomain domain({
                               variant: IncompleteObservations,
                               numCards: 3,
                               fixChanceCards: false,
                               chanceCards: {}
                           });
    auto settings = CFRSettings();
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, Player(0), data, settings);
    double expectedUtilities[] =
        {-0.113618827, -0.122875501, -0.0612063457, -0.0277733968, -0.015568159, -0.0034593418,
         0.00288986606, 0.0063839834, 0.0083464808, 0.00943900775};
    double expectedBestResp0[] =
        {1.27777778, 0.791132479, 0.601918687, 0.501760024, 0.368453093, 0.294855071, 0.252732918,
         0.221141303, 0.196570047, 0.176913042};
    double expectedBestResp1[] =
        {1.03009259, 0.719285615, 0.545334106, 0.404925503, 0.330338847, 0.275842824, 0.23680958,
         0.207464495, 0.184595117, 0.166269436};

    for (int i = 0; i < 10; ++i) {
        cfr.runIterations(1);
        auto profile = getAverageStrategy(data);
        auto bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
        auto bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
        double utility = computeUtilitiesTwoPlayerGame(domain, profile)[0];

        EXPECT_LE(std::fabs(utility - expectedUtilities[i]), 1e-8);
        EXPECT_LE(std::fabs(bestResp0 - expectedBestResp0[i]), 1e-8);
        EXPECT_LE(std::fabs(bestResp1 - expectedBestResp1[i]), 1e-8);
    }
}

TEST(CFR, CheckConvergenceSmallStepsInSmallDomain2) {
    GoofSpielDomain domain({
                               variant: IncompleteObservations,
                               numCards: 2,
                               fixChanceCards: true,
                               chanceCards: {}
                           });
    auto settings = CFRSettings();
    auto data = CFRData(domain, settings.cfrUpdating);
    CFRAlgorithm cfr(domain, Player(0), data, settings);
    double expectedUtilities[] =
        {-0.25, -0.125, -0.0833333333, -0.0625, -0.05, -0.0416666667, -0.0357142857, -0.03125, -0.0277777778, -0.025};
    double expectedBestResp0[] =
        {0.5, 0.25, 0.166666667, 0.125, 0.1, 0.0833333333, 0.0714285714, 0.0625, 0.0555555556, 0.05};
    double expectedBestResp1[] =
        {0.25, 0.125, 0.0833333333, 0.0625, 0.05, 0.0416666667, 0.0357142857, 0.03125, 0.0277777778, 0.025};

    for (int i = 0; i < 10; ++i) {
        cfr.runIterations(1);
        auto profile = getAverageStrategy(data);
        auto bestResp0 = bestResponseTo(profile[0], Player(1), domain).value;
        auto bestResp1 = bestResponseTo(profile[1], Player(0), domain).value;
        double utility = computeUtilitiesTwoPlayerGame(domain, profile)[0];

        EXPECT_LE(std::fabs(utility - expectedUtilities[i]), 1e-8);
        EXPECT_LE(std::fabs(bestResp0 - expectedBestResp0[i]), 1e-8);
        EXPECT_LE(std::fabs(bestResp1 - expectedBestResp1[i]), 1e-8);
    }
}

TEST(CFR, CheckExploitabilityInSmallDomain) {
    auto domain = GoofSpielDomain::IIGS(5);
    auto settings = CFRSettings();
    auto data = CFRData(*domain, settings.cfrUpdating);
    CFRAlgorithm cfr(*domain, Player(0), data, settings);

    cfr.runIterations(2);
    auto profile = getAverageStrategy(data);
    EXPECT_LE(std::fabs(0.159173 - calcExploitability(*domain, getAverageStrategy(data)).expl), 0.0001);
}

}  // namespace GTLib2
