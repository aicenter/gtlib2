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

#ifndef GTLIB2_CPW_ISMCTS_H
#define GTLIB2_CPW_ISMCTS_H

#include "ISMCTS.h"
#include "algorithms/common.h"




//auto fact = make_shared<UCTSelectorFactory>(sqrt(2));
//int a = 0;
//ISMCTSSettings s = {.useBelief = true, .fact_ = std::static_pointer_cast<SelectorFactory>(fact), .randomSeed = 123};
//PreparedAlgorithm firstAction = createInitializer<ISMCTS>(s);
//PreparedAlgorithm lastAction = createInitializer<RandomPlayer>();
//
//GTLib2::domains::GoofSpielSettings settings
//        ({variant:  GTLib2::domains::IncompleteObservations, numCards: 3, fixChanceCards: false});
////    settings.shuffleChanceCards(2);
//GoofSpielDomain domain(settings);
//vector<double> actualUtilities = playMatch(
//        domain, vector<PreparedAlgorithm>{firstAction, lastAction},
//        vector<int>{10000, 10000}, vector<int>{100, 100}, BudgetIterations, 0);
//
////    GTLib2::domains::StrategoSettings settings = {3,2,{},{'1', '2'}};
////    GTLib2::domains::StrategoDomain domain(settings);
////    vector<double> actualUtilities = playMatch(
////            domain, vector<PreparedAlgorithm>{firstAction, lastAction},
////            vector<int>{1000, 1000}, vector<int>{10, 10}, BudgetIterations, 0);



namespace GTLib2::algorithms {
class CPW_ISMCTS : public ISMCTS {
public:
    explicit CPW_ISMCTS(const Domain &domain, Player playingPlayer, ISMCTSSettings config) : ISMCTS(domain, playingPlayer, config) {};
    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;

private:
    unordered_map<shared_ptr<AOH>, vector<shared_ptr<EFGNode>>> nodesMap_;
    double handlePlayerNode(const shared_ptr<EFGNode> &h) override;

    void setCurrentInfoset(const shared_ptr <AOH> &newInfoset) override;

    void fillBelief(const shared_ptr <EFGNode> &currentNode, const shared_ptr <AOH> &newInfoset, const double prob, vector<shared_ptr<EFGNode>> newNodes);
};
}

#endif //GTLIB2_CPW_ISMCTS_H
