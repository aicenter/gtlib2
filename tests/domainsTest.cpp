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

#include <boost/test/unit_test.hpp>
#include "algorithms/common.h"
#include "domains/genericPoker.h"
#include "domains/goofSpiel.h"
#include "domains/matching_pennies.h"
#include "domains/oshiZumo.h"
#include "domains/randomGame.h"

namespace GTLib2::domains {

using algorithms::treeWalkEFG;
using GoofSpielVariant::IncompleteObservations;
using GoofSpielVariant::CompleteObservations;
using algorithms::createRootEFGNodes;

bool isDomainZeroSum(const Domain &domain) {
    int num_violations = 0;
    auto countViolations = [&num_violations](shared_ptr<EFGNode> node) {
      if (node->rewards_[0] != -node->rewards_[1]) num_violations++;
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return num_violations == 0;
}

bool isEFGNodeAndStateConsistent(const Domain &domain) {
    int num_violations = 0;
    EFGCache cache(domain);
    cache.buildForest(domain.getMaxDepth());
    auto nodes = cache.getNodes();
    for (const auto &n1: nodes) {
        for (const auto &n2: nodes) {
            if (n1 == n2 && (n1->getHash() != n2->getHash() || n1->getState() != n2->getState())) {
                num_violations++;
            }
        }
    }
    return num_violations == 0;
}

bool areAvailableActionsSorted(const Domain &domain) {
    int num_violations = 0;
    auto countViolations = [&num_violations](shared_ptr<EFGNode> node) {
        auto actions = node->availableActions();
        for (int j = 0; j < actions.size(); ++j) {
            if (actions[j]->getId() != j) num_violations++;
        }
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return num_violations == 0;
}

bool isDomainMaxUtilityCorrect(const Domain &domain, bool strict) {
    double maxLeafUtility = 0;
    auto countViolations = [&maxLeafUtility](shared_ptr<EFGNode> node) {
        maxLeafUtility = max({node->rewards_[0], node->rewards_[1], maxLeafUtility});
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    if (strict) {
        return maxLeafUtility == domain.getMaxUtility();
    }
    return maxLeafUtility <= domain.getMaxUtility();
}

bool isDomainMaxDepthCorrect(const Domain &domain) {
    int maxDepth = 0;
    auto countViolations = [&maxDepth](shared_ptr<EFGNode> node) {
        maxDepth = max(node->getStateDepth(), maxDepth);
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return maxDepth == domain.getMaxDepth();
}

bool doesCreateRootNodes(const Domain &domain) {
    return !createRootEFGNodes(domain).empty();
}

bool isActionGenerationAndAOHConsistent(const Domain &domain) {
    int num_violation = 0;
    std::vector<std::unordered_map<size_t, std::vector<shared_ptr<Action>>>> maps
        (domain.getNumberOfPlayers(),
         std::unordered_map<size_t, std::vector<shared_ptr<Action>>>());

    auto countViolations = [&num_violation, &maps](shared_ptr<EFGNode> node) {
      auto aoh = node->getAOHInfSet();
      if (aoh) {
          size_t hashAOH = aoh->getHash();
          Player currentPlayer = *node->getCurrentPlayer();
          auto actionsNode = node->availableActions();
          auto mappedAOH = maps[currentPlayer].find(hashAOH);
          if (mappedAOH != maps[currentPlayer].end()) {
              auto actionsMappedAOH = mappedAOH->second;

              if (actionsNode.size() == actionsMappedAOH.size()) {
                  for (int j = 0; j < actionsNode.size(); ++j) {
                      if (!(*actionsNode[j] == *actionsMappedAOH[j])) {
                          num_violation++;
                      }
                  }
              } else {
                  num_violation++;
              }
          } else {
              maps[currentPlayer].insert({hashAOH, actionsNode});
          }
      }
    };
    treeWalkEFG(domain, countViolations, domain.getMaxDepth());

    return num_violation == 0;
}

BOOST_AUTO_TEST_SUITE(DomainsTests)


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

RandomGameDomain rg1({});
RandomGameDomain rg2
    ({.seed = 13, .maximalDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 4, .maxRewardModification = 2, .maxUtility = 100, .binaryUtility = true, .utilityCorrelation = true, .fixedBranchingFactor = true});
RandomGameDomain rg3
    ({.seed = 13, .maximalDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 2, .maxUtility = 100, .binaryUtility = true, .utilityCorrelation = true, .fixedBranchingFactor = true});
RandomGameDomain rg4
    ({.seed = 13, .maximalDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 2, .maxUtility = 100, .binaryUtility = true, .utilityCorrelation = true, .fixedBranchingFactor = false});
RandomGameDomain rg5
    ({.seed = 13, .maximalDepth = 3, .maxBranchingFactor = 4, .maxDifferentObservations = 2, .maxRewardModification = 2, .maxUtility = 100, .binaryUtility = false, .utilityCorrelation = true, .fixedBranchingFactor = false});

// @formatter:on
GenericPokerDomain gp1(2, 2, 2, 2, 2);
GenericPokerDomain gp2(3, 3, 1, 2, 3);

MatchingPenniesDomain mp1(AlternatingMoves);
MatchingPenniesDomain mp2(SimultaneousMoves);


Domain *testDomains[] = { // NOLINT(cert-err58-cpp)
    &gs1, &gs2, &gs3, &gs1_fix, &gs2_fix, &gs3_fix,
    &iigs1, &iigs2, &iigs3, &iigs1_fix, &iigs2_fix, &iigs3_fix,
//    // todo: maxUtility and maxDepth do not work for poker!
//    // &gp1, &gp2,
    &oz1, &oz2, &oz3, &oz4, &oz5, &iioz1, &iioz2, &iioz3, &iioz4, &iioz5,
    &mp1, &mp2,
    &rg1, &rg2, &rg3, &rg4, &rg5,
};

BOOST_AUTO_TEST_CASE(zeroSumGame) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << endl;
        BOOST_CHECK(isDomainZeroSum(*domain));
    }
}

BOOST_AUTO_TEST_CASE(checkEFGNodeStateEqualityConsistency) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << endl;
        BOOST_CHECK(isEFGNodeAndStateConsistent(*domain));
    }
}

BOOST_AUTO_TEST_CASE(checkAvailableActionsAreSorted) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << endl;
        BOOST_CHECK(areAvailableActionsSorted(*domain));
    }
}

BOOST_AUTO_TEST_CASE(maxUtility) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << endl;
        if (typeid(*domain) == typeid(RandomGameDomain)) {
            BOOST_CHECK(isDomainMaxUtilityCorrect(*domain, false));
        } else {
            BOOST_CHECK(isDomainMaxUtilityCorrect(*domain, true));
        }
    }
}

BOOST_AUTO_TEST_CASE(maxDepth) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << endl;
        BOOST_CHECK(isDomainMaxDepthCorrect(*domain));
    }
}

BOOST_AUTO_TEST_CASE(createsRootNodes) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << endl;
        BOOST_CHECK(doesCreateRootNodes(*domain));
    }
}

BOOST_AUTO_TEST_CASE(AOHconsistency) {
    for (auto &domain : testDomains) {
        cout << "checking " << domain->getInfo() << endl;
        BOOST_CHECK(isActionGenerationAndAOHConsistent(*domain));
    }
}

BOOST_AUTO_TEST_SUITE_END()
}


