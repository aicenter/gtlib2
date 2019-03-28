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


#include "algorithms/cfr.h"
#include "domains/matching_pennies.h"
#include "tests/domainsTest.h"
#include <boost/test/unit_test.hpp>
#include <algorithms/utility.h>
#include "algorithms/tree.h"
#include "domains/goofSpiel.h"
#include "algorithms/bestResponse.h"


namespace GTLib2 {

using domains::MatchingPenniesDomain;
using domains::MatchingPenniesAction;
using algorithms::CFRData;
using algorithms::CFRiterations;
using algorithms::buildForest;
using algorithms::bestResponseTo;


BOOST_AUTO_TEST_SUITE(CFRTest)

BOOST_AUTO_TEST_CASE(CheckRegretsAndAccInSmallDomain) {
    MatchingPenniesDomain mp;
    CFRData data(mp.getRootStatesDistribution());
    buildForest(&data);
    auto rootNode = data.getRootNodes()[0].first;
    auto rootInfoset = rootNode->getAOHInfSet();
    auto childNode = rootNode->performAction(rootNode->availableActions()[0])[0].first;
    auto childInfoset = childNode->getAOHInfSet();

    // ------ iteration player 0 ------
    double cfvInfoset = CFRiteration(&data, rootNode, new double[2]{1., 1.}, Player(0));
    auto&[regRoot, accRoot] = data.infosetData.at(rootInfoset);
    auto&[regChild, accChild] = data.infosetData.at(childInfoset);
    BOOST_CHECK(cfvInfoset == 0.0);
    BOOST_CHECK(regRoot[0] == 0.0);
    BOOST_CHECK(regRoot[1] == 0.0);
    BOOST_CHECK(accRoot[0] == 0.5);
    BOOST_CHECK(accRoot[1] == 0.5);
    // does not change regrets / acc for player 1
    BOOST_CHECK(regChild[0] == 0.0);
    BOOST_CHECK(regChild[1] == 0.0);
    BOOST_CHECK(accChild[0] == 0.0);
    BOOST_CHECK(accChild[1] == 0.0);

    // ------ iteration player 1 ------
    cfvInfoset = CFRiteration(&data, rootNode, new double[2]{1., 1.}, Player(1));
    auto &[regRoot2, accRoot2] = data.infosetData.at(rootInfoset);
    auto &[regChild2, accChild2] = data.infosetData.at(childInfoset);
    // does not change regrets / acc for player 0
    BOOST_CHECK(regRoot2[0] == 0.0);
    BOOST_CHECK(regRoot2[1] == 0.0);
    BOOST_CHECK(accRoot2[0] == 0.5);
    BOOST_CHECK(accRoot2[1] == 0.5);
    BOOST_CHECK(regChild2[0] == 0.5);
    BOOST_CHECK(regChild2[1] == 0.5);
    // acc is not two equal numbers because first
    // the left node is traversed and then the right node of the infoset
    BOOST_CHECK(accChild2[0] == 0.5);
    BOOST_CHECK(accChild2[1] == 1.5);
    // this is also why cfv is not zero
    BOOST_CHECK(cfvInfoset == -0.5);
}

BOOST_AUTO_TEST_SUITE_END()


}  // namespace GTLib2
