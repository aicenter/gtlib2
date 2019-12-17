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

#include "domains/phantomTTT.h"

#ifdef DString
#  define D(x) x
#else
#  define DebugString(x) x
#endif  // MyDEBUG

namespace GTLib2::domains {

PhantomTTTAction::PhantomTTTAction(ActionId id, int move) : Action(id), move_(move) {}

bool PhantomTTTAction::operator==(const Action &that) const {
    if (typeid(*this) == typeid(that)) {
        const auto rhsAction = static_cast<const PhantomTTTAction *>(&that);
        return move_ == rhsAction->move_;
    }
    return false;
}

HashType PhantomTTTAction::getHash() const {
    std::hash<int> h;
    return h(move_);
}

unsigned long PhantomTTTState::countAvailableActionsFor(Player player) const {
    int count = 0;
    for (int i = 0; i < 9; ++i)
        if (board_[player][i] == 0) ++count;
    return count;
}

vector<shared_ptr<Action>> PhantomTTTState::getAvailableActionsFor(Player player) const {
    auto list = vector<shared_ptr<Action>>();
    int count = 0;
    for (int i = 0; i < 9; ++i) {
        if (board_[player][i] == 0) {
            list.push_back(make_shared<PhantomTTTAction>(count, i));
            ++count;
        }
    }
    return list;
}

OutcomeDistribution PhantomTTTState::performActions
    (const vector <shared_ptr<Action>> &actions) const {
    auto a1 = dynamic_cast<PhantomTTTAction *>(actions[0].get());
    auto a2 = dynamic_cast<PhantomTTTAction *>(actions[1].get());
    vector<shared_ptr<Observation>> observations(2);
    vector<double> rewards(2);
    int success = 0;
    vector<Player> pla2;
    // observations.reserve(2);
    vector<vector<int>> moves = board_;
    if (a1) {
        if (moves[1][a1->GetMove()] == 0) {
            moves[0][a1->GetMove()] = 1;
            success = 1;
            pla2.emplace_back(1);
        } else {
            moves[0][a1->GetMove()] = 2;
            pla2.emplace_back(0);
        }
        observations[0] = make_shared<PhantomTTTObservation>(success);
        observations[1] = make_shared<Observation>(NO_OBSERVATION);
    } else {
        if (moves[0][a2->GetMove()] == 0) {
            moves[1][a2->GetMove()] = 1;
            success = 1;
            pla2.emplace_back(0);
        } else {
            moves[1][a2->GetMove()] = 2;
            pla2.emplace_back(1);
        }
        observations[0] = make_shared<Observation>(NO_OBSERVATION);
        observations[1] = make_shared<PhantomTTTObservation>(success);
    }
    auto board = moves[0];
    for (int i = 0; i < 9; ++i) {
        if (moves[1][i] == 1) {
            board[i] = 2;
        }
    }
    shared_ptr<PhantomTTTState> s;

    if ((board[0] == board[1] && board[1] == board[2])
        || (board[0] == board[3] && board[3] == board[6])
        || (board[0] == board[4] && board[4] == board[8])) {
        if (board[0] == 1) {
            rewards[0] = +1;
            rewards[1] = -1;
        } else if (board[0] == 2) {
            rewards[0] = -1;
            rewards[1] = +1;
        }
    }
    if ((board[8] == board[5] && board[5] == board[2])
        || (board[8] == board[7] && board[7] == board[6])) {
        if (board[8] == 1) {
            rewards[0] = +1;
            rewards[1] = -1;
        } else if (board[8] == 2) {
            rewards[0] = -1;
            rewards[1] = +1;
        }
    }
    if ((board[4] == board[1] && board[4] == board[7])
        || (board[4] == board[3] && board[4] == board[5])) {
        if (board[4] == 1) {
            rewards[0] = +1;
            rewards[1] = -1;
        } else if (board[4] == 2) {
            rewards[0] = -1;
            rewards[1] = +1;
        }
    }
    if ((board[2] == board[4] && board[4] == board[6])) {
        if (board[2] == 1) {
            rewards[0] = +1;
            rewards[1] = -1;
        } else if (board[2] == 2) {
            rewards[0] = -1;
            rewards[1] = +1;
        }
    }
    if (rewards[0] != 0) {
        for (int j = 0; j < 9; ++j) {
            if (moves[0][j] == 0) {
                moves[0][j] = -1;
            }
            if (moves[1][j] == 0) {
                moves[1][j] = -1;
            }
        }
        s = make_shared<PhantomTTTState>(domain_, moves, pla2);
    } else {
        s = make_shared<PhantomTTTState>(domain_, moves, pla2);
    }
    Outcome o(move(s), observations, shared_ptr<Observation>(), rewards);
    OutcomeDistribution prob;
    prob.push_back(Outcome(o));
    return prob;
}

bool PhantomTTTState::operator==(const State &rhs) const {
    auto ptttRhs = dynamic_cast<const PhantomTTTState &>(rhs);

    return hash_ == ptttRhs.hash_
        && board_ == ptttRhs.board_
        && players_ == ptttRhs.players_;
}

string PhantomTTTState::toString() const {
    string s;
    for (Player player = 0; player < board_.size(); ++player) {
        s += "Player: " + to_string(player) + " board:\n";
        for (int i = 0; i < 9; ++i) {
            switch (board_[player][i]) {
                case 0:
                    s += "_ ";
                    break;
                case 1:
                    s += "x ";
                    break;
                case 2:
                    s += "o ";
                    break;
                default:
                    s += "- ";
                    break;
            }
            if (i == 2 || i == 5 || i == 8) {
                s += "\n";
            }
        }
    }
    return s;
}

PhantomTTTDomain::PhantomTTTDomain(unsigned int max) :
    Domain(max, 2, true, make_shared<PhantomTTTAction>(),
           make_shared<PhantomTTTObservation>()) {
    auto vec = vector<vector<int>>{{0, 0, 0, 0, 0, 0, 0, 0, 0},
                                   {0, 0, 0, 0, 0, 0, 0, 0, 0}};
    auto players = vector<Player>({0});
    vector<double> rewards(2);
    vector<shared_ptr<Observation>> privateObs{
        make_shared<Observation>(NO_OBSERVATION),
        make_shared<Observation>(NO_OBSERVATION)
    };

    rootStatesDistribution_.push_back(OutcomeEntry(
        Outcome(make_shared<PhantomTTTState>(this, vec, players), privateObs,
                shared_ptr<Observation>(), rewards)));
}

string PhantomTTTDomain::getInfo() const {
    return "************ Phantom Tic Tac Toe *************\n" +
        rootStatesDistribution_[0].outcome.state->toString() + "\n";
}
}  // namespace GTLib2
