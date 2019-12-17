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
#include "base/fogefg.h"

#include "algorithms/bestResponse.h"
#include "algorithms/cfr.h"
#include "algorithms/strategy.h"
#include "algorithms/continualResolving.h"
#include "algorithms/oos.h"

#include "algorithms/utility.h"
#include "domains/goofSpiel.h"
#include "domains/simple_games.h"

#include "gtest/gtest.h"


namespace GTLib2::algorithms {

using domains::GoofSpielDomain;
using domains::RPSDomain;

class ExampleCR: public ContinualResolving {
 public:
    ExampleCR(const Domain &domain, Player playingPlayer, OOSData &cache)
        : ContinualResolving(domain, playingPlayer, cache) {}

    inline PlayControl preplayIteration(const shared_ptr<EFGNode> &rootNode) override {
        cache_.buildTree();
        return StopImproving;
    }
    // resolve as: play only action at index of "possible actions % public state depth"
    inline PlayControl resolveIteration(const shared_ptr<GadgetRootNode> &rootNode,
                                        const shared_ptr<AOH> &currentInfoset) override {
        const auto &summary = rootNode->gadget_.summary_;
        for (auto &h : summary.topmostHistories) {
            if (h->type_ != PlayerNode) continue;

            const auto infoset = h->getAOHInfSet();
            const auto aodepth = infoset->getAOids().size();
            const auto numActions = h->countAvailableActions();
            cache_.infosetData.at(infoset).avgStratAccumulator.at(aodepth % numActions) = 1.0;
        }
        return StopImproving;
    }
};

//TEST(ContinualResolving, ResolverStepsThroughPublicStates) {
//    auto rps = RPSDomain();
//    auto iigs = GoofSpielDomain::IIGS(4);
//    auto domains = vector<Domain *>{&rps, iigs.get()};
//
//    for (auto d : domains) {
//        auto root = createRootEFGNode(*d);
//
//        auto cache = OOSData(*d);
//        auto cr = ExampleCR(*d, Player(0), cache);
//        cr.solveEntireGame(1, 1, BudgetIterations); // todo: solving only for "owner" of ps?
//
//        for (const auto &[is, nodes] : cache.getInfoset2NodeMapping()) {
//            const auto aodepth = is->getAOids().size();
//            const auto aHistory = *nodes.begin();
//            const auto numActions = aHistory->countAvailableActions();
//            if (aHistory->type_ == PlayerNode && is->getPlayer() == aHistory->getPlayer())
//                EXPECT_EQ(cache.infosetData.at(is).avgStratAccumulator.at(aodepth % numActions),
//                          1.0);
//        }
//    }
//}

}
