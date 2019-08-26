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

#ifndef GTLIB2_RMSELECTOR_H
#define GTLIB2_RMSELECTOR_H

#include <utility>

#include "RMSelectorFactory.h"

namespace GTLib2::algorithms {
class RMSelector : public Selector  {
public:
    RMSelector(const vector<shared_ptr<Action>>& actions, const RMSelectorFactory * fact) : fact_(fact) {
        actionProbability = vector<double>(actions.size());
        actionMeanProbability = vector<double>(actions.size());
        regretEstimate = vector<double>(actions.size());}
    RMSelector(int actionsNumber, const RMSelectorFactory * fact) : fact_(fact) {
        actionProbability = vector<double>(actionsNumber);
        actionMeanProbability = vector<double>(actionsNumber);
        regretEstimate = vector<double>(actionsNumber);}

    ActionId select() override;
    void update(ActionId ai, double value) override;
    ProbDistribution getActionsProbDistribution() override ;

private:
    const RMSelectorFactory * fact_;
    /** Current probability of playing this action. */
    ProbDistribution actionProbability;
    vector<double> actionMeanProbability;
        /** Cumulative regret estimate. */
    vector<double> regretEstimate;

    void updateProb();
};
}

#endif //GTLIB2_RMSELECTOR_H
