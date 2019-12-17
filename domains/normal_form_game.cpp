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


#include "base/base.h"
#include "domains/normal_form_game.h"


namespace GTLib2::domains {

vector<vector<double>> NFGSettings::getUtilities(vector<vector<double>> twoPlayerZeroSumMatrix) {
    uint32 d1 = twoPlayerZeroSumMatrix.size();
    uint32 d2 = twoPlayerZeroSumMatrix[0].size();

    vector<vector<double>> u;
    for (unsigned int i1 = 0; i1 < d1; i1++) {
        for (unsigned int i2 = 0; i2 < d2; i2++) {
            u.push_back({twoPlayerZeroSumMatrix.at(i1).at(i2),
                          -twoPlayerZeroSumMatrix.at(i1).at(i2)});
        }
    }

    return u;
}

vector<unsigned int> NFGSettings::getIndexingOffsets() const {
    vector<unsigned int> indexingOffsets(numPlayers, 1);

    for (int i = numPlayers - 1; i > 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
            indexingOffsets[j] *= dimensions[i];
        }
    }

    // check of settings integrity
    assert(indexingOffsets[0] * dimensions[0] == utilities.size());

    return indexingOffsets;
}

vector<vector<string>> NFGSettings::getActionNames() const {
    if(!actionNames.empty()) return actionNames;

    vector<vector<string>> allNames;
    allNames.reserve(dimensions.size());
    for (unsigned int d = 0; d < dimensions.size(); ++d) {
        vector<string> names;
        names.reserve(dimensions[d]);
        for (unsigned int i = 0; i < dimensions[d]; ++i) {
            names.emplace_back(to_string(i));
        }
        allNames.emplace_back(names);
    }
    return allNames;
}
bool NFGSettings::isZeroSum() const {
    for (const auto &utility : utilities) {
        double sum = 0;
        for (const double u : utility) sum += u;
        if (sum != 0.0) return false;
    }
    return true;
}

NFGDomain::NFGDomain(NFGSettings settings) :
    Domain(2, settings.numPlayers, settings.isZeroSum(), make_shared<NFGAction>(), make_shared<Observation>()),
    dimensions_(settings.dimensions),
    numPlayers_(settings.numPlayers),
    utilities_(settings.utilities),
    indexingOffsets_(settings.getIndexingOffsets()),
    actionNames_(settings.getActionNames()) {

    auto newState = make_shared<NFGState>(this, vector<uint32>());
    vector<double> rewards(numPlayers_, 0.0);
    vector<shared_ptr<Observation>> privateObs(numPlayers_, noObservation_);

    maxUtility_ = -INFINITY;
    for(const auto &util : utilities_)
        for(const double u : util)
            maxUtility_ = std::max(u, maxUtility_);

    rootStatesDistribution_.emplace_back(OutcomeEntry(Outcome(
        newState, privateObs, noObservation_, rewards)));
}

unsigned long NFGState::countAvailableActionsFor(Player player) const {
    if(isTerminal()) return 0;
    const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
    return nfgDomain->dimensions_[player];
}

vector<shared_ptr<Action>> NFGState::getAvailableActionsFor(const Player player) const {
    const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
    unsigned long numActions = countAvailableActionsFor(player);
    vector<shared_ptr<Action>> actions;
    actions.reserve(numActions);

    for (unsigned long i = 0; i < numActions; i++) {
        actions.push_back(make_shared<NFGAction>(i, nfgDomain->getActionName(i, player)));
    }
    return actions;
}

OutcomeDistribution NFGState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
    vector<ActionId> actionIds;
    for (const shared_ptr<Action> &a : actions) {
        actionIds.push_back(a->getId());
    }

    const auto newState = make_shared<NFGState>(nfgDomain, actionIds);

    unsigned int utilityIndex = 0;
    for (int i = nfgDomain->numPlayers_ - 1; i >= 0; i--) {
        utilityIndex += actionIds[i] * nfgDomain->indexingOffsets_[i];
    }

    vector<shared_ptr<Observation>> privateObservations(
        nfgDomain->numPlayers_, nfgDomain->getNoObservation());

    return {OutcomeEntry(Outcome(
        newState, privateObservations, nfgDomain->getNoObservation(),
        nfgDomain->utilities_[utilityIndex]))};
}

string NFGState::toString() const {
    if (!isTerminal()) return "Initial state\n";

    string ret;
    ret.append("Terminal state\n");
    for (unsigned int i = 0; i < playedActions_.size(); i++) {
        ret.append("Player ");
        ret.append(std::to_string(i + 1));
        ret.append(" played: ");
        ret.append(std::to_string(playedActions_[i]));
        ret.append("\n");
    }
    return ret;
}

bool NFGState::operator==(const GTLib2::State &rhs) const {
    auto nfgState = dynamic_cast<const NFGState &>(rhs);
    return hash_ == nfgState.hash_
        && playedActions_ == nfgState.playedActions_;
}

}  // namespace GTLib2
