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

#ifndef GTLIB2_EXP3SELECTOR_H
#define GTLIB2_EXP3SELECTOR_H

#include "Exp3SelectorFactory.h"

namespace GTLib2::algorithms {
class Exp3Selector : public Selector {
public:
    Exp3Selector(const vector<shared_ptr<Action>>& actions, const Exp3SelectorFactory * fact) : fact_(fact) {
        actionProbability_ = vector<double>(actions.size());
        actionMeanProbability_ = vector<double>(actions.size());
        rewards_ = vector<double>(actions.size());}
    Exp3Selector(int actionsNumber, const Exp3SelectorFactory * fact) : fact_(fact) {
        actionProbability_ = vector<double>(actionsNumber);
        actionMeanProbability_ = vector<double>(actionsNumber);
        rewards_ = vector<double>(actionsNumber);}
    ActionId select() override;
    void update(ActionId ai, double value) override;
    ProbDistribution getActionsProbDistribution() override ;
private:
    const Exp3SelectorFactory * fact_;
    /** Current probability of playing this action. */
    ProbDistribution actionProbability_;
    /** Mean strategy. */
    vector<double> actionMeanProbability_;
    /** Cumulative reward. */
    vector<double> rewards_;

    void updateProb();
};
}

#endif //GTLIB2_EXP3SELECTOR_H
