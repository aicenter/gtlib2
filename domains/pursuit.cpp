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

#include "domains/pursuit.h"


namespace GTLib2::domains {

// Moore neighborhood for observations
static array<Pos, 10> pursuitEightSurrounding = {{{-2, -2}, {-1, -1}, {0, -1}, {1, -1},
                                                  {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1},
                                                  {0, 0}}};
// moves
static array<Pos, 5> pursuitMoves = {{{0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}}};


PursuitAction::PursuitAction(ActionId id, int move) : Action(id), move_(move) {}

bool PursuitAction::operator==(const Action &that) const {
    if (typeid(*this) == typeid(that)) {
        const auto rhsAction = static_cast<const PursuitAction *>(&that);
        return move_ == rhsAction->move_;
    }
    return false;
}

HashType PursuitAction::getHash() const {
    std::hash<int> h;
    return h(move_);
}

PursuitObservation::PursuitObservation(int id, vector<int> values) :
    Observation(id), values_(move(values)) {}

string PursuitObservation::toString() const {
    string string1;
    if (values_.empty()) {
        return "NoOb;";
    }
    for (unsigned int i = 0; i < values_.size() - 1; ++i) {
        string1 += eightdes_[values_[i]];
        string1 += "; ";
    }
    string1 += "Result: ";
    string1 += values_[values_.size() - 1] == 1 ? "ok" : "fail";
    return string1;
}

PursuitObservationLoc::PursuitObservationLoc(int id, vector<Pos> values) :
    Observation(id), values_(move(values)) {}

string PursuitObservationLoc::toString() const {
    string string1;
    if (values_.empty()) {
        return "NoOb;";
    }
    for (unsigned int i = 0; i < values_.size() - 1; ++i) {
        string1 += to_string(values_[i].x);
        string1 += ", ";
        string1 += to_string(values_[i].y);
        string1 += "; ";
    }
    string1 += "Result: ";
    string1 += values_[values_.size() - 1].y == 1 ? "ok" : "fail";
    return string1;
}

PursuitState::PursuitState(const Domain *domain, const vector<Pos> &place)
    : PursuitState(domain, place, 1) {}

PursuitState::PursuitState(const Domain *domain, const vector<Pos> &place, double prob) :
    State(domain, hashCombine(3215648641314, place, prob)), place_(place), prob_(prob) {
    players_ = vector<Player>(place.size());
    std::iota(players_.begin(), players_.end(), 0);
}

// todo: this is only hotfix
unsigned long PursuitState::countAvailableActionsFor(Player player) const {
    return getAvailableActionsFor(player).size();
}

vector<shared_ptr<Action>> PursuitState::getAvailableActionsFor(Player player) const {
    auto list = vector<shared_ptr<Action>>();
    for (unsigned int i = 1; i < players_.size(); ++i) {
        if (place_[players_[0]].x == place_[players_[i]].x
            && place_[0].y == place_[players_[i]].y) {
            return list;
        }
    }
    const auto purDomain = static_cast<const PursuitDomain *>(domain_);
    int count = 0;
    for (int i = 1; i < 5; ++i) {  // verifies whether moves are correct
        if ((place_[player].x + pursuitMoves[i].x) >= 0 && (place_[player].x + pursuitMoves[i].x)
            < purDomain->width_ &&
            (place_[player].y + pursuitMoves[i].y) >= 0 && (place_[player].y + pursuitMoves[i].y)
            < purDomain->height_) {
            list.push_back(make_shared<PursuitAction>(count, i));
            count++;
        }
    }
    return list;
}

OutcomeDistribution PursuitState::performActions(const vector <shared_ptr<Action>> &actions) const {
    vector<PursuitAction *> pursuitActions(actions.size());
    const auto purDomain = static_cast<const PursuitDomain *>(domain_);
    for (unsigned int i = 0; i < actions.size(); ++i) {
        pursuitActions[i] = dynamic_cast<PursuitAction *>(actions[i].get());
    }
    // number of all combinations
    auto actionssize = static_cast<unsigned int>(pursuitActions.size());
    auto powsize = static_cast<unsigned int>(pursuitActions.size());
    for (auto &i : pursuitActions) {
        if (i->getId() == NO_ACTION)
            powsize--;
    }
    unsigned int count = (1 << powsize);
    OutcomeDistribution prob;
    prob.reserve(static_cast<unsigned long>(count));
//  count = 1;
    for (unsigned int k = 0; k < count; ++k) {
        double probability = prob_;
        vector<shared_ptr<Observation>> observations(place_.size());
        vector<double> rewards(place_.size());
        // making moves and assigning probability
        vector<Pos> moves = place_;
        for (unsigned int i = 0; i < actionssize; ++i) {
            if ((k >> i & 1) == 1) {
                if (pursuitActions[i]->getId() > NO_ACTION) {
                    moves[i].x = pursuitMoves[pursuitActions[i]->GetMove()].x + place_[i].x;
                    moves[i].y = pursuitMoves[pursuitActions[i]->GetMove()].y + place_[i].y;
                }
                probability *= purDomain->probability_[1];
            } else {
                probability *= purDomain->probability_[0];
            }
        }

        shared_ptr<PursuitState> s = make_shared<PursuitState>(domain_, moves, probability);
        auto size = static_cast<unsigned int>(s->place_.size());
        moves.clear();
        // detection if first has caught others
        for (unsigned int i = 1; i < size; ++i) {
            if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
                s->place_[i].x == place_[0].x &&
                s->place_[i].y == place_[0].y) ||
                (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
                ++rewards[0];
                --rewards[i];
            }
        }
        int index;
        int maximum = 0;
        int id;
        vector<int> ob = vector<int>();
        ob.reserve(size + 1);
        for (unsigned int m = 0; m < size; ++m) {  // making observations
            id = 0;
            maximum = 0;
            for (unsigned int i = 0, p = 0; i < size; ++i, ++p) {
                if (m == i) {
                    --p;
                    continue;
                }
                index = 0;
                int id2 = 1;
                for (unsigned int l = 1; l < pursuitEightSurrounding.size(); ++l) {
                    if ((s->place_[m].x + pursuitEightSurrounding[l].x) >= 0
                        && (s->place_[m].x + pursuitEightSurrounding[l].x)
                            < purDomain->width_ &&
                        (s->place_[m].y + pursuitEightSurrounding[l].y) >= 0
                        && (s->place_[m].y + pursuitEightSurrounding[l].y)
                            < purDomain->height_) {
                        id2++;
                    }

                    if (s->place_[m].x + pursuitEightSurrounding[l].x == s->place_[i].x &&
                        s->place_[m].y + pursuitEightSurrounding[l].y == s->place_[i].y) {
                        index = l;
                    }
                }
                maximum += pow(id2, p + 1);
                ob.push_back(index);
                id += index * pow(id2, p);  // counting observation id
            }
            id += (k >> m & 1) * maximum;
            ob.push_back((k >> m & 1));
//      ob.push_back(1);
            observations[m] = make_shared<PursuitObservation>(id, ob);
            ob.clear();
        }

        double p2 = s->prob_ / prob_;
        Outcome o(move(s), observations, shared_ptr<Observation>(), rewards);
        prob.emplace_back(OutcomeEntry(o, p2));
    }
    return prob;
}

bool PursuitState::operator==(const State &rhs) const {
    auto rhsPursuit = dynamic_cast<const PursuitState &>(rhs);

    return hash_ == rhsPursuit.hash_
        && place_ == rhsPursuit.place_
        && players_ == rhsPursuit.players_
        && prob_ == rhsPursuit.prob_;
}

string PursuitState::toString() const {
    string s;
    for (Player player = 0; player < place_.size(); ++player) {
        s += "player: " + to_string(player) + ", location: " +
            to_string(place_[player].x) + " " + to_string(place_[player].y) + "\n";
    }
    return s;
}

MMPursuitState::MMPursuitState(const Domain *domain, const vector<Pos> &p,
                               const vector<Player> &players, vector<int> numberOfMoves) :
    MMPursuitState(domain, p, players, numberOfMoves, numberOfMoves[0], 0) {}

MMPursuitState::MMPursuitState(const Domain *domain,
                               const vector<Pos> &p,
                               const vector<Player> &players,
                               vector<int> numberOfMoves,
                               int currentNOM,
                               int currentPlayer) :
    PursuitState(domain, p), numberOfMoves_(move(numberOfMoves)), players_(players),
    currentNOM_(currentNOM), currentPlayer_(currentPlayer) {}

MMPursuitState::MMPursuitState(const Domain *domain, const vector<Pos> &p, double prob,
                               const vector<Player> &players, vector<int> numberOfMoves) :
    MMPursuitState(domain, p, prob, players, numberOfMoves, numberOfMoves[0], 0) {}

MMPursuitState::MMPursuitState(const Domain *domain, const vector<Pos> &p, double prob,
                               const vector<Player> &players, vector<int> numberOfMoves,
                               int currentNOM, int currentPlayer) :
    PursuitState(domain, p, prob), numberOfMoves_(move(numberOfMoves)), players_(players),
    currentNOM_(currentNOM), currentPlayer_(currentPlayer) {}

OutcomeDistribution MMPursuitState::performActions(const vector <shared_ptr<Action>> &actions) const {
    vector<PursuitAction *> pursuitActions(actions.size());
    const auto purDomain = static_cast<const PursuitDomain *>(domain_);
    for (unsigned int i = 0; i < actions.size(); ++i) {
        pursuitActions[i] = dynamic_cast<PursuitAction *>(actions[i].get());
    }
    // unsigned int count = 2;
    auto actionssize = static_cast<unsigned int>(pursuitActions.size());
    // number of all combinations
    OutcomeDistribution prob;
    prob.reserve(players_.size());
    for (unsigned int j = 0; j < players_.size(); ++j) {
        vector<shared_ptr<Observation>> observations(place_.size());
        vector<double> rewards(place_.size());
        double probability = prob_;
        // making moves and assigning probability
        vector<Pos> moves = place_;
        for (unsigned int i = 0; i < actionssize; ++i) {
            if (pursuitActions[i]->getId() > NO_ACTION) {
                moves[i].x = pursuitMoves[pursuitActions[i]->GetMove()].x + place_[i].x;
                moves[i].y = pursuitMoves[pursuitActions[i]->GetMove()].y + place_[i].y;
            }
        }
        shared_ptr<MMPursuitState> s;
        if (currentNOM_ == 1) {
            int newPlayer = (currentPlayer_ + 1) % players_.size();
            s = make_shared<MMPursuitState>(domain_, moves, probability, players_, numberOfMoves_,
                                            numberOfMoves_[newPlayer], newPlayer);
        } else {
            s = make_shared<MMPursuitState>(domain_,
                                            moves,
                                            probability,
                                            players_,
                                            numberOfMoves_,
                                            currentNOM_ - 1,
                                            currentPlayer_);
        }
        auto size = static_cast<unsigned int>(s->place_.size());
        moves.clear();
        // detection if first has caught the others
        for (unsigned int i = 1; i < size; ++i) {
            if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
                s->place_[i].x == place_[0].x &&
                s->place_[i].y == place_[0].y) ||
                (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
                ++rewards[0];
                --rewards[i];
            }
        }
        int index;
        int id;
        vector<int> ob = vector<int>();
        ob.reserve(size + 1);
        for (unsigned int m = 0, pom = 0; m < size; ++m, ++pom) {  // making observations
            if (pursuitActions[m]->getId() == NO_ACTION && currentNOM_ > 1) {
                observations[m] = make_shared<Observation>(NO_OBSERVATION);
                --pom;
                continue;
            }

            id = 0;
            for (unsigned int i = 0, p = 0; i < size; ++i, ++p) {
                if (m == i) {
                    --p;
                    continue;
                }
                index = 0;
                int id2 = 1;
                for (unsigned int l = 1; l < pursuitEightSurrounding.size(); ++l) {
                    if ((s->place_[m].x + pursuitEightSurrounding[l].x) >= 0
                        && (s->place_[m].x + pursuitEightSurrounding[l].x)
                            < purDomain->width_ &&
                        (s->place_[m].y + pursuitEightSurrounding[l].y) >= 0
                        && (s->place_[m].y + pursuitEightSurrounding[l].y)
                            < purDomain->height_) {
                        id2++;
                    }

                    if (s->place_[m].x + pursuitEightSurrounding[l].x == s->place_[i].x &&
                        s->place_[m].y + pursuitEightSurrounding[l].y == s->place_[i].y) {
                        index = l;
                    }
                }
                ob.push_back(index);
                id += index * pow(id2, p);  // counting observation id
            }
            ob.push_back(1);
            observations[m] = make_shared<PursuitObservation>(id, ob);
            ob.clear();
        }
        double p2 = s->prob_ / prob_;
        Outcome p(move(s), observations, shared_ptr<Observation>(), rewards);
        prob.emplace_back(OutcomeEntry(p, p2));
    }
    return prob;
}

bool MMPursuitState::operator==(const State &rhs) const {
    auto rhsState = dynamic_cast<const MMPursuitState &>(rhs);


    return place_ == rhsState.place_
        && players_ == rhsState.players_
        && prob_ == rhsState.prob_
        && numberOfMoves_ == rhsState.numberOfMoves_
        && currentNOM_ == rhsState.currentNOM_
        && currentPlayer_ == rhsState.currentPlayer_;
}

ObsPursuitState::ObsPursuitState(const Domain *domain, const vector<Pos> &p) :
    PursuitState(domain, p) {}

ObsPursuitState::ObsPursuitState(const Domain *domain, const vector<Pos> &p,
                                 double prob) : PursuitState(domain, p, prob) {}

OutcomeDistribution ObsPursuitState::performActions(const vector <shared_ptr<Action>> &actions) const {
    vector<PursuitAction *> pursuitActions(actions.size());
    const auto purDomain = static_cast<const PursuitDomain *>(domain_);
    for (unsigned int i = 0; i < actions.size(); ++i) {
        pursuitActions[i] = dynamic_cast<PursuitAction *>(actions[i].get());
    }
    // number of all combinations
    auto actionssize = static_cast<unsigned int>(pursuitActions.size());
    auto powsize = static_cast<unsigned int>(pursuitActions.size());
    for (auto &i : pursuitActions) {
        if (i->getId() == NO_ACTION)
            powsize--;
    }
    int count = 1 << powsize;
    OutcomeDistribution outcomes;
    outcomes.reserve(static_cast<size_t>(count));
    for (int k = 0; k < count; ++k) {
        vector<shared_ptr<Observation>> observations(place_.size());
        vector<double> rewards(place_.size());
        double probability = prob_;
        // making moves and assigning probability
        vector<Pos> moves = place_;
        for (unsigned int i = 0; i < actionssize; ++i) {
            if (((k >> i) & 1) == 1) {
                if (pursuitActions[i]->getId() > NO_ACTION) {
                    moves[i].x = pursuitMoves[pursuitActions[i]->GetMove()].x + place_[i].x;
                    moves[i].y = pursuitMoves[pursuitActions[i]->GetMove()].y + place_[i].y;
                }
                probability *= purDomain->probability_[1];
            } else {
                probability *= purDomain->probability_[0];
            }
        }

        shared_ptr<ObsPursuitState> s =
            make_shared<ObsPursuitState>(domain_, moves, probability);
        auto size = static_cast<unsigned int>(s->place_.size());
        moves.clear();
        // detection if first has caught others
        for (unsigned int i = 1; i < size; ++i) {
            if ((s->place_[0].x == place_[i].x && s->place_[0].y == place_[i].y &&
                s->place_[i].x == place_[0].x &&
                s->place_[i].y == place_[0].y) ||
                (s->place_[0].x == s->place_[i].x && s->place_[0].y == s->place_[i].y)) {
                ++rewards[0];
                --rewards[i];
            }
        }
        int id;
        vector<Pos> ob = vector<Pos>();
        ob.reserve(size + 1);
        for (unsigned int m = 0; m < size; ++m) {  // making observations
            id = 0;
            for (unsigned int i = 0, p = 0; i < size; ++i, ++p) {
                if (m == i) {
                    --p;
                    continue;
                }
                int id2 = purDomain->width_ * purDomain->height_;

                ob.push_back({s->place_[i].y, s->place_[i].x});
                // counting observation id
                id += (s->place_[i].x + s->place_[i].y * purDomain->width_) * pow(id2, p);
            }
            ob.push_back({((k >> m) & 1), -1});
            id += ((k >> m) & 1) * ob.size();
            observations[m] = make_shared<PursuitObservationLoc>(id, ob);
            ob.clear();
        }
        double p2 = s->prob_ / prob_;
        outcomes.emplace_back(OutcomeEntry(
            Outcome(move(s), observations, shared_ptr<Observation>(), rewards), p2));
    }
    return outcomes;
}

PursuitDomain::PursuitDomain(unsigned int max,
                             unsigned int numberOfPlayers,
                             const vector<Pos> &loc,
                             int height,
                             int width,
                             vector<double> probability) :
    Domain(max, numberOfPlayers, true, make_shared<PursuitAction>(), make_shared<PursuitObservation>()),
    probability_(move(probability)), height_(height), width_(width) {
    auto state = make_shared<PursuitState>(this, loc);
    vector<double> rewards(numberOfPlayers);
    vector<shared_ptr<Observation>> Obs;
    for (unsigned int j = 0; j < numberOfPlayers; ++j) {
        Obs.push_back(make_shared<Observation>(NO_OBSERVATION));
    }
    rootStatesDistribution_.push_back(OutcomeEntry(
        Outcome(state, Obs, shared_ptr<Observation>(), rewards)));
}

string PursuitDomain::getInfo() const {
    return "Pursuit evasion\nDimensions: " + to_string(height_) + " x " +
        to_string(width_) + "\nMax depth: " +
        to_string(maxStateDepth_) + "\nPlayers' starting location: " +
        rootStatesDistribution_[0].outcome.state->toString();
}

PursuitDomain::PursuitDomain(unsigned int max, int height, int width) :
    PursuitDomain(max, 2, vector<Pos>{{0, 0}, {height - 1, width - 1}},
                  height, width, vector<double>{0.1, 0.9}) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<MMPursuitState> &state,
                             int height, int width, vector<double> probability) :
    Domain(max, numberOfPlayers, true, make_shared<PursuitAction>(), make_shared<PursuitObservation>()),
    probability_(move(probability)), height_(height), width_(width) {
    vector<double> rewards(numberOfPlayers);
    vector<shared_ptr<Observation>> Obs;
    for (unsigned int j = 0; j < numberOfPlayers; ++j) {
        Obs.push_back(make_shared<Observation>(NO_OBSERVATION));
    }

    rootStatesDistribution_.push_back(OutcomeEntry(
        Outcome(state, Obs, shared_ptr<Observation>(), rewards)));
}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<ObsPursuitState> &state,
                             int height, int width, vector<double> probability) :
    Domain(max, numberOfPlayers, true, make_shared<PursuitAction>(), make_shared<PursuitObservation>()),
    probability_(move(probability)), height_(height), width_(width) {
    vector<double> rewards(numberOfPlayers);
    vector<shared_ptr<Observation>> Obs;
    for (unsigned int j = 0; j < numberOfPlayers; ++j) {
        Obs.push_back(make_shared<Observation>(NO_OBSERVATION));
    }
    rootStatesDistribution_.push_back(OutcomeEntry(
        Outcome(state, Obs, shared_ptr<Observation>(), rewards)));
}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers, const vector<Pos> &loc,
                             int height, int width) :
    PursuitDomain(max, numberOfPlayers, loc, height, width, vector<double>{0.1, 0.9}) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<MMPursuitState> &state, int height, int width) :
    PursuitDomain(max, numberOfPlayers, state, height, width, vector<double>{0.1, 0.9}) {}

PursuitDomain::PursuitDomain(unsigned int max, unsigned int numberOfPlayers,
                             const shared_ptr<ObsPursuitState> &state, int height, int width) :
    PursuitDomain(max, numberOfPlayers, state, height, width, vector<double>{0.1, 0.9}) {}

PursuitDomainChance::PursuitDomainChance(unsigned int max,
                                         unsigned int numberOfPlayers,
                                         const vector<Pos> &firstPlayerLocation,
                                         const vector<Pos> &secondPlayerLocation,
                                         int height, int width, vector<double> probability) :
    PursuitDomain(max, numberOfPlayers, {firstPlayerLocation[0], secondPlayerLocation[0]},
                  height, width, move(probability)) {
    rootStatesDistribution_.clear();
    for (unsigned int i = 0; i < firstPlayerLocation.size(); ++i) {
        auto state = make_shared<PursuitState>(this, vector<Pos>{firstPlayerLocation[i],
                                                                 secondPlayerLocation[i]});
        vector<double> rewards(numberOfPlayers);
        vector<shared_ptr<Observation>> Obs;
        for (unsigned int j = 0; j < numberOfPlayers; ++j) {
            Obs.push_back(make_shared<Observation>(NO_OBSERVATION));
        }

        rootStatesDistribution_.push_back(OutcomeEntry(
            Outcome(state, Obs, shared_ptr<Observation>(), rewards),
            1.0 / firstPlayerLocation.size()
        ));
    }
}

PursuitDomainChance::PursuitDomainChance(unsigned int max, unsigned int numberOfPlayers,
                                         const vector<Pos> &firstPlayerLocation,
                                         const vector<Pos> &secondPlayerLocation,
                                         int height, int width) :
    PursuitDomainChance(max, numberOfPlayers,
                        firstPlayerLocation,
                        secondPlayerLocation,
                        height, width,
                        vector<double>{0.1, 0.9}) {}

}  // namespace GTLib2
