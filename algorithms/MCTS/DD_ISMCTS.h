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

#ifndef GTLIB2_ALGORITHMS_MCTS_DD_ISMCTS_H_
#define GTLIB2_ALGORITHMS_MCTS_DD_ISMCTS_H_
#include "CPW_ISMCTS.h"
#include "domains/stratego.h"
namespace GTLib2::algorithms {
class DD_ISMCTS : public CPW_ISMCTS {
 public:
    explicit DD_ISMCTS(const domains::StrategoDomain &domain, Player playingPlayer, ISMCTSSettings config) :
        CPW_ISMCTS(domain, playingPlayer, std::move(config)) {};

    PlayControl runPlayIteration(const optional<shared_ptr<AOH>> &currentInfoset) override;

 private:
    bool isCurrentISUndiscovered = false;
};
}
#endif //GTLIB2_ALGORITHMS_MCTS_DD_ISMCTS_H_
