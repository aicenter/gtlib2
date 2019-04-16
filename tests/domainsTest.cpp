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

bool isDomainMaxUtilityCorrect(const Domain &domain) {
    double maxLeafUtility = 0;
    auto countViolations = [&maxLeafUtility](shared_ptr<EFGNode> node) {
        maxLeafUtility = max({node->rewards_[0], node->rewards_[1], maxLeafUtility});
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return maxLeafUtility == domain.getMaxUtility();
}

bool isDomainMaxDepthCorrect(const Domain &domain) {
    int maxDepth = 0;
    auto countViolations = [&maxDepth](shared_ptr<EFGNode> node) {
        maxDepth = max(node->getDepth(), maxDepth);
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return maxDepth == domain.getMaxDepth();
}

bool doesCreateRootNodes(const Domain &domain) {
    return !createRootEFGNodes(domain).empty();
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
// @formatter:on

GenericPokerDomain gp1(2, 2, 2, 2, 2);
GenericPokerDomain gp2(3, 3, 1, 2, 3);

OshiZumoDomain oz1(3, 3, 1);
OshiZumoDomain oz2(3, 0, 1);
OshiZumoDomain oz3(1, 3, 0);
OshiZumoDomain oz4(3, 3, 1, false);
OshiZumoDomain oz5(5, 3, 1, false);

MatchingPenniesDomain mp1(AlternatingMoves);
MatchingPenniesDomain mp2(SimultaneousMoves);

Domain *testDomains[] = { // NOLINT(cert-err58-cpp)
    &gs1, &gs2, &gs3, &gs1_fix, &gs2_fix, &gs3_fix,
    &iigs1, &iigs2, &iigs3, &iigs1_fix, &iigs2_fix, &iigs3_fix,
    // todo: maxUtility and maxDepth do not work for poker!
    // &gp1, &gp2,
    &oz1, &oz2, &oz3, &oz4, &oz5,
    &mp1, &mp2,
};

BOOST_AUTO_TEST_CASE(zeroSumGame) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isDomainZeroSum(*domain));
    }
}

BOOST_AUTO_TEST_CASE(checkEFGNodeStateEqualityConsistency) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isEFGNodeAndStateConsistent(*domain));
    }
}

BOOST_AUTO_TEST_CASE(checkAvailableActionsAreSorted) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(areAvailableActionsSorted(*domain));
    }
}

BOOST_AUTO_TEST_CASE(maxUtility) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isDomainMaxUtilityCorrect(*domain));
    }
}

BOOST_AUTO_TEST_CASE(maxDepth) {
    for (auto domain : testDomains) {
        cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isDomainMaxDepthCorrect(*domain));
    }
}

BOOST_AUTO_TEST_CASE(createsRootNodes) {
    for (auto domain : testDomains) {
        BOOST_CHECK(doesCreateRootNodes(*domain));
    }
}


BOOST_AUTO_TEST_SUITE_END()
}


