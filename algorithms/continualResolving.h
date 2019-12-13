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
#include "utils/export.h"

namespace GTLib2::algorithms {

/**
 * When accessing terminal leaf nodes, you must multiple utilities by the pubStateReach_ term of the gadget game!
 */
class ContinualResolving: public GamePlayingAlgorithm {
 protected:
    unique_ptr<GadgetGame> gadget_;
    shared_ptr<GadgetRootNode> gadgetRoot_;

 public:
    ContinualResolving(const Domain &domain, Player playingPlayer)
        : GamePlayingAlgorithm(domain, playingPlayer), gadget_(nullptr) {}

    virtual ~ContinualResolving() = default;

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override {
        // we can't make targetting, if the infoset is not in cache. Give up - play randomly
        // todo: make partial targetting up to the last known infoset at least?
        if (currentInfoset && !getCache().hasPublicStateFor(*currentInfoset)) {
            return GiveUp;
        }

        if (playInfoset_ != currentInfoset) {
            // new infoset!
            playInfoset_ = currentInfoset;
            playPublicState_ = playInfoset_ ? optional(getCache().getPublicStateFor(*playInfoset_))
                                            : nullopt;

            updateGadget();
        }

        if (currentInfoset == nullopt)
            return preplayIteration(getCache().getRootNode());
        else
            return resolveIteration(gadget_->getRootNode(), *currentInfoset);
    }

    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override {
        return getCache().strategyFor(currentInfoset);
    }

    virtual void updateGadget() {
        const auto summary = getCache().getPublicStateSummary(*playPublicState_);

        GadgetVariant variant = UNSAFE_RESOLVING;
        // if opponent's augmented infoset spans all histories, it is not necessary
        // to build safe gadget, we only need chance node
        const auto oneOpponentAOH = summary.topmostHistories.at(0)->getAOHAugInfSet(opponent(playingPlayer_));
        for (const auto& h : summary.topmostHistories) {
            if(h->getAOHAugInfSet(opponent(playingPlayer_)) != oneOpponentAOH) {
                variant = SAFE_RESOLVING;
                break;
            }
        }

        gadget_ = make_unique<GadgetGame>(summary, playingPlayer_, *playInfoset_, variant);
        gadgetRoot_ = gadget_->getRootNode();
        // for debugging:
//        utils::exportGambit(gadgetRoot_, "export_" + (*playPublicState_)->toString()
//        + "_pl"+to_string(int(playingPlayer_))+".gbt");
    }

    virtual const OOSData &getCache() const = 0;
    virtual PlayControl preplayIteration(const shared_ptr<EFGNode> &rootNode) = 0;
    virtual PlayControl resolveIteration(const shared_ptr<GadgetRootNode> &rootNode,
                                         const shared_ptr<AOH> &currentInfoset) = 0;

 protected:
    optional<shared_ptr<AOH>> playInfoset_ = nullopt;
    optional<shared_ptr<PublicState>> playPublicState_ = nullopt;

};

}

#endif //GTLIB2_CONTINUALRESOLVING_H
