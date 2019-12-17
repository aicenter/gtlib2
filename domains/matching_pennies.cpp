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


#include "domains/matching_pennies.h"


namespace GTLib2::domains {
MatchingPenniesDomain::MatchingPenniesDomain(MatchingPenniesVariant variant)
    : Domain((variant == AlternatingMoves ? 2 : 1) + 1, 2, true,
             make_shared<MatchingPenniesAction>(),
             make_shared<MatchingPenniesObservation>()), variant_(variant) {
    maxUtility_ = 1.0;
    auto rootState =
        make_shared<MatchingPenniesState>(this, array<ActionId, 2>{NO_ACTION, NO_ACTION});

    auto obs0 = make_shared<MatchingPenniesObservation>(NO_OBSERVATION);
    auto obs1 = make_shared<MatchingPenniesObservation>(NO_OBSERVATION);
    auto pubObs = make_shared<MatchingPenniesObservation>(NO_OBSERVATION);
    vector<double> rewards(2, 0.);

    Outcome outcome(rootState, {obs0, obs1}, pubObs, rewards);
    rootStatesDistribution_.push_back(OutcomeEntry(outcome));
}

const vector<Player> MatchingPenniesState::makePlayers(array<ActionId, 2> moves,
                                                       MatchingPenniesVariant variant) {
    vector<Player> players;
    if (variant == SimultaneousMoves) {
        assert((moves[0] != NO_ACTION && moves[1] != NO_ACTION)
                   || (moves[0] == NO_ACTION && moves[1] == NO_ACTION));
        if (moves[0] == NO_ACTION) players.push_back(Player(0));
        if (moves[1] == NO_ACTION) players.push_back(Player(1));
    } else {
        if (moves[0] == NO_ACTION && moves[1] == NO_ACTION) players.push_back(Player(0));
        if (moves[1] == NO_ACTION && moves[0] != NO_ACTION) players.push_back(Player(1));
    }
    return players;
}

unsigned long MatchingPenniesState::countAvailableActionsFor(Player pl) const {
    //@formatter:off
    return (variant_ == SimultaneousMoves && moves_[0] == NO_ACTION && moves_[1] == NO_ACTION)
        || (variant_ == AlternatingMoves && pl == 0 && moves_[0] == NO_ACTION && moves_[1] == NO_ACTION)
        || (variant_ == AlternatingMoves && pl == 1 && moves_[0] != NO_ACTION && moves_[1] == NO_ACTION)
        ? 2 : 0;
    //@formatter:on
}

vector<shared_ptr<Action>> MatchingPenniesState::getAvailableActionsFor(Player pl) const {
    //@formatter:off
    vector<shared_ptr<Action>> actions = vector<shared_ptr<Action>>();
    if ((variant_ == SimultaneousMoves && moves_[0] == NO_ACTION && moves_[1] == NO_ACTION)
     || (variant_ == AlternatingMoves && pl == 0 && moves_[0] == NO_ACTION && moves_[1] == NO_ACTION)
     || (variant_ == AlternatingMoves && pl == 1 && moves_[0] != NO_ACTION && moves_[1] == NO_ACTION)) {
        actions.push_back(make_shared<MatchingPenniesAction>(ActionHeads));
        actions.push_back(make_shared<MatchingPenniesAction>(ActionTails));
    }
    return actions;
    //@formatter:on
}

OutcomeDistribution
MatchingPenniesState::performActions(const vector <shared_ptr<Action>> &actions) const {
    auto p0Action = dynamic_cast<MatchingPenniesAction *>(actions[0].get());
    auto p1Action = dynamic_cast<MatchingPenniesAction *>(actions[1].get());

    if (variant_ == SimultaneousMoves) {
        // Both actions must be performed
        assert(p0Action->getId() != NO_ACTION && p1Action->getId() != NO_ACTION);
    } else {
        // Only one action can be performed
        assert(p0Action->getId() == NO_ACTION || p1Action->getId() == NO_ACTION);
        // pl0 played -> pl1 has to play.
        assert(moves_[0] == NO_ACTION || p1Action->getId() != NO_ACTION);
    }

    auto newState = make_shared<MatchingPenniesState>(
        domain_, array<ActionId, 2>{
            p0Action->getId() == NO_ACTION ? moves_[0] : p0Action->getId(),
            p1Action->getId()
        });

    const bool finalState = newState->moves_[0] != NO_ACTION && newState->moves_[1] != NO_ACTION;
    auto obs0 = make_shared<MatchingPenniesObservation>(
        finalState ? (newState->moves_[1] == ActionHeads ? OtherHeads : OtherTails)
                   : NO_OBSERVATION);
    auto obs1 = make_shared<MatchingPenniesObservation>(
        finalState ? (newState->moves_[0] == ActionHeads ? OtherHeads : OtherTails)
                   : NO_OBSERVATION);

    shared_ptr<MatchingPenniesObservation> pubObs;
    vector<double> rewards(2, 0.);
    if (newState->moves_[0] == NO_ACTION || newState->moves_[1] == NO_ACTION) {
        pubObs = make_shared<MatchingPenniesObservation>(NO_OBSERVATION);
    } else if (newState->moves_[0] == newState->moves_[1]) {
        rewards[0] = 1;
        rewards[1] = -1;
        pubObs = make_shared<MatchingPenniesObservation>(Pl0Wins);
    } else {
        rewards[0] = -1;
        rewards[1] = 1;
        pubObs = make_shared<MatchingPenniesObservation>(Pl1Wins);
    }

    Outcome outcome(newState, {obs0, obs1}, pubObs, rewards);
    OutcomeDistribution distr;
    distr.push_back(OutcomeEntry(outcome));
    return distr;
}

string MatchingPenniesState::toString() const {
    return "Pl1: " + to_string(moves_[0]) + "\nPl2: " + to_string(moves_[1]);
}

bool MatchingPenniesState::operator==(const State &rhs) const {
    auto mpState = dynamic_cast<const MatchingPenniesState &>(rhs);
    return hash_ == mpState.hash_
        && moves_ == mpState.moves_
        && players_ == mpState.players_;
}

string MatchingPenniesAction::toString() const {
    switch (id_) {
        case ActionHeads:
            return "Heads";
        case ActionTails:
            return "Tails";
        default:
            return "Nothing";
    }
}

}  // namespace GTLib2
