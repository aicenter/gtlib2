//
// Created by maco on 6.10.18.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"
#ifndef GTLIB2_CHESSBOARDS_H
#define GTLIB2_CHESSBOARDS_H

#include "kriegspiel.h"
#include "../base/base.h"

namespace GTLib2 {
    namespace domains {
        class KriegspielState;
    }
}

namespace chess {
    class AbstractPiece;
    enum BOARD : int;

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
        boardInfo create(BOARD, GTLib2::domains::KriegspielState*);
    private:
        boardInfo createStandard(GTLib2::domains::KriegspielState*);
        boardInfo createMicrochess(GTLib2::domains::KriegspielState*);
        boardInfo createMinimal(GTLib2::domains::KriegspielState*);
        boardInfo createDemichess(GTLib2::domains::KriegspielState*);
        boardInfo createSilverman4x4(GTLib2::domains::KriegspielState*);
    };

    class FenBoardFactory {
    public:
        boardInfo create(string, GTLib2::domains::KriegspielState*);
        string createFEN(const GTLib2::domains::KriegspielState*);
    private:
        shared_ptr<AbstractPiece> createPiece(char&, int, int, GTLib2::domains::KriegspielState*);
        int pawnid = 1;
    };
}

#endif //GTLIB2_CHESSBOARDS_H
#pragma clang diagnostic pop