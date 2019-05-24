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
        return actionIndex_ == rhsAction->actionIndex_;
    }
    return false;
}

vector<double> NFGSettings::getUtilities() {
    if(inputVariant == Vector){
        return utilities;
    }

    // two players symmetrical matrix input

    assert(dimensions[0] == dimensions[1]);

    uint32 d = dimensions[0];
    uint32 size  = numPlayers * d^2;

    vector<double> ut(size);

    // player 1
    for (int i1 = 0; i1 < d; i1++) {
        for (int i2 = 0; i2 < d; i2++) {
            ut.push_back(utilityMatrix[i1][i2]);
        }
    }

    // player 2
    for (int i1 = 0; i1 < d; i1++) {
        for (int i2 = 0; i2 < d; i2++) {
            ut.push_back(utilityMatrix[i2][i1]);
        }
    }

    return ut;
}


NFGDomain::NFGDomain(GTLib2::domains::NFGSettings settings) :
    Domain(2, settings.numPlayers, make_shared<NFGAction>(), make_shared<NFGObservation>()),
    dimensions_(settings.dimensions),
    numPlayers_(settings.numPlayers),
    utilities_(settings.getUtilities()){


    auto newState = make_shared<NFGState>(this, false, {});

    auto publicObs = make_shared<NFGObservation>();

    Outcome outcome(newState, {publicObs, publicObs}, publicObs, {0.0, 0.0});
    rootStatesDistribution_.emplace_back(OutcomeEntry(outcome));

}


string NFGDomain::getInfo() const {
    std::stringstream ss;
    ss << ( to_string(numPlayers_) + " players normal form game");
    return ss.str();
}

vector<Player> NFGDomain::getPlayers() {
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
        actions.push_back(make_shared<NFGAction>(i, i));
    }
    return actions;
}

OutcomeDistribution
NFGState::performActions(const vector<shared_ptr<Action>> &actions) const {
    const auto nfgDomain = static_cast<const NFGDomain *>(domain_);
    vector<uint32> actionValues;
    for(shared_ptr<Action> a : actions){
        actionValues.push_back(dynamic_cast<NFGAction &>(*a).actionIndex_);
    }

    OutcomeDistribution newOutcomes;
    const auto newState = make_shared<NFGState>(nfgDomain, true, actionValues);
    shared_ptr<NFGObservation> publicObs = make_shared<NFGObservation>();

    auto get_reward_for_player = [&](unsigned i){

    };

    auto extractRewards = [&](){
        vector<double> rewards;
        for(unsigned i = 0; i < nfgDomain->numPlayers_; i++){
            rewards.push_back(get_reward_for_player(i));
        }
    };

    const auto newOutcome = Outcome(newState, {publicObs, publicObs}, publicObs, extractRewards());
    newOutcomes.emplace_back(OutcomeEntry(newOutcome, 1.0));
    return newOutcomes;
}


}  // namespace GTLib2
#pragma clang diagnostic pop
