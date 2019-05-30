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


#ifndef ALGORITHMS_OOS_TEST_H_
#define ALGORITHMS_OOS_TEST_H_

#include "algorithms/oos.h"


namespace GTLib2::algorithms {

class FixedSamplingOOS: public OOSAlgorithm {
 public:
    FixedSamplingOOS(const Domain &domain, Player playingPlayer, OOSData &cache,
                     const OOSSettings &cfg, vector<vector<ActionId>> samples)
        : OOSAlgorithm(domain, playingPlayer, cache, cfg), samples_(move(samples)) {};

 protected:
    vector<vector<ActionId>> samples_;
    int moveIdx_ = 0;

    inline int nextAction() {
        assert(samples_.at(stats_.rootVisits).size() > moveIdx_);
        return samples_.at(stats_.rootVisits).at(moveIdx_++);
    }

    inline void rootIteration(double compensation, Player exploringPl) override {
        moveIdx_ = 0;
        assert(stats_.rootVisits < samples_.size());
        OOSAlgorithm::rootIteration(compensation, exploringPl);
    }

    pair<int, RandomLeafOutcome> selectLeaf(const shared_ptr<EFGNode> &h,
                                            const vector<shared_ptr<Action>> &actions) override;
    inline int
    selectChanceAction(const shared_ptr<EFGNode> &h, double bsum) override { return nextAction(); };
    inline int selectExploringPlayerAction(const shared_ptr<EFGNode> &h,
                                           int biasApplicableActions,
                                           double bsum) override { return nextAction(); };
    inline int selectNonExploringPlayerAction(const shared_ptr<EFGNode> &h,
                                              double bsum) override { return nextAction(); };

    inline double handleTerminalNode(const shared_ptr<EFGNode> &h,
                                     double bs_h_all, double us_h_all,
                                     Player exploringPl) override {
        assert(samples_.at(stats_.rootVisits).size() == moveIdx_);
        return OOSAlgorithm::handleTerminalNode(h, bs_h_all, us_h_all, exploringPl);
    };

};

}  // namespace GTLib2


#endif  // ALGORITHMS_OOS_TEST_H_
