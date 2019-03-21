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
#include "algorithms/treeWalk.h"

#include <boost/test/unit_test.hpp>
#include <domains/matching_pennies.h>
#include "domains/goofSpiel.h"
#include "domains/genericPoker.h"


namespace GTLib2 {

using domains::GoofSpielDomain;
using domains::IIGoofSpielDomain;
using domains::GenericPokerDomain;
using domains::MatchingPenniesDomain;
using domains::SimultaneousMatchingPenniesDomain;
using algorithms::treeWalkEFG;

bool isDomainZeroSum(const Domain &domain) {
  int num_violations = 0;
  auto countViolations = [&num_violations](shared_ptr<EFGNode> node) {
    if (node->rewards_[0] != -node->rewards_[1]) num_violations++;
  };

  treeWalkEFG(domain, countViolations, domain.getMaxDepth());
  return num_violations == 0;
}

BOOST_AUTO_TEST_SUITE(DomainTests)

// todo: make nicer initialization?
GoofSpielDomain gs1(3, 1, nullopt);
GoofSpielDomain gs2(3, 2, nullopt);
GoofSpielDomain gs3(3, 3, nullopt);
GoofSpielDomain gs4(3, 4, nullopt);
GoofSpielDomain gs5(4, 4, nullopt);
GenericPokerDomain gp1(2, 2, 2, 2, 2);
GenericPokerDomain gp2(3, 3, 1, 2, 3);
MatchingPenniesDomain mp1;
SimultaneousMatchingPenniesDomain mp2;

Domain* testDomains[] = { // NOLINT(cert-err58-cpp)
    & gs1, & gs2, & gs3, & gs4, & gs5,
    & gp1, & gp2,
    & mp1, & mp2,
};

BOOST_AUTO_TEST_CASE(zeroSumGame) {
  for (auto domain : testDomains) {
    BOOST_CHECK(isDomainZeroSum(*domain));
  }
}


BOOST_AUTO_TEST_SUITE_END()
}


