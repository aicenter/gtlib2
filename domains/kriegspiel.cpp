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


#include "kriegspiel.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "TemplateArgumentsIssues"

namespace chess {
    char coords[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    int bishopMoves[4][2] = {{-1, 1}, {-1, -1}, {1, -1}, {1, 1}};
    int rookMoves[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    int queenKingMoves[8][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}, {-1, 1}, {-1, -1}, {1, -1}, {1, 1}};
    int knightMoves[8][2] = {{1, 2}, {-1, 2}, {-1, -2}, {1, -2}, {2, 1}, {-2, 1}, {-2, -1}, {2, -1}};

    const string coordToString(const Square c) {
        return coords[c.x-1] + std::to_string(c.y);
    }

    const Square stringToCoord(string s) {
        int x = s[0] - 96;
        int y = s[1] - 48;
        return Square(x, y);
    }

    AbstractPiece::AbstractPiece(chess::pieceName k, int c, Square pos, const GTLib2::domains::KriegspielState *s) : position(pos), kind(k), color(c), board(s) {}

    Pawn::Pawn(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, int id): AbstractPiece(k, c, p, b), id(id) {}
    Rook::Rook(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Queen::Queen(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Knight::Knight(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    Bishop::Bishop(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    King::King(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b): AbstractPiece(k, c, p, b) {}
    King::King(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, bool mo): AbstractPiece(k, c, p, b) {
        moved = mo;
    }
    Rook::Rook(chess::pieceName k, int c, chess::Square p, const GTLib2::domains::KriegspielState * b, bool mo): AbstractPiece(k, c, p, b) {
        moved = mo;
    }

    shared_ptr<AbstractPiece> Queen::clone() const {
        return make_shared<Queen>(kind, color, position, board);
    }

    shared_ptr<AbstractPiece> Pawn::clone() const {
        return make_shared<Pawn>(kind, color, position, board, id);
    }

    shared_ptr<AbstractPiece> Knight::clone() const {
        return make_shared<Knight>(kind, color, position, board);
    }

    shared_ptr<AbstractPiece> King::clone() const {
        return make_shared<King>(kind, color, position, board, moved);
    }

    shared_ptr<AbstractPiece> Rook::clone() const {
        return make_shared<Rook>(kind, color, position, board, moved);
    }

    shared_ptr<AbstractPiece> Bishop::clone() const {
        return make_shared<Bishop>(kind, color, position, board);
    }

    int invertColor(int c) {
        return c == WHITE ? BLACK : WHITE;
    }

    void AbstractPiece::move(Square pos) {
        position = pos;
        moved = true;
    }

    bool AbstractPiece::hasMoved() const {
        if(kind != PAWN) {
            return moved;
        } else {
            if(getColor() == WHITE) {
                return position.y == 2;
            } else {
                return position.y == board->getYSize() - 2;
            }
        }
    }

    AbstractPiece* AbstractPiece::getPinnedBy() const {
        return pinnedBy;
    }

    AbstractPiece* AbstractPiece::getProtectedBy() const {
        return protectedBy;
    }

    void AbstractPiece::setPinnedBy(AbstractPiece* p) {
        pinnedBy = p;
    }

    void AbstractPiece::setProtectedBy(AbstractPiece* p) {
        protectedBy = p;
    }

    vector<Square>* AbstractPiece::getAllMoves() const {
        return moves.get();
    }

    vector<Square>* AbstractPiece::getAllValidMoves() const {
        return validMoves.get();
    }

    void AbstractPiece::updateValidMovesWhilePinned() {
        AbstractPiece* pinner = getPinnedBy();
        validMoves->clear();
        int dx = position.x;
        int dy = position.y;
        Square pinnerPosition = pinner->getPosition();

        int incrementX = dx == pinnerPosition.x ? 0 : dx < pinnerPosition.x ? 1 : -1;
        int incrementY = dy == pinnerPosition.y ? 0 : dy < pinnerPosition.y ? 1 : -1;

        dx += incrementX;
        dy += incrementY;

        for(; !(dx == pinnerPosition.x && dy == pinnerPosition.y); dx += incrementX, dy += incrementY) {
            Square newPos(dx, dy);
            validMoves->push_back(newPos);
        }

        validMoves->push_back(pinnerPosition);
    }

    void AbstractPiece::update() {
        updateMoves();
        if(board->isPlayerInCheck() == getColor()) {
            if(getPinnedBy() != nullptr) {
                //pinned piece cant block check
                validMoves->clear();
                return;
            }
            updateValidMovesWhileInCheck();
        } else updateValidMovesPinsProtects(getPinnedBy() != nullptr);
    }

    void AbstractPiece::updateValidMovesWhileInCheck() {
        if(kind == KING) {
            for(int* i: queenKingMoves) {
                Square newPos(position.x+i[0], position.y+i[1]);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if(board->coordOutOfBounds(newPos) || board->isSquareUnderAttack(invertColor(color), newPos) ||
                        (p != nullptr && p->getColor() == getColor()) || (p != nullptr && p->getProtectedBy() != nullptr)) {
                    continue;
                }
                validMoves->push_back(newPos);

            }
            return;
        }
        if(board->getCheckingFigures().size() == 2) {
            validMoves->clear();
            return;
        }
        shared_ptr<AbstractPiece> king = board->getPiecesOfColorAndKind(getColor(), KING)[0];
        shared_ptr<AbstractPiece> checker = board->getCheckingFigures().at(0);
        if(checker->getKind() != KNIGHT) {
            vector<Square> blockingSquares = board->getSquaresBetween(king.get(), checker.get());
            for(Square s: blockingSquares) {
                if(std::find(moves->begin(), moves->end(), s) != moves->end()) {
                    validMoves->push_back(s);
                }
            }
        }
        if(std::find(moves->begin(), moves->end(), checker->getPosition()) != moves->end()) {
            validMoves->push_back(checker->getPosition());
        }

    }

    void AbstractPiece::reset() {
        pinnedBy = nullptr;
        protectedBy = nullptr;
        moves->clear();
        validMoves->clear();
    }

    string AbstractPiece::toString() const {
        return std::string(1 , color == chess::WHITE ? toupper(kind) : tolower(kind));
    }

    bool AbstractPiece::operator==(const chess::AbstractPiece &that) const {
        return getPosition() == that.getPosition() && getKind() == that.getKind() && getColor() == that.getColor();
    }

    int AbstractPiece::getColor() const {
        return color;
    }

    Square AbstractPiece::getPosition() const {
        return position;
    }

    void AbstractPiece::setHasMoved(bool h) {
        moved = h;
    }

    bool AbstractPiece::equal_to(const AbstractPiece & that) const {
        return getColor() == that.getColor() && getPosition() == that.getPosition() && getKind() == that.getKind();
    }

    void Pawn::updateMoves() {
        //cut moves
        int ycut = color == WHITE ? position.y + 1 : position.y - 1;
        Square cutpos(position.x-1, ycut);
        if(board->getPieceOnCoords(cutpos) != nullptr) {
            moves->push_back(cutpos);
        }

        if(board->getEnPassantSquare() == cutpos) {
            moves->push_back(cutpos);
        }

        Square cutpos2(position.x+1, ycut);
        if(board->getPieceOnCoords(cutpos2) != nullptr) {
            moves->push_back(cutpos2);
        }

        if(board->getEnPassantSquare() == cutpos2) {
            moves->push_back(cutpos2);
        }


        Square moveForward(position.x, ycut);
        moves->push_back(moveForward);

        if(!moved) {
            Square moveByTwo(position.x, color == WHITE ? position.y + 2 : position.y - 2);
            moves->push_back(moveByTwo);
        }
    }

    void Pawn::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        //cut moves
        int ycut = color == WHITE ? position.y + 1 : position.y - 1;
        for(int i = -1; i < 2; i += 2) {
            Square cutpos(position.x + i, ycut);
            if(board->coordOutOfBounds(cutpos)) continue;
            shared_ptr<AbstractPiece> p = board->getPieceOnCoords(cutpos);
            if(p != nullptr && p->getColor() != getColor()) {
                if(!onlyPinsAndProtects) validMoves->push_back(cutpos);
            } else if (p != nullptr && p->getColor() == getColor()) {
                p->setProtectedBy(this);
            } else if(board->getEnPassantSquare() == cutpos) {
                if(!onlyPinsAndProtects) validMoves->push_back(cutpos);
            }
        }

        Square moveForward(position.x, ycut);
        if(board->getPieceOnCoords(moveForward) == nullptr) {
            if(!onlyPinsAndProtects) validMoves->push_back(moveForward);
        }

        Square moveByTwo(position.x, color == WHITE ? position.y + 2 : position.y - 2);
        if(!moved && board->getPieceOnCoords(moveByTwo) == nullptr && board->getPieceOnCoords(moveForward) == nullptr) {
            if(!onlyPinsAndProtects) validMoves->push_back(moveByTwo);
        }
    }

    vector<Square> Pawn::getSquaresAttacked() const {
        vector<Square> v;
        int ycut = color == WHITE ? position.y + 1 : position.y - 1;
        Square pos1(position.x-1, ycut);
        v.push_back(pos1);

        Square pos2(position.x+1, ycut);
        v.push_back(pos2);
        return v;
    }

    int Pawn::getId() const {
        return id;
    }

    void Rook::updateMoves() {
        for(int* i: rookMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == getColor()) {
                    breakInner = true;
                    continue;
                }
                moves->push_back(newPos);
            }
        }
    }

    void Rook::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: rookMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx,dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(board->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Rook::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: rookMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Knight::updateMoves() {
        for(int* i: knightMoves) {
            int dx = position.x;
            int dy = position.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
            if(!board->coordOutOfBounds(newPos) && (p == nullptr || (p != nullptr && p->getColor() != getColor()))) {
                moves->push_back(newPos);
            }
        }
    }

    void Knight::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: knightMoves) {
            int dx = position.x;
            int dy = position.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
            if(!board->coordOutOfBounds(newPos)) {
                if(p != nullptr && p->getColor() == getColor()) {
                    p->setProtectedBy(this);
                    continue;
                }
                if(!onlyPinsAndProtects) validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Knight::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: knightMoves) {
            int dx = position.x;
            int dy = position.y;
            Square newPos(dx+i[0], dy+i[1]);
            shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
            if(!board->coordOutOfBounds(newPos)) {
                if(p != nullptr && p->getColor() == getColor()) {
                    continue;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Bishop::updateMoves() {
        for(int* i: bishopMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == getColor()) {
                    breakInner = true;
                    continue;
                }
                moves->push_back(newPos);
            }
        }
    }

    void Bishop::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: bishopMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(board->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Bishop::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: bishopMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void Queen::updateMoves() {
        for(int* i: queenKingMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) continue;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if(p != nullptr && p->getColor() == getColor()) {
                    breakInner = true;
                    continue;
                }
                moves->push_back(newPos);
            }
        }
    }

    void Queen::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: queenKingMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if(p != nullptr) {
                    breakInner = true;
                    if(p->getColor() == getColor()) {
                        p->setProtectedBy(this);
                        continue;
                    } else {
                        if(board->checkPinnedPiece(this, p.get())) {
                            p->setPinnedBy(this);
                            p->updateValidMovesWhilePinned();
                        }
                    }
                }
                if(!onlyPinsAndProtects) validMoves->push_back(newPos);
            }
        }
    }

    vector<Square> Queen::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: queenKingMoves) {
            int dx = position.x;
            int dy = position.y;
            bool breakInner = false;
            dx += i[0];
            dy += i[1];
            //for cycle runs until coordinates are out of bounds
            for(;!board->coordOutOfBounds(Square(dx, dy));dx += i[0], dy += i[1]) {
                if(breakInner) break;
                Square newPos(dx, dy);
                shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
                if((p != nullptr && p->getKind() != KING) || (p != nullptr && p->getKind() == KING && p->getColor() == getColor())) {
                    breakInner = true;
                }
                v.push_back(newPos);
            }
        }
        return v;
    }

    void King::updateMoves() {
        for(int* i: queenKingMoves) {
            Square newPos(position.x+i[0], position.y+i[1]);
            if(board->coordOutOfBounds(newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
            if(p != nullptr && p->getColor() == getColor()) continue;
            moves->push_back(newPos);
        }

        if(!hasMoved() && board->isPlayerInCheck() != getColor() && board->canCastle()) {
            vector<shared_ptr<AbstractPiece>> rooks = board->getPiecesOfColorAndKind(color, ROOK);
            for(const shared_ptr<AbstractPiece>& r: rooks) {
                if(!r->hasMoved()) {
                    vector<Square> squaresBetween = board->getSquaresBetween(r.get(), this);
                    if(squaresBetween.size() == 2) {
                        //short castle

                        bool breakinner = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board->getPieceOnCoords(i) != nullptr) {
                                breakinner = true;
                                break;
                            }
                        }
                        if(breakinner) continue;

                        int dx = r->getPosition().x > position.x ? r->getPosition().x - 1: r->getPosition().x + 1;
                        Square newPos(dx, position.y);
                        moves->push_back(newPos);
                    } else {
                        //long castle

                        bool breakinner = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board->getPieceOnCoords(i) != nullptr) {
                                breakinner = true;
                                break;
                            }
                        }
                        if(breakinner) continue;


                        int dx = r->getPosition().x > position.x ? r->getPosition().x - 2: r->getPosition().x + 2;
                        Square newPos(dx, position.y);
                        moves->push_back(newPos);
                    }
                }
            }
        }
    }

    void King::updateValidMovesPinsProtects(bool onlyPinsAndProtects) {
        for(int* i: queenKingMoves) {
            Square newPos(position.x+i[0], position.y+i[1]);
            if(board->coordOutOfBounds(newPos) || board->isSquareUnderAttack(invertColor(getColor()), newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
            if(p != nullptr && p->getColor() == getColor()) {
                p->setProtectedBy(this);
                continue;
            }
            if(p != nullptr && p->getProtectedBy() != nullptr) continue;
            validMoves->push_back(newPos);
        }

        if(!hasMoved() && board->isPlayerInCheck() != getColor() && board->canCastle()) {
            vector<shared_ptr<AbstractPiece>> rooks = board->getPiecesOfColorAndKind(color, ROOK);
            for(const shared_ptr<AbstractPiece>& r: rooks) {
                if(!r->hasMoved()) {
                    vector<Square> squares = board->getSquaresBetween(r.get(), this);
                    bool breakinner = false;
                    for(Square i: squares) {
                        if(board->isSquareUnderAttack(invertColor(color), i)) {
                            breakinner = true;
                        }
                    }
                    if(breakinner) continue;
                    vector<Square> squaresBetween = board->getSquaresBetween(r.get(), this);
                    if(squaresBetween.size() == 2) {
                        //short castle

                        bool breakin = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board->getPieceOnCoords(i) != nullptr) {
                                breakin = true;
                                break;
                            }
                        }
                        if(breakin) continue;

                        int dx = r->getPosition().x > position.x ? r->getPosition().x - 1: r->getPosition().x + 1;
                        Square newPos(dx, position.y);
                        validMoves->push_back(newPos);
                    } else {
                        //long castle

                        bool breakin = false;
                        //check if there are pieces in the way
                        for(Square i: squaresBetween) {
                            if(board->getPieceOnCoords(i) != nullptr) {
                                breakin = true;
                                break;
                            }
                        }
                        if(breakin) continue;

                        int dx = r->getPosition().x > position.x ? r->getPosition().x - 2: r->getPosition().x + 2;
                        Square newPos(dx, position.y);
                        validMoves->push_back(newPos);
                    }
                }
            }
        }
    }

    vector<Square> King::getSquaresAttacked() const {
        vector<Square> v;
        for(int* i: queenKingMoves) {
            Square newPos(position.x+i[0], position.y+i[1]);
            if(board->coordOutOfBounds(newPos)) {
                continue;
            }
            shared_ptr<AbstractPiece> p = board->getPieceOnCoords(newPos);
            if(p != nullptr) {
                continue;
            }
            v.push_back(newPos);
        }
        return v;
    }
}

namespace GTLib2 {
    namespace domains {
        using chess::Square;

        KriegspielAction::KriegspielAction
        (ActionId id, pair<shared_ptr<AbstractPiece>, Square> move, Square originalPos):
        Action(id), moveFrom(originalPos), move_(std::move(move)) {
        }

        KriegspielAction::KriegspielAction(ActionId id):
        Action(id), moveFrom(-1, -1),
        move_(pair<shared_ptr<AbstractPiece>, Square>(nullptr, Square(-1, -1))) {

        }

        Square KriegspielAction::movingFrom() const {
            return moveFrom;
        }

        KriegspielDomain::KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, chess::BOARD b):Domain(maxDepth, 2) {
            vector<double> rewards(2);
            vector<shared_ptr<Observation>> Obs{make_shared<Observation>(NO_OBSERVATION), make_shared<Observation>(NO_OBSERVATION)};
            Outcome o(make_shared<KriegspielState>(this, legalMaxDepth, b), Obs, rewards);
            rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
            maxUtility_ = 1;
        }
        KriegspielObservation::KriegspielObservation(int id):Observation(id) {}
        KriegspielState::KriegspielState(GTLib2::Domain *domain, int legalMaxDepth, chess::BOARD b):State(domain), enPassantSquare(-1, -1) {
            moveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            attemptedMoveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            playerOnTheMove = chess::WHITE;
            legalMaxDepth = legalMaxDepth;
            initBoard(b);
            updateState(chess::WHITE);
        }

        KriegspielState::KriegspielState(Domain *domain, int legalMaxDepth, string s): State(domain), enPassantSquare(-1, -1) {
            moveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            attemptedMoveHistory = make_shared<vector<shared_ptr<KriegspielAction>>>();
            playerOnTheMove = chess::WHITE;
            legalMaxDepth = legalMaxDepth;
            initBoard(std::move(s));
            updateAllPieces();
        }

        KriegspielState::KriegspielState(GTLib2::Domain *domain, int legalMaxDepth, int x, int y, shared_ptr<vector<shared_ptr<chess::AbstractPiece>>> pieces,
                                         Square enPassantSquare, shared_ptr<vector<shared_ptr<KriegspielAction>>> moves, int p, bool castle,
                                         shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedMoves):State(domain), enPassantSquare(enPassantSquare) {
            xSize = x;
            ySize = y;
            pieces = std::move(pieces);
            moveHistory = std::move(moves);
            legalMaxDepth = legalMaxDepth;
            attemptedMoveHistory = std::move(attemptedMoves);
            canPlayerCastle = castle;

            //rebind to new board state
            for(shared_ptr<AbstractPiece>& piece: *pieces) {
                piece->setBoard(this);
            }
            playerOnTheMove = p;
        }

        pair<shared_ptr<AbstractPiece>, Square> KriegspielAction::getMove() const {
            return move_;
        }

        bool KriegspielAction::operator==(const GTLib2::Action &that) const {
            const auto rhsAction = dynamic_cast<const KriegspielAction *>(&that);
            return move_.first->equal_to(*rhsAction->move_.first) && move_.second == rhsAction->move_.second && movingFrom() == rhsAction->movingFrom();
        }

        size_t KriegspielAction::getHash() const {
            std::hash<string> h;
            string s = move_.first->toString() + chess::coordToString(move_.second) + chess::coordToString(movingFrom());
            return h(s);
        }

        shared_ptr<KriegspielAction> KriegspielAction::clone() const {
            Square s = Square(getMove().second.x, getMove().second.y);
            shared_ptr<AbstractPiece> p = getMove().first->clone();
            Square f = Square(movingFrom().x, movingFrom().y);
            auto a = make_shared<KriegspielAction>(id, pair<shared_ptr<AbstractPiece>, chess::Square>(p, s), f);
            return a;
        }

        string KriegspielDomain::getInfo() const {
            return "************ Kriegspiel *************\n" +
                   rootStatesDistribution[0].first.state->toString() + "\n";
        }

        KriegspielDomain::KriegspielDomain(unsigned int maxDepth, unsigned int legalMaxDepth, string s): Domain(maxDepth, 2) {
            vector<double> rewards(2);
            vector<shared_ptr<Observation>> Obs{make_shared<Observation>(NO_OBSERVATION), make_shared<Observation>(-1)};
            Outcome o(make_shared<KriegspielState>(this, legalMaxDepth, s), Obs, rewards);
            rootStatesDistribution.push_back(pair<Outcome, double>(move(o), 1.0));
        }

        vector<shared_ptr<Action>> KriegspielState::getAvailableActionsFor(Player player) const {
            auto list = vector<shared_ptr<Action>>();
            if(player != playerOnTheMove || gameHasEnded || moveHistory->size() == legalMaxDepth) {
                return list;
            }
            int count = 0;
            for (shared_ptr<AbstractPiece>p: *pieces) {
                if(p->getColor() != player) continue;
                auto moves = p->getAllMoves();
                for(Square move: *moves) {
                    auto ksa = make_shared<KriegspielAction>(count, std::pair<shared_ptr<AbstractPiece>, Square>(p, move), p->getPosition());
                    if(std::find(attemptedMoveHistory->begin(), attemptedMoveHistory->end(), ksa) == attemptedMoveHistory->end()) {
                        list.push_back(ksa);
                        ++count;
                    }
                }
            }
            return list;
        }

        void KriegspielState::promote(shared_ptr<AbstractPiece> p, Square pos) const {
            auto q = make_shared<chess::Queen>(chess::QUEEN, p->getColor(), pos, this);
            for(auto It = pieces->begin(); It != pieces->end();) {
                if(*(*It) == *p) {
                    pieces->erase(It);
                    pieces->push_back(q);
                    break;
                }
                else {
                    ++It;
                }
            }
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getPiecesOfColor(int color) const {
            vector<shared_ptr<AbstractPiece>> v;
            for(shared_ptr<AbstractPiece>& p: *pieces) {
                if(p->getColor() == color) {
                    v.push_back(p);
                }
            }
            return v;
        }

        int KriegspielState::isPlayerInCheck() const {
            return playerInCheck;
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getCheckingFigures() const {
            return checkingFigures;
        }

        int KriegspielState::checkGameOverCheck(int c) const {
            int inCheck = -1;
            shared_ptr<AbstractPiece> king = getPiecesOfColorAndKind(c, chess::KING)[0];
            for(const shared_ptr<AbstractPiece>& p: *pieces) {
                if(p->getColor() == c) continue;
                Square kingPos = king->getPosition();
                vector<Square> attackSquares = p->getSquaresAttacked();
                if(std::find(attackSquares.begin(), attackSquares.end(), kingPos) != attackSquares.end()) {
                    inCheck = king->getColor();
                }
            }
            return inCheck;
        }

        void KriegspielState::checkPlayerInCheck() {
            int kingColor = playerOnTheMove;
            playerInCheck = -1;
            shared_ptr<AbstractPiece> king = getPiecesOfColorAndKind(kingColor, chess::KING)[0];
            for(const shared_ptr<AbstractPiece>& p: *pieces) {
                if(p->getColor() == kingColor) continue;
                Square kingPos = king->getPosition();
                vector<Square> attackSquares = p->getSquaresAttacked();
                if(std::find(attackSquares.begin(), attackSquares.end(), kingPos) != attackSquares.end()) {
                    playerInCheck = kingColor;
                    checkingFigures.push_back(p);
                }
            }
        }

        bool KriegspielState::canCastle() const {
            return canPlayerCastle;
        }

        void KriegspielState::castle(KriegspielAction* a) const {
            pair<shared_ptr<AbstractPiece>, Square> move = a->getMove();
            shared_ptr<AbstractPiece> king = move.first;
            Square pos = move.second;
            int dx = king->getPosition().x;
            int dy = king->getPosition().y;
            //get rook in the direction of the move so we don't have to make special cases for other boards and black/white board asymmetry
            int incrementX = pos.x > dx ? 1 : -1;
            shared_ptr<AbstractPiece> rook;
            dx += incrementX;
            for(; ((rook = getPieceOnCoords(Square(dx, dy))) == nullptr); dx += incrementX);
            int newRookX = pos.x > rook->getPosition().x ? pos.x + 1 : pos.x - 1;
            Square newRookPos(newRookX, rook->getPosition().y);
            rook->move(newRookPos);
        }

        bool KriegspielState::move(KriegspielAction * a) {
            pair<shared_ptr<AbstractPiece>, Square> move = a->getMove();
            shared_ptr<AbstractPiece> p = getPieceOnCoords(move.first->getPosition());
            Square pos = move.second;

            if(std::find(p->getAllValidMoves()->begin(), p->getAllValidMoves()->end(), pos) != p->getAllValidMoves()->end()) {
                //move valid
                shared_ptr<AbstractPiece> checkCut = getPieceOnCoords(pos);
                if(checkCut != nullptr) {
                    lastCut = 30 + checkCut->getPosition().x + ((checkCut->getPosition().y-1) * ySize);
                    for(auto It = pieces->begin(); It != pieces->end();) {
                        if(*(*It) == *checkCut) {
                            pieces->erase(It);
                            break;
                        }
                        else {
                            ++It;
                        }
                    }
                }

                if(pos.x == enPassantSquare.x && pos.y == enPassantSquare.y) {
                    int pawnColor = chess::invertColor(p->getColor());
                    int y = pawnColor == chess::WHITE ? pos.y + 1 : pos.y - 1;
                    Square enPassPos(pos.x, y);
                    shared_ptr<AbstractPiece> enPassPawn = getPieceOnCoords(enPassPos);
                    if(enPassPawn != nullptr) {
                        for(auto It = pieces->begin(); It != pieces->end();) {
                            if(*(*It) == *enPassPawn) {
                                pieces->erase(It);
                                break;
                            }
                            else {
                                ++It;
                            }
                        }
                    }
                }

                if(p->getKind() == chess::KING && abs(a->movingFrom().x - pos.x) > 1) {
                    castle(a);
                }
                p->move(pos);
                checkPlayerInCheck();
                if(p->getKind() == chess::PAWN && ((pos.y == getYSize() && p->getColor() == chess::WHITE) || (pos.y == 1 && p->getColor() == chess::BLACK))) {
                    promote(p, pos);
                }
                return true;
            } else {
                return false;
            }
        }

        vector<double> KriegspielState::checkGameOver() const {
            vector<double> rewards(2);
            vector<shared_ptr<AbstractPiece>> whitePieces = getPiecesOfColor(0);
            vector<shared_ptr<AbstractPiece>> blackPieces = getPiecesOfColor(1);
            if(whitePieces.size() <= 2 && blackPieces.size() <= 2) {
                bool isDraw = true;
                whitePieces.insert(whitePieces.end(), blackPieces.begin(), blackPieces.end());
                for(const shared_ptr<AbstractPiece>& p: whitePieces) {
                    if(p->getKind() == chess::KING) continue;
                    else if(p->getKind() == chess::PAWN || p->getKind() == chess::ROOK || p->getKind() == chess::QUEEN) isDraw = false;
                }
                if(isDraw) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                    return rewards;
                }
            }
            rewards[chess::WHITE] = 0;
            rewards[chess::BLACK] = 0;
            bool hasMoves[] = {false, false};
            int isCheck = checkGameOverCheck(playerOnTheMove);

            for(const shared_ptr<AbstractPiece>& p: *pieces) {
                hasMoves[p->getColor()] = hasMoves[p->getColor()] ? true : !p->getAllValidMoves()->empty();
            }

            if(isCheck == playerOnTheMove) {
                if(!hasMoves[playerOnTheMove]) {
                    rewards[playerOnTheMove] = 0;
                    rewards[chess::invertColor(playerOnTheMove)] = 1;
                } else if(moveHistory->size() == legalMaxDepth) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                } else if(moveHistory->size() + attemptedMoveHistory->size() == domain->getMaxDepth()) {
                    rewards[chess::WHITE] = 0.5;
                    rewards[chess::BLACK] = 0.5;
                }
            } else if(!hasMoves[playerOnTheMove]) {
                rewards[chess::WHITE] = 0.5;
                rewards[chess::BLACK] = 0.5;
            } else if(moveHistory->size() == legalMaxDepth) {
                rewards[chess::WHITE] = 0.5;
                rewards[chess::BLACK] = 0.5;
            }

            return rewards;
        }

        Square KriegspielState::getEnPassantSquare() const {
            return enPassantSquare;
        }



        OutcomeDistribution KriegspielState::performActions(
                const vector<pair<Player, shared_ptr<GTLib2::Action>>> &actions) const {
            auto a1 = dynamic_cast<KriegspielAction*>(actions[0].second.get());
            auto a2 = dynamic_cast<KriegspielAction*>(actions[1].second.get());
            vector<shared_ptr<Observation>> observations(2);
            vector<double> rewards(2);
            shared_ptr<KriegspielState> s;
            int nextMove = playerOnTheMove;
            Square enPassSquare(-1, -1);
            KriegspielAction* a = a1 ? a1 : a2;
            shared_ptr<vector<shared_ptr<AbstractPiece>>> pieces = copyPieces();
            shared_ptr<vector<shared_ptr<KriegspielAction>>> history = copyMoveHistory();
            shared_ptr<vector<shared_ptr<KriegspielAction>>> attemptedmoves = copyAttemptedMoves();
            s = make_shared<KriegspielState>(domain, legalMaxDepth, xSize, ySize, pieces, enPassantSquare, history, nextMove, canPlayerCastle, attemptedmoves);
            s->updateState(playerOnTheMove);
            shared_ptr<KriegspielAction> ac = a->clone();
            if(s->move(a)) {
                s->addToHistory(ac);
                enPassSquare = checkEnPassant(a);
                nextMove = chess::invertColor(playerOnTheMove);
                s->setPlayerOnMove(nextMove);
                s->setEnPassant(enPassSquare);
                s->updateState(nextMove);
                rewards = s->checkGameOver();
                observations[playerOnTheMove] = make_shared<Observation>(1);
                observations[chess::invertColor(playerOnTheMove)] = make_shared<Observation>(s->calculateObservation(chess::invertColor(playerOnTheMove)));
                int x = 4;
            } else {
                s->addToAttemptedMoves(ac);
                nextMove = playerOnTheMove;
                observations[playerOnTheMove] = make_shared<Observation>(a->getId());
                observations[chess::invertColor(playerOnTheMove)] = make_shared<Observation>(NO_OBSERVATION);
                s->updateState(nextMove);
                rewards = s->checkGameOver();
            }
            if(rewards[0] != 0 || rewards[1] != 0) {
                s->setGameHasEnded(true);
            }
            Outcome o(std::move(s), observations, rewards);
            OutcomeDistribution prob;
            prob.push_back(std::pair<Outcome, double>(std::move(o), 1.0));
            return prob;
        }

        Square KriegspielState::checkEnPassant(KriegspielAction * a) const {
            shared_ptr<AbstractPiece> abstractPiece = a->getMove().first;
            Square pos(-1, -1);
            if(abstractPiece->getKind() != chess::PAWN) {
                return pos;
            } else {
                if((a->getMove().second.y - a->movingFrom().y) == 2) {
                    pos.x = a->movingFrom().x;
                    pos.y = a->movingFrom().y + 1;
                    return pos;
                } else if ((a->getMove().second.y - a->movingFrom().y) == -2){
                    pos.x = a->movingFrom().x;
                    pos.y = a->movingFrom().y - 1;
                    return pos;
                } else {
                    return pos;
                }
            }
        }

        bool KriegspielState::operator==(const GTLib2::State &rhs) const {
            const auto rhsState = dynamic_cast<const KriegspielState *>(&rhs);
            if(xSize != rhsState->xSize || ySize != rhsState->ySize) {
                return false;
            }
            for(int i = 1; i <= xSize; i++) {
                for(int j = 1; j <= ySize; j++) {
                    shared_ptr<AbstractPiece> p1 = getPieceOnCoords(Square(i, j));
                    shared_ptr<AbstractPiece> p2 = getPieceOnCoords(Square(i, j));
                    if(p1 == nullptr && p2 == nullptr) continue;
                    if((p1 != nullptr && p2 == nullptr) || (p1 == nullptr && p2 != nullptr)) return false;
                    if(!(*p1 == *p2)) return false;
                }
            }
            return true;
        }

        size_t KriegspielState::getHash() const {
            std::hash<string> h;
            return h(toString());
        }

        string KriegspielState::toString() const {
            //DEPRECATED
            vector<vector<string>> board;
            for(int j = 0; j < ySize; j++) {
                vector<string> row;
                for(int i = 0; i < xSize; i++) {
                    row.emplace_back("_");
                }
                board.push_back(row);
            }

            for(shared_ptr<AbstractPiece>p: *pieces) {
                Square pos = p->getPosition();
                pos.y -= 1;
                pos.x -= 1;
                board.at(pos.y).at(pos.x) = p->toString();
            }

            string s;
            for(int i = ySize-1; i >= 0; i--) {
                vector<string> row = board.at(i);
                for(const string &str: row) {
                    s += str;
                }
                s += "\n";
            }
            s += "oooooooo\n";
            return s;

            /*chess::FenBoardFactory f;
            return f.createFEN(this);*/
        }

        shared_ptr<AbstractPiece> KriegspielState::getPieceOnCoords(Square pos) const {
            for(const shared_ptr<AbstractPiece>& p: *pieces) {
                if(p->getPosition() == pos) {
                    return p;
                }
            }
            return nullptr;
        }

        bool KriegspielState::coordOutOfBounds(Square pos) const {
            return pos.x > xSize || pos.y > ySize || pos.x < 1 || pos.y < 1;
        }

        void KriegspielState::updateAllPieces() const {
            for(shared_ptr<AbstractPiece>& p: *pieces) {
                p->update();
            }
        }

        void KriegspielState::resetAllPieces() {
            for(shared_ptr<AbstractPiece>& p: *pieces) {
                p->reset();
            }
        }

        void KriegspielState::updatePiecesOfColor(int color) const {
            vector<shared_ptr<AbstractPiece>> v = getPiecesOfColor(color);
            for(shared_ptr<AbstractPiece>& p: v) {
                p->update();
            }
        }

        vector<shared_ptr<AbstractPiece>> KriegspielState::getPiecesOfColorAndKind(int c, chess::pieceName n) const {
            vector<shared_ptr<AbstractPiece>> v;
            for(const shared_ptr<AbstractPiece>& p: *pieces) {
                if(p->getColor() == c && p->getKind() == n) {
                    v.push_back(p);
                }
            }
            return v;
        }

        bool KriegspielState::isSquareUnderAttack(int color, Square square) const {
            vector<shared_ptr<AbstractPiece>> pieces = getPiecesOfColor(color);
            for(const shared_ptr<AbstractPiece>& p: pieces) {
                vector<Square> attackedSquares = p->getSquaresAttacked();
                if(std::find(attackedSquares.begin(), attackedSquares.end(), square) != attackedSquares.end()) {
                    return true;
                }
            }
            return false;
        }

        void KriegspielState::clearBoard() {
            pieces->clear();
        }

        void KriegspielState::insertPiece(shared_ptr<AbstractPiece> p) {
            pieces->push_back(p);
        }

        vector<Square> KriegspielState::getSquaresPierced(AbstractPiece* pierced, AbstractPiece* piercer) const {
            Square posPiercer = piercer->getPosition();
            Square posPierced = pierced->getPosition();
            vector<Square> squares;

            int dx = posPierced.x;
            int dy = posPierced.y;

            int incrementX = dx == posPiercer.x ? 0 : dx < posPiercer.x ? -1 : 1;
            int incrementY = dy == posPiercer.y ? 0 : dy < posPiercer.y ? -1 : 1;

            dx += incrementX;
            dy += incrementY;

            for(; !coordOutOfBounds(Square(dx, dy)); dx += incrementX, dy += incrementY) {
                Square newPos(dx, dy);
                squares.push_back(newPos);
            }
            return squares;
        }

        vector<Square> KriegspielState::getSquaresBetween(AbstractPiece* p1, AbstractPiece* p2) const {

            Square pos1 = p1->getPosition();
            Square pos2 = p2->getPosition();
            vector<Square> squares;

            int dx = pos1.x;
            int dy = pos1.y;

            int incrementX = dx == pos2.x ? 0 : dx < pos2.x ? 1 : -1;
            int incrementY = dy == pos2.y ? 0 : dy < pos2.y ? 1 : -1;

            dx += incrementX;
            dy += incrementY;

            for(; !(dx == pos2.x && dy == pos2.y); dx += incrementX, dy += incrementY) {
                Square newPos(dx, dy);
                if(coordOutOfBounds(newPos)) {
                    string s1 = "Position 1: " + to_string(p1->getPosition().x) + ", " + to_string(p1->getPosition().y) + "\r\n"
                                                                                                                          "Figure 1: " + p1->toString() + "\r\n";
                    string s2 = "Position 2: " + to_string(p2->getPosition().x) + ", " + to_string(p2->getPosition().y) + "\r\n"
                                                                                                                          "Figure 2: " + p2->toString() + "\r\n";
                    std::cout << s1;
                    std::cout << s2;
                    throw std::invalid_argument("wrong figures");
                }
                squares.push_back(newPos);
            }

            return squares;
        }

        bool KriegspielState::checkPinnedPiece(AbstractPiece* pinner, AbstractPiece* pinned) const {
            int kingColor = pinned->getColor();
            shared_ptr<AbstractPiece> king = getPiecesOfColorAndKind(kingColor, chess::pieceName::KING)[0];
            vector<Square> squaresToCheck = getSquaresPierced(pinned, pinner);
            for(Square i: squaresToCheck) {
                shared_ptr<AbstractPiece> p = getPieceOnCoords(i);
                if(p != nullptr) {
                    return p == king;
                }
            }
            return false;
        }

        void KriegspielState::initBoard(chess::BOARD b) {
            chess::BoardFactory bf;
            chess::boardInfo binfo = bf.create(b, this);
            pieces = binfo.pieces;
            xSize = binfo.x;
            ySize = binfo.y;
            canPlayerCastle = binfo.castle;
        }

        int KriegspielState::getXSize() const {
            return xSize;
        }

        int KriegspielState::getYSize() const {
            return ySize;
        }

        void KriegspielState::setPlayerOnMove(int p) {
            playerOnTheMove = p;
        }

        void KriegspielState::setGameHasEnded(bool gameHasEnded) {
            gameHasEnded = gameHasEnded;
        }

        shared_ptr<vector<shared_ptr<AbstractPiece>>> KriegspielState::copyPieces() const {
            shared_ptr<vector<shared_ptr<AbstractPiece>>> v = make_shared<vector<shared_ptr<AbstractPiece>>>();
            for(const shared_ptr<AbstractPiece>& p: *pieces) {
                auto asdf = p->clone();
                v->push_back(asdf);
            }
            return v;
        }

        shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyMoveHistory() const {
            shared_ptr<vector<shared_ptr<KriegspielAction>>> v = make_shared<vector<shared_ptr<KriegspielAction>>>();
            for(const shared_ptr<KriegspielAction>& a: *moveHistory) {
                auto asdf = a->clone();
                v->push_back(asdf);
            }
            return v;
        }

        void KriegspielState::addToHistory(shared_ptr<KriegspielAction> a) {
            moveHistory->push_back(a);
            attemptedMoveHistory->clear();
        }

        void KriegspielState::setEnPassant(chess::Square s) {
            enPassantSquare = s;
            //refresh moves on pawns after setting enpassant square
            vector<shared_ptr<AbstractPiece>> pawns = getPiecesOfColorAndKind(playerOnTheMove, chess::PAWN);
            for(shared_ptr<AbstractPiece>& p: pawns) {
                p->update();
            }
        }

        void KriegspielState::updateState(int p) {
            playerOnTheMove = p;
            resetAllPieces();
            updatePiecesOfColor(chess::invertColor(playerOnTheMove));
            checkPlayerInCheck();
            updatePiecesOfColor(playerOnTheMove);

            //update kings again to remove moves that would cut protected pieces
            shared_ptr<AbstractPiece> whiteKing = getPiecesOfColorAndKind(chess::WHITE, chess::KING)[0];
            shared_ptr<AbstractPiece> blackKing = getPiecesOfColorAndKind(chess::BLACK, chess::KING)[0];
            whiteKing->reset();
            blackKing->reset();
            whiteKing->update();
            blackKing->update();
        }

        void KriegspielState::initBoard(string s) {
            chess::FenBoardFactory bf;
            chess::boardInfo binfo = bf.create(std::move(s), this);
            pieces = binfo.pieces;
            xSize = binfo.x;
            ySize = binfo.y;
            canPlayerCastle = binfo.castle;
            if(binfo.xEnpass) {
                Square sq(binfo.xEnpass, binfo.yEnpass);
                setEnPassant(sq);
            }
        }

        shared_ptr<vector<shared_ptr<KriegspielAction>>> KriegspielState::copyAttemptedMoves() const {
            shared_ptr<vector<shared_ptr<KriegspielAction>>> v = make_shared<vector<shared_ptr<KriegspielAction>>>();
            for(const shared_ptr<KriegspielAction>& a: *attemptedMoveHistory) {
                auto asdf = a->clone();
                v->push_back(asdf);
            }
            return v;
        }

        void KriegspielState::addToAttemptedMoves(shared_ptr<KriegspielAction> a) {
            attemptedMoveHistory->push_back(a);
        }

        int KriegspielState::calculateObservation(int player) const {
            auto pawns = getPiecesOfColorAndKind(player, chess::PAWN);
            int toreturn = lastCut;
            for(const shared_ptr<AbstractPiece>& p: pawns) {
                auto curr = dynamic_cast<chess::Pawn*>(p.get());
                for(auto move: *curr->getAllMoves()) {
                    if(move.x == curr->getPosition().x - 1) {
                        toreturn += curr->getId();
                    } else if(move.x == curr->getPosition().x + 1) {
                        toreturn += curr->getId()+1;
                    }
                }
            }
            return toreturn;
        }
    }
}

#pragma clang diagnostic pop
