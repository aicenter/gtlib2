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


#ifndef GTLIB2_CONTINUALRESOLVING_H
#define GTLIB2_CONTINUALRESOLVING_H

#include "base/efg.h"
#include "base/fogefg.h"
#include "base/gadget.h"
#include "algorithms/cfr.h"
#include "algorithms/oos.h"

namespace GTLib2::algorithms {

/**
 * When accessing terminal leaf nodes, you must multiple utilities by the pubStateReach_ term of the gadget game!
 */
class ContinualResolving: public GamePlayingAlgorithm {
 protected:
    OOSData &cache_;
    unique_ptr<GadgetGame> gadget_;
    shared_ptr<GadgetRootNode> gadgetRoot_;

 public:
    ContinualResolving(const Domain &domain, Player playingPlayer, OOSData &cache)
        : GamePlayingAlgorithm(domain, playingPlayer), cache_(cache), gadget_(nullptr) {}

    virtual ~ContinualResolving() = default;

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override {
        // we can't make targetting, if the infoset is not in cache. Give up - play randomly
        // todo: make partial targetting up to the last known infoset at least?
        if (currentInfoset && !cache_.hasPublicStateFor(*currentInfoset)) return GiveUp;

        if (playInfoset_ != currentInfoset) {
            // new infoset!
            playInfoset_ = currentInfoset;
            playPublicState_ = playInfoset_ ? optional(cache_.getPublicStateFor(*playInfoset_))
                                            : nullopt;

            updateGadget();
        }

        if (currentInfoset == nullopt)
            return preplayIteration(cache_.getRootNode());
        else
            return resolveIteration(gadget_->getRootNode(), *currentInfoset);
    }

    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override {
        return cache_.strategyFor(currentInfoset);
    }

    virtual void updateGadget() {
        gadget_ = make_unique<GadgetGame>(
            cache_.getPublicStateSummary(*playPublicState_), playingPlayer_,
            *playInfoset_, SAFE_RESOLVING);

        gadgetRoot_ = gadget_->getRootNode();
    }

    virtual PlayControl preplayIteration(const shared_ptr<EFGNode> &rootNode) = 0;
    virtual PlayControl resolveIteration(const shared_ptr<GadgetRootNode> &rootNode,
                                         const shared_ptr<AOH> &currentInfoset) = 0;

 protected:
    optional<shared_ptr<AOH>> playInfoset_ = nullopt;
    optional<shared_ptr<PublicState>> playPublicState_ = nullopt;

};

}

#endif //GTLIB2_CONTINUALRESOLVING_H
