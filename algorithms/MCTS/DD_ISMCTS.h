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

#ifndef GTLIB2_ALGORITHMS_MCTS_DD_ISMCTS_H_
#define GTLIB2_ALGORITHMS_MCTS_DD_ISMCTS_H_
#include "CPW_ISMCTS.h"
#include "domains/stratego.h"

namespace GTLib2::algorithms {
class DD_ISMCTS : public CPW_ISMCTS {
 public:
    explicit DD_ISMCTS(const Domain &domain, Player playingPlayer, ISMCTSSettings config) :
        CPW_ISMCTS(domain, playingPlayer, std::move(config)), generateIters_(config.generateIters), iterateRoot_(config.iterateRoot) {
        dynamic_cast<const ExtendedDomain&>(domain).prepareRevealedMap(revealed_); // check is domain is appliable and prepare map
    };

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;

    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset, long actionsNum) override;

    void setCurrentInfoset(const shared_ptr<AOH> &newInfoset) override;

 private:
    bool isCurrentISUndiscovered_ = false;
    unordered_map<unsigned long, shared_ptr<RevealedInfo>> revealed_;
    long  lastRevealAoid_ = -1;
    bool currentISChecked_ = true;
    const int generateIters_ = 1000;
    bool iterateRoot_ = false;
};
}
#endif //GTLIB2_ALGORITHMS_MCTS_DD_ISMCTS_H_
