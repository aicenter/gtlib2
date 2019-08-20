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
    Exp3Selector(const vector<shared_ptr<Action>>& actions, Exp3SelectorFactory * fact) : fact_(fact), gamma_(fact->gamma) {
        actionProbability_ = vector<double>(actions.size());
        actionMeanProbability_ = vector<double>(actions.size());
        rewards_ = vector<double>(actions.size());}
    Exp3Selector(int actionsNumber, Exp3SelectorFactory * fact) : fact_(fact), gamma_(fact->gamma) {
        actionProbability_ = vector<double>(actionsNumber);
        actionMeanProbability_ = vector<double>(actionsNumber);
        rewards_ = vector<double>(actionsNumber);}
    int select() override;
    void update(int ai, double value) override;
    ProbDistribution getActionsProbDistribution() override ;
private:
    Exp3SelectorFactory * fact_;
    /** Current probability of playing this action. */
    vector<double> actionProbability_;
    /** Mean strategy. */
    vector<double> actionMeanProbability_;
    /** Cumulative reward. */
    vector<double> rewards_;
    const double gamma_;

    void updateProb();
};
}

#endif //GTLIB2_EXP3SELECTOR_H
