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
#include <algorithms/MCTS/selectors/UCTSelectorFactory.h>
#include <algorithms/MCTS/selectors/RMSelectorFactory.h>
#include <algorithms/MCTS/selectors/Exp3SelectorFactory.h>
#include "gtest/gtest.h"

namespace GTLib2::algorithms {

TEST(ISMCTS, UCTTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 1; ++seed) {
        domains::GoofSpielSettings settings({/*.variant =*/domains::IncompleteObservations,
                                                /*.numCards =*/5,
                                                /*.fixChanceCards =*/true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        UCT_ISMCTSSettings settings0;
        settings0.useBelief = true;
        settings0.seed = seed;
        settings0.c = sqrt(2);
        UCT_ISMCTSSettings settings1;
        settings1.seed = seed;
        settings1.c = sqrt(2);
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = -1; // useBelief is better
    double summ = 0;
    cout << rewards << endl;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
    // todo: This might fail for release (sum=44), we're not sure why.
    //       Maybe it has something to do with randomization compiled differently.
}

TEST(ISMCTS, RMTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 1; ++seed) {
        domains::GoofSpielSettings settings({/*.variant =*/domains::IncompleteObservations,
                                                /*.numCards =*/5,
                                                /*.fixChanceCards =*/ true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);

        RM_ISMCTSSettings settings0(-14, 14);
        settings0.gamma = 0.2;
        settings0.seed = seed;
        UCT_ISMCTSSettings settings1;
        settings1.seed = seed;
        settings1.c = sqrt(2);
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 3;
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, Exp3StoreObsTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 1; ++seed) {
        domains::GoofSpielSettings settings({/*.variant =*/domains::IncompleteObservations,
                                                /*.numCards =*/5,
                                                /*.fixChanceCards =*/true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        EXP3_ISMCTSSettings settings0(-14, 14);
        settings0.seed = seed;
        settings0.gamma = 0.05;
        settings0.storeExploration = true;
        UCT_ISMCTSSettings settings1;
        settings1.seed = seed;
        settings1.c = sqrt(2);
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 3;
    double summ = 0;
    for (auto r : rewards) summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, Exp3Test) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 1; ++seed) {
        domains::GoofSpielSettings settings({/*.variant =*/ domains::IncompleteObservations,
                                                /*.numCards =*/5,
                                                /*.fixChanceCards =*/true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        EXP3_ISMCTSSettings settings0(-14, 14);
        settings0.seed = seed;
        settings0.gamma = 0.05;
        settings0.storeExploration = false;
        UCT_ISMCTSSettings settings1;
        settings1.seed = seed;
        settings1.c = sqrt(2);
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 3;
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, Exp3LTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 1; ++seed) {
        domains::GoofSpielSettings settings({/*.variant =*/  domains::IncompleteObservations,
                                                /*.numCards =*/5,
                                                /*.fixChanceCards =*/ true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        EXP3_ISMCTSSettings settings0(-14, 14);
        settings0.type = EXP3_ISMCTSSettings::Exp3L;
        settings0.seed = seed;
        settings0.gamma = 0.05;
        UCT_ISMCTSSettings settings1;
        settings1.seed = seed;
        settings1.c = sqrt(2);
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<CPW_ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 6;
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, MidgameTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 1; ++seed) {
        domains::GoofSpielSettings settings({/*.variant =*/  domains::IncompleteObservations,
                                                /*.numCards =*/7,
                                                /*.fixChanceCards =*/ true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        UCT_ISMCTSSettings settings0;
        settings0.seed = seed;
        settings0.c = sqrt(2);
        UCT_ISMCTSSettings settings1;
        settings1.seed = seed;
        settings1.c = sqrt(2);
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = 1;
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

TEST(ISMCTS, BiggameTest) {
    auto rewards = vector<double>(100);

    for (int seed = 0; seed < 1; ++seed) {
        domains::GoofSpielSettings settings({/*.variant =*/  domains::IncompleteObservations,
                                                /*.numCards =*/10,
                                                /*.fixChanceCards =*/ true});
        settings.shuffleChanceCards(seed);
        domains::GoofSpielDomain domain(settings);
        UCT_ISMCTSSettings settings0;
        settings0.seed = seed;
        settings0.c = sqrt(2);
        UCT_ISMCTSSettings settings1;
        settings1.seed = seed;
        settings1.c = sqrt(2);
        vector<PreparedAlgorithm> algs = {
            createInitializer<CPW_ISMCTS>(settings0),
            createInitializer<ISMCTS>(settings1)
        };
        auto actualOutcome = playMatch(domain, algs, {500, 500}, {50, 50}, BudgetIterations, seed);
        rewards[seed] = actualOutcome[0];
    }
    const double expsumm = -6;
    double summ = 0;
    for (auto r : rewards)summ += r;
    EXPECT_EQ(summ, expsumm);
}

}

