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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace GTLib2::domains {

bool NFGAction::operator==(const Action &that) const {
    if (typeid(that) == typeid(*this)) {
        const auto rhsAction = static_cast<const NFGAction *>(&that);
        return id_ == rhsAction->getId();
    }
    return false;
}

vector<vector<double >> NFGSettings::getUtilities(vector<vector<double>> twoPlayerZeroSumMatrix) {
    uint32 d1 = twoPlayerZeroSumMatrix.size();
    uint32 d2 = twoPlayerZeroSumMatrix[0].size();

    vector<vector<double>> ut;

    for (int i1 = 0; i1 < d1; i1++) {
        for (int i2 = 0; i2 < d2; i2++) {
            ut.push_back({twoPlayerZeroSumMatrix[i1][i2], (-1) * twoPlayerZeroSumMatrix[i1][i2]});
        }
    }

    return ut;
}

vector<unsigned int> NFGSettings::getIndexingOffsets() {
    vector<unsigned int> indexingOffsets(this->numPlayers);

    std::fill(indexingOffsets.begin(), indexingOffsets.end(), 1);

    for (int i = numPlayers - 1; i > 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
            indexingOffsets[j] *= dimensions[i];
        }
    }


    // check of settings integrity
    assert(indexingOffsets[0] * dimensions[0] == utilities.size());

    for (vector<double> u : utilities) {
        assert(u.size() == numPlayers);
    }

    return indexingOffsets;
}

NFGDomain::NFGDomain(GTLib2::domains::NFGSettings settings) :
    Domain(2, settings.numPlayers, make_shared<NFGAction>(), make_shared<NFGObservation>()),
    dimensions_(settings.dimensions),
    numPlayers_(settings.numPlayers),
    utilities_(settings.utilities),
    indexingOffsets_(settings.getIndexingOffsets()) {

    auto newState = make_shared<NFGState>(this, false, vector<uint32>());

    auto publicObs = make_shared<NFGObservation>();

    vector<double> rewards(numPlayers_);
    std::fill(rewards.begin(), rewards.end(), 0.0);

    vector<shared_ptr<Observation>> observations(numPlayers_);
    std::fill(observations.begin(), observations.end(), publicObs);

    Outcome outcome(newState, observations, publicObs, rewards);
    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));

}

string NFGDomain::getInfo() const {
    std::stringstream ss;
    ss << (to_string(numPlayers_) + " players normal form game");
    return ss.str();
}

vector<Player> NFGDomain::getPlayers() const {
    vector<Player> ps(numPlayers_);
    std::iota(ps.begin(), ps.end(), 0);
    return ps;
}

unsigned long NFGState::countAvailableActionsFor(Player player) const {
    const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
    return nfgDomain->dimensions_[player];
}

vector<shared_ptr<Action>> NFGState::getAvailableActionsFor(const Player player) const {
    vector<shared_ptr<Action>> actions;
    unsigned long limit = countAvailableActionsFor(player);

    for (unsigned long i = 0; i < limit; i++) {
        actions.push_back(make_shared<NFGAction>(i));
    }
    return actions;
}

OutcomeDistribution
NFGState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
    vector<uint32> actionValues;
    for (shared_ptr<Action> a : actions) {
        actionValues.push_back(dynamic_cast<NFGAction &>(*a).getId());
    }

    OutcomeDistribution newOutcomes;
    const auto newState = make_shared<NFGState>(nfgDomain, true, actionValues);
    shared_ptr<NFGObservation> publicObs = make_shared<NFGObservation>();

    unsigned int utilityIndex = 0;

    for(int i = nfgDomain->numPlayers_ - 1; i >= 0; i--){
        utilityIndex += actionValues[i] * nfgDomain->indexingOffsets_[i];
    }

    vector<shared_ptr<Observation>> observations(nfgDomain->numPlayers_);
    std::fill(observations.begin(), observations.end(), publicObs);

    const auto newOutcome = Outcome(newState,
                                    observations,
                                    publicObs,
                                    nfgDomain->utilities_[utilityIndex]);
    newOutcomes.emplace_back(OutcomeEntry(newOutcome, 1.0));
    return newOutcomes;
}

vector<Player> NFGState::getPlayers() const {
    const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
    return nfgDomain->getPlayers();
}

bool NFGState::isTerminal() const {
    return terminal_;
}

string NFGState::toString() const {
    string ret;
    if (terminal_) {
        ret.append("Terminal state\n");
        for (int i = 0; i < playerActions_.size(); i++) {
            ret.append("Player ");
            ret.append(std::to_string(i + 1));
            ret.append(" played: ");
            ret.append(std::to_string(playerActions_[i]));
            ret.append("\n");
        }
    } else {
        ret.append("Initial state\n");
    }
    return ret;
}

bool NFGState::operator==(const GTLib2::State &rhs) const {
    auto nfgState = dynamic_cast<const NFGState &>(rhs);

    return hash_ == nfgState.hash_
        && terminal_ == nfgState.terminal_
        && playerActions_ == nfgState.playerActions_;
}

}  // namespace GTLib2
#pragma clang diagnostic pop
