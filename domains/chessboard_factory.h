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


#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef GTLIB2_CHESSBOARDS_H
#define GTLIB2_CHESSBOARDS_H

#include "kriegspiel.h"
#include "../base/base.h"

namespace GTLib2::domains {
class KriegspielState;
}

namespace GTLib2::domains::chess {
class AbstractPiece;
enum BOARD: int;

struct boardInfo {
    int x;
    int y;
    bool castle;
    int xEnpass;
    int yEnpass;
    shared_ptr<vector<shared_ptr<AbstractPiece>>> pieces;
};

class BoardFactory {
 public:
    BoardFactory() = default;
    boardInfo create(BOARD, GTLib2::domains::KriegspielState *);
 private:
    boardInfo createStandard(GTLib2::domains::KriegspielState *);
    boardInfo createMicrochess(GTLib2::domains::KriegspielState *);
    boardInfo createMinimal(GTLib2::domains::KriegspielState *);
    boardInfo createDemichess(GTLib2::domains::KriegspielState *);
    boardInfo createSilverman4x4(GTLib2::domains::KriegspielState *);
};

class FenBoardFactory {
 public:
    boardInfo create(string, GTLib2::domains::KriegspielState *);
    string createFEN(const GTLib2::domains::KriegspielState *);
 private:
    shared_ptr<AbstractPiece> createPiece(char &, int, int, GTLib2::domains::KriegspielState *);
    int pawnid_ = 1;
};
}

#endif //GTLIB2_CHESSBOARDS_H
#pragma clang diagnostic pop
