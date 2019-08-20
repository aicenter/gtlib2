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

#ifndef GTLIB2_ISMCTS_H
#define GTLIB2_ISMCTS_H
#include "base/algorithm.h"
#include "selectors/UCTSelector.h"
#include "base/random.h"

namespace GTLib2::algorithms {
struct ISMCTSSettings{
    shared_ptr<SelectorFactory> fact_;
    //unsigned long batchSize = 1;

    bool useBelief = false;

    unsigned long randomSeed = 0;
};

class ISMCTS : public GamePlayingAlgorithm {
public:
    explicit ISMCTS(const Domain &domain, Player playingPlayer, ISMCTSSettings config) :
        GamePlayingAlgorithm(domain, playingPlayer), config_(config), rootNode_(createRootEFGNode(domain)) {
        generator_ = std::mt19937(config.randomSeed);
        belief_ = {1.0};
    };

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;
    optional<ProbDistribution> getPlayDistribution(const shared_ptr<AOH> &currentInfoset) override;

    virtual void setCurrentInfoset(const shared_ptr<AOH> &newInfoset);

protected:
    const ISMCTSSettings config_;
    std::mt19937 generator_;
    unordered_map<shared_ptr<AOH>, unique_ptr<Selector>> infosetSelectors_;
    const shared_ptr<EFGNode> rootNode_;
    shared_ptr<AOH> currentInfoset_;
    vector<double> belief_;
    bool giveUp_ = false;


    virtual double iteration(const shared_ptr <EFGNode> &h);

    double handleTerminalNode(const shared_ptr <EFGNode> &h);

    double handleChanceNode(const shared_ptr <EFGNode> &h);

    virtual double handlePlayerNode(const shared_ptr <EFGNode> &h);

    double simulate(const shared_ptr <EFGNode> &h);
};
}

#endif //GTLIB2_ISMCTS_H
