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

    You should have received a copy of the GNU Lesser General Public 
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/



#include "base/base.h"
#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/oos.h"
#include "algorithms/tree.h"
#include "algorithms/utility.h"
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

TEST(OOS, CheckRegretsAndAccInSmallDomain) {
    MatchingPenniesDomain domain(AlternatingMoves);

    auto settings = OOSSettings();
    settings.samplingBlock = OOSSettings::OutcomeSampling;
    settings.accumulatorWeighting = OOSSettings::UniformAccWeighting;
    settings.regretMatching = OOSSettings::RegretMatchingNormal;
    settings.targeting = OOSSettings::InfosetTargeting;
    settings.playStrategy = OOSSettings::PlayUsingAvgStrategy;
    settings.samplingScheme = OOSSettings::EpsilonOnPolicySampling;
    settings.avgStrategyComputation = OOSSettings::StochasticallyWeightedAveraging;
    settings.baseline = OOSSettings::NoBaseline;
    settings.exploration = 0.6;
    settings.targetBiasing = 0.0;
    settings.gadgetExploration = 0.0;
    settings.gadgetInfosetBiasing = 0.0;
    settings.approxRegretMatching = 0.001;
    settings.batchSize = 1;
    settings.seed = 0;

    auto data = OOSData(domain);
    data.buildForest();
    auto rootNode = data.getRootNode();
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0]);
    auto childInfoset = childNode->getAOHInfSet();
    auto &rootData = data.infosetData.at(rootInfoset);
    auto &childData = data.infosetData.at(childInfoset);

    OOSAlgorithm oos(domain, Player(0), data, settings);

    EXPECT_TRUE(!rootData.fixRMStrategy);
    EXPECT_TRUE(!childData.fixRMStrategy);
    EXPECT_TRUE(!rootData.fixAvgStrategy);
    EXPECT_TRUE(!childData.fixAvgStrategy);
    EXPECT_TRUE(rootData.regretUpdates.empty());
    EXPECT_TRUE(childData.regretUpdates.empty());
    EXPECT_EQ(rootData.regrets[0], 0.0);
    EXPECT_EQ(rootData.regrets[1], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[0], 0.0);
    EXPECT_EQ(rootData.avgStratAccumulator[1], 0.0);
    EXPECT_EQ(childData.regrets[0], 0.0);
    EXPECT_EQ(childData.regrets[1], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[0], 0.0);
    EXPECT_EQ(childData.avgStratAccumulator[1], 0.0);

    // ------ one sample per player from the root ------
    oos.runPlayIteration(nullopt);

}

}
