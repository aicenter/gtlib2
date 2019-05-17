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
#include "tests/domainsTest.h"

#include "algorithms/tree.h"
#include "algorithms/common.h"

#include "domains/genericPoker.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"
#include "domains/oshiZumo.h"

#include "gtest/gtest.h"

namespace GTLib2::domains {

using algorithms::treeWalkEFG;
using GoofSpielVariant::IncompleteObservations;
using GoofSpielVariant::CompleteObservations;

bool isDomainZeroSum(const Domain &domain) {
    int num_violations = 0;
    auto countViolations = [&num_violations](shared_ptr<EFGNode> node) {
        if (node->type_ != TerminalNode) return;
        if (node->getUtilities()[0] != -node->getUtilities()[1]) num_violations++;
    };

    treeWalkEFG(domain, countViolations, domain.getMaxStateDepth());
    return num_violations == 0;
}

// todo: needs friend
//bool isEFGNodeAndStateConsistent(const Domain &domain) {
//    int num_violations = 0;
//    EFGCache cache(domain);
//    cache.buildForest(domain.getMaxStateDepth());
//    auto nodes = cache.getNodes();
//    for (const auto &n1: nodes) {
//        for (const auto &n2: nodes) {
//            if (n1 == n2 && (n1->getHash() != n2->getHash() || n1->outcomeDist_->state_ != n2->outcomeDist_->state_)) {
//                num_violations++;
//            }
//        }
//    }
//    return num_violations == 0;
//}

bool areAvailableActionsSorted(const Domain &domain) {
    int num_violations = 0;
    auto countViolations = [&num_violations](shared_ptr<EFGNode> node) {
        if(node->type_ == TerminalNode) return;
        auto actions = node->availableActions();
        for (int j = 0; j < actions.size(); ++j) {
            if (actions[j]->getId() != j) num_violations++;
        }
    };

    treeWalkEFG(domain, countViolations, domain.getMaxStateDepth());
    return num_violations == 0;
}

double domainFindMaxUtility(const Domain &domain) {
    double maxLeafUtility = 0;
    auto traverse = [&maxLeafUtility](shared_ptr<EFGNode> node) {
        if(node->type_ != TerminalNode) return;
        maxLeafUtility = max({node->getUtilities()[0], node->getUtilities()[1], maxLeafUtility});
    };

    treeWalkEFG(domain, traverse, domain.getMaxStateDepth());
    return maxLeafUtility;
}

double domainFindMaxDepth(const Domain &domain) {
    int maxDepth = 0;
    auto countViolations = [&maxDepth](shared_ptr<EFGNode> node) {
        maxDepth = max(node->stateDepth_, maxDepth);
    };

    treeWalkEFG(domain, countViolations, domain.getMaxStateDepth());
    return maxDepth;
}

// todo: needs friend
//bool isNumPlayersCountActionsConsistentInState(const Domain &domain) {
//    int num_violations = 0;
//    auto domainPlayers = vector<Player>(domain.getPlayers());
//    std::sort(domainPlayers.begin(), domainPlayers.end());
//
//    auto countViolations = [&](shared_ptr<EFGNode> node) {
//        auto statePlayers = vector<Player>(node->outcomeDist_->state_->getPlayers());
//        std::sort(statePlayers.begin(), statePlayers.end());
//
//        // players that should play must have non-zero number of actions
//        for (const auto &player: statePlayers) {
//            if (node->outcomeDist_->state_->countAvailableActionsFor(player) == 0) num_violations++;
//        }
//
//        // players that are not playing should have zero number of actions
//        auto playerDiff = vector<Player>();
//        std::set_difference(domainPlayers.begin(), domainPlayers.end(),
//                            statePlayers.begin(), statePlayers.end(),
//                            playerDiff.begin());
//
//        for (const auto &player: playerDiff) {
//            if (node->outcomeDist_->state_->countAvailableActionsFor(player) > 0) num_violations++;
//        }
//    };
//
//    treeWalkEFG(domain, countViolations, domain.getMaxStateDepth());
//    return num_violations == 0;
//}


// @formatter:off
GoofSpielDomain gs1      ({ variant:  CompleteObservations,   numCards: 1, fixChanceCards: false, chanceCards: {}});
GoofSpielDomain gs2      ({ variant:  CompleteObservations,   numCards: 2, fixChanceCards: false, chanceCards: {}});
GoofSpielDomain gs3      ({ variant:  CompleteObservations,   numCards: 3, fixChanceCards: false, chanceCards: {}});
GoofSpielDomain gs1_fix  ({ variant:  CompleteObservations,   numCards: 1, fixChanceCards: true,  chanceCards: {}});
GoofSpielDomain gs2_fix  ({ variant:  CompleteObservations,   numCards: 2, fixChanceCards: true,  chanceCards: {}});
GoofSpielDomain gs3_fix  ({ variant:  CompleteObservations,   numCards: 3, fixChanceCards: true,  chanceCards: {}});
GoofSpielDomain iigs1    ({ variant:  IncompleteObservations, numCards: 1, fixChanceCards: false, chanceCards: {}});
GoofSpielDomain iigs2    ({ variant:  IncompleteObservations, numCards: 2, fixChanceCards: false, chanceCards: {}});
GoofSpielDomain iigs3    ({ variant:  IncompleteObservations, numCards: 3, fixChanceCards: false, chanceCards: {}});
GoofSpielDomain iigs1_fix({ variant:  IncompleteObservations, numCards: 1, fixChanceCards: true,  chanceCards: {}});
GoofSpielDomain iigs2_fix({ variant:  IncompleteObservations, numCards: 2, fixChanceCards: true,  chanceCards: {}});
GoofSpielDomain iigs3_fix({ variant:  IncompleteObservations, numCards: 3, fixChanceCards: true,  chanceCards: {}});


OshiZumoDomain oz1  ({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 3,  .minBid = 1, .optimalEndGame = true});
OshiZumoDomain oz2  ({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 0,  .minBid = 1, .optimalEndGame = true});
OshiZumoDomain oz3  ({.variant =  CompleteObservation, .startingCoins = 1, .startingLocation = 3,  .minBid = 0, .optimalEndGame = true});
OshiZumoDomain oz4  ({.variant =  CompleteObservation, .startingCoins = 3, .startingLocation = 3,  .minBid = 1, .optimalEndGame = false});
OshiZumoDomain oz5  ({.variant =  CompleteObservation, .startingCoins = 5, .startingLocation = 3,  .minBid = 1, .optimalEndGame = false});
OshiZumoDomain iioz1({.variant =  IncompleteObservation, .startingCoins = 3, .startingLocation = 3,  .minBid = 1, .optimalEndGame = true});
OshiZumoDomain iioz2({.variant =  IncompleteObservation, .startingCoins = 3, .startingLocation = 0,  .minBid = 1, .optimalEndGame = true});
OshiZumoDomain iioz3({.variant =  IncompleteObservation, .startingCoins = 1, .startingLocation = 3,  .minBid = 0, .optimalEndGame = true});
OshiZumoDomain iioz4({.variant =  IncompleteObservation, .startingCoins = 3, .startingLocation = 3,  .minBid = 1, .optimalEndGame = false});
OshiZumoDomain iioz5({.variant =  IncompleteObservation, .startingCoins = 5, .startingLocation = 3,  .minBid = 1, .optimalEndGame = false});

// @formatter:on
GenericPokerDomain gp1(2, 2, 2, 2, 2);
GenericPokerDomain gp2(3, 3, 1, 2, 3);

MatchingPenniesDomain mp1(AlternatingMoves);
MatchingPenniesDomain mp2(SimultaneousMoves);

Domain *testDomains[] = { // NOLINT(cert-err58-cpp)
    &gs1, &gs2, &gs3, &gs1_fix, &gs2_fix, &gs3_fix,
    &iigs1, &iigs2, &iigs3, &iigs1_fix, &iigs2_fix, &iigs3_fix,
    // todo: maxUtility and maxDepth do not work for poker!
    // &gp1, &gp2,
    &oz1, &oz2, &oz3, &oz4, &oz5, &iioz1, &iioz2, &iioz3, &iioz4, &iioz5,
    &mp1, &mp2,
};

TEST(Domain, ZeroSumGame) {
    for (auto domain : testDomains) {
        cout << "\nchecking " << domain->getInfo() << "\n";
        EXPECT_TRUE(isDomainZeroSum(*domain));
    }
}

//TEST(Domain, CheckEFGNodeStateEqualityConsistency) {
//    for (auto domain : testDomains) {
//        cout << "\nchecking " << domain->getInfo() << "\n";
//        EXPECT_TRUE(isEFGNodeAndStateConsistent(*domain));
//    }
//}

TEST(Domain, CheckAvailableActionsAreSorted) {
    for (auto domain : testDomains) {
        cout << "\nchecking " << domain->getInfo() << "\n";
        EXPECT_TRUE(areAvailableActionsSorted(*domain));
    }
}

TEST(Domain, MaxUtility) {
    for (auto domain : testDomains) {
        cout << "\nchecking " << domain->getInfo() << "\n";
        EXPECT_EQ(domainFindMaxUtility(*domain), domain->getMaxUtility());
    }
}

TEST(Domain, MaxDepth) {
    for (auto domain : testDomains) {
        cout << "\nchecking " << domain->getInfo() << "\n";
        EXPECT_EQ(domainFindMaxDepth(*domain), domain->getMaxStateDepth());
    }
}

TEST(Domain, CreatesRootNode) {
    for (auto domain : testDomains) {
        EXPECT_NE(createRootEFGNode(*domain), nullptr);
    }
}

//BOOST_AUTO_TEST_CASE(checkNumPlayersCountActionsConsistentInState) {
//    for (auto domain : testDomains) {
//        BOOST_CHECK(isNumPlayersCountActionsConsistentInState(*domain));
//    }
//}


}


