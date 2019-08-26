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

#include <domains/goofSpiel.h>
#include <algorithms/MCTS/ISMCTS.h>
#include <algorithms/MCTS/CPW_ISMCTS.h>
#include <algorithms/MCTS/selectors/RMSelectorFactory.h>
#include <algorithms/MCTS/selectors/Exp3SelectorFactory.h>
#include "gtest/gtest.h"

namespace GTLib2::algorithms {

TEST(ISMCTS, UCTTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  5, .fixChanceCards =  true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact, .useBelief=true, .randomSeed = seed};
        ISMCTSSettings settings1 = {.fact_ = fact, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 17; // useBelief is better
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, RMTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  5, .fixChanceCards =  true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact0 = make_shared<RMSelectorFactory>(0.2, -14, 14, seed);
        auto fact1 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact0, .randomSeed = seed};
        ISMCTSSettings settings1 = {.fact_ = fact1, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = -190; //very bad
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, Exp3StoreObsTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  5, .fixChanceCards = true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact0 = make_shared<Exp3SelectorFactory>(-14, 14, 0.05, true, seed);
        auto fact1 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact0, .randomSeed = seed};
        ISMCTSSettings settings1 = {.fact_ = fact1, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = -199; // very bad
    double summ = 0;
    for (auto r : rewards) summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, Exp3Test) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =  GTLib2::domains::IncompleteObservations, .numCards = 5, .fixChanceCards = true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact0 = make_shared<Exp3SelectorFactory>(-14, 14, 0.05, false, seed);
        auto fact1 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact0, .randomSeed = seed};
        ISMCTSSettings settings1 = {.fact_ = fact1, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = -199; // same => storeExploration makes no diff
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, Exp3LTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  5, .fixChanceCards =  true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact0 = make_shared<Exp3SelectorFactory>(true, -14, 14, 0.05, seed);
        auto fact1 = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact0, .randomSeed = seed};
        ISMCTSSettings settings1 = {.fact_ = fact1, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = -178; // very bad
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, MidgameTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  7, .fixChanceCards =  true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact, .randomSeed = seed};
        ISMCTSSettings settings1 = {.fact_ = fact, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 175; // CPW is better
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, BiggameTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 100; ++seed) {
        GTLib2::domains::GoofSpielSettings settings
            ({.variant =   GTLib2::domains::IncompleteObservations, .numCards =  10, .fixChanceCards =  true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        auto fact = make_shared<UCTSelectorFactory>(sqrt(2), seed);
        ISMCTSSettings settings0 = {.fact_ = fact, .randomSeed = seed};
        ISMCTSSettings settings1 = {.fact_ = fact, .randomSeed = seed};
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 508; // CPW is better
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

}

