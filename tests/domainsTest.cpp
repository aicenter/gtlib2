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


#include "tests/domainsTest.h"

#include <memory>
#include "algorithms/tree.h"

#include <boost/test/unit_test.hpp>
#include <domains/matching_pennies.h>
#include "domains/goofSpiel.h"
#include "domains/genericPoker.h"
#include "domains/oshiZumo.h"


namespace GTLib2 {

using domains::GoofSpielDomain;
using domains::IIGoofSpielDomain;
using domains::GenericPokerDomain;
using domains::MatchingPenniesDomain;
using domains::SimultaneousMatchingPenniesDomain;
using domains::OshiZumoDomain;
using algorithms::treeWalkEFG;

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
    EFGCache cache(domain.getRootStatesDistribution());
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
            if(actions[j]->getId() != j) num_violations++;
        }
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return num_violations == 0;
}

bool isDomainMaxUtilityCorrect(const Domain &domain) {
    double maxLeafUtility = 0;
    auto countViolations = [&maxLeafUtility](shared_ptr<EFGNode> node) {
        maxLeafUtility = std::max({node->rewards_[0], node->rewards_[1], maxLeafUtility});
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return maxLeafUtility == domain.getMaxUtility();
}

bool isDomainMaxDepthCorrect(const Domain &domain) {
    int maxDepth = 0;
    auto countViolations = [&maxDepth](shared_ptr<EFGNode> node) {
        maxDepth = std::max(node->getDepth(), maxDepth);
    };

    treeWalkEFG(domain, countViolations, domain.getMaxDepth());
    return maxDepth == domain.getMaxDepth();
}

BOOST_AUTO_TEST_SUITE(DomainTests)

GoofSpielDomain gs1(3, nullopt);
GoofSpielDomain gs2(3, nullopt);
GoofSpielDomain gs3(3, nullopt);
GoofSpielDomain gs4(3, nullopt);
GoofSpielDomain gs5(4, nullopt);
GenericPokerDomain gp1(2, 2, 2, 2, 2);
GenericPokerDomain gp2(3, 3, 1, 2, 3);
OshiZumoDomain oz1(3, 3, 1);
OshiZumoDomain oz2(3, 0, 1);
OshiZumoDomain oz3(1, 3, 0);
OshiZumoDomain oz4(3, 3, 1, false);
OshiZumoDomain oz5(5, 3, 1, false);
MatchingPenniesDomain mp1;
SimultaneousMatchingPenniesDomain mp2;

Domain* testDomains[] = { // NOLINT(cert-err58-cpp)
    & gs1, & gs2, & gs3, & gs4, & gs5,
    & gp1, & gp2,
    & oz1, & oz2, & oz3, & oz4, & oz5,
    & mp1, & mp2,
};

BOOST_AUTO_TEST_CASE(zeroSumGame) {
    for (auto domain : testDomains) {
        std::cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isDomainZeroSum(*domain));
    }
}

BOOST_AUTO_TEST_CASE(checkEFGNodeStateEqualityConsistency) {
    for (auto domain : testDomains) {
        std::cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isEFGNodeAndStateConsistent(*domain));
    }
}

BOOST_AUTO_TEST_CASE(checkAvailableActionsAreSorted) {
    for (auto domain : testDomains) {
        std::cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(areAvailableActionsSorted(*domain));
    }
}

BOOST_AUTO_TEST_CASE(maxUtility) {
    for (auto domain : testDomains) {
        std::cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isDomainMaxUtilityCorrect(*domain));
    }
}

BOOST_AUTO_TEST_CASE(maxDepth) {
    for (auto domain : testDomains) {
        std::cout << "checking " << domain->getInfo() << "\n";
        BOOST_CHECK(isDomainMaxDepthCorrect(*domain));
    }
}


BOOST_AUTO_TEST_SUITE_END()
}


