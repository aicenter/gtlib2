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

#include "chessboard_factory.h"


namespace GTLib2::domains::chess {
boardInfo BoardFactory::create(BOARD b, GTLib2::domains::KriegspielState *s) {
    switch (b) {
        case STANDARD:
            return createStandard(s);
        case MICROCHESS:
            return createMicrochess(s);
        case MINIMAL4x3:
            return createMinimal(s);
        case DEMICHESS:
            return createDemichess(s);
        case SILVERMAN4BY4:
            return createSilverman4x4(s);
        default:
            unreachable("unrecognized option!");
    }
}

boardInfo BoardFactory::createStandard(GTLib2::domains::KriegspielState *b) {
    boardInfo binfo;
    binfo.pieces = make_shared<vector<shared_ptr<AbstractPiece>>>();
    for (int j = WHITE; j <= BLACK; j++) {
        int currid = 1;
        int y = j == WHITE ? 2 : 7;
        for (int i = 1; i < 9; i++) {
            Square pos(i, y);
            binfo.pieces->push_back(make_shared<Pawn>(PAWN, j, pos, b, currid));
            currid += 2;
        }

        y = j == WHITE ? 1 : 8;
        for (int i = 1; i < 9; i++) {
            Square pos(i, y);
            switch (i) {
                case 1:
                case 8:
                    binfo.pieces->push_back(make_shared<Rook>(ROOK, j, pos, b));
                    break;
                case 2:
                case 7:
                    binfo.pieces->push_back(make_shared<Knight>(KNIGHT, j, pos, b));
                    break;
                case 3:
                case 6:
                    binfo.pieces->push_back(make_shared<Bishop>(BISHOP, j, pos, b));
                    break;
                case 4:
                    binfo.pieces->push_back(make_shared<Queen>(QUEEN, j, pos, b));
                    break;
                case 5:
                    binfo.pieces->push_back(make_shared<King>(KING, j, pos, b));
                    break;
                default:
                    break;
            }
        }
    }
    binfo.castle = true;
    binfo.x = 8;
    binfo.y = 8;
    return binfo;
}

boardInfo BoardFactory::createSilverman4x4(
    GTLib2::domains::KriegspielState *b) {
    boardInfo binfo;
    binfo.pieces = make_shared<vector<shared_ptr<AbstractPiece>>>();
    for (int j = WHITE; j <= BLACK; j++) {
        int currid = 1;
        int y = j == WHITE ? 2 : 3;
        for (int i = 1; i < 5; i++) {
            Square pos(i, y);
            binfo.pieces->push_back(make_shared<Pawn>(PAWN, j, pos, b, currid));
            currid += 2;
        }

        y = j == WHITE ? 1 : 4;
        for (int i = 1; i < 5; i++) {
            Square pos(i, y);
            switch (i) {
                case 2:
                    binfo.pieces->push_back(make_shared<Queen>(QUEEN, j, pos, b));
                    break;
                case 3:
                    binfo.pieces->push_back(make_shared<King>(KING, j, pos, b));
                    break;
                default:
                    binfo.pieces->push_back(make_shared<Rook>(ROOK, j, pos, b));
                    break;
            }
        }
    }
    binfo.castle = false;
    binfo.x = 4;
    binfo.y = 4;
    return binfo;
}

boardInfo BoardFactory::createMinimal(
    GTLib2::domains::KriegspielState *b) {
    boardInfo binfo;
    binfo.pieces = make_shared<vector<shared_ptr<AbstractPiece>>>();
    binfo.pieces->push_back(make_shared<King>(KING, WHITE, Square(3, 1), b));
    binfo.pieces->push_back(make_shared<King>(KING, BLACK, Square(1, 3), b));
    binfo.pieces->push_back(make_shared<Rook>(ROOK, WHITE, Square(4, 2), b));
    binfo.castle = false;
    binfo.x = 4;
    binfo.y = 3;
    return binfo;
}

boardInfo BoardFactory::createMicrochess(
    GTLib2::domains::KriegspielState *b) {
    boardInfo binfo;
    binfo.pieces = make_shared<vector<shared_ptr<AbstractPiece>>>();
    for (int j = WHITE; j <= BLACK; j++) {
        int y = j == WHITE ? 2 : 4;
        Square pos(j == WHITE ? 4 : 1, y);
        binfo.pieces->push_back(make_shared<Pawn>(PAWN, j, pos, b, 1));

        y = j == WHITE ? 1 : 5;
        for (int i = 1; i < 5; i++) {
            Square pos2(i, y);
            switch (i) {
                case 1:
                    if (j == WHITE) {
                        binfo.pieces->push_back(make_shared<Rook>(ROOK, j, pos2, b));
                    } else {
                        binfo.pieces->push_back(make_shared<King>(KING, j, pos2, b));
                    }
                    break;
                case 2:
                    if (j == WHITE) {
                        binfo.pieces->push_back(make_shared<Bishop>(BISHOP, j, pos2, b));
                    } else {
                        binfo.pieces->push_back(make_shared<Knight>(KNIGHT, j, pos2, b));
                    }
                    break;
                case 3:
                    if (j == WHITE) {
                        binfo.pieces->push_back(make_shared<Knight>(KNIGHT, j, pos2, b));
                    } else {
                        binfo.pieces->push_back(make_shared<Bishop>(BISHOP, j, pos2, b));
                    }
                    break;
                case 4:
                    if (j == WHITE) {
                        binfo.pieces->push_back(make_shared<King>(KING, j, pos2, b));
                    } else {
                        binfo.pieces->push_back(make_shared<Rook>(ROOK, j, pos2, b));
                    }
                    break;
            }
        }
    }
    binfo.castle = true;
    binfo.x = 4;
    binfo.y = 5;
    return binfo;
}

boardInfo BoardFactory::createDemichess(
    GTLib2::domains::KriegspielState *b) {
    boardInfo binfo;
    binfo.pieces = make_shared<vector<shared_ptr<AbstractPiece>>>();
    for (int j = WHITE; j <= BLACK; j++) {
        int currid = 1;
        int y = j == WHITE ? 2 : 7;
        for (int i = 1; i < 5; i++) {
            Square pos(i, y);
            binfo.pieces->push_back(make_shared<Pawn>(PAWN, j, pos, b, currid));
            currid += 2;
        }

        y = j == WHITE ? 1 : 8;
        for (int i = 1; i < 5; i++) {
            Square pos(i, y);
            switch (i) {
                case 1:
                    binfo.pieces->push_back(make_shared<King>(KING, j, pos, b));
                    break;
                case 2:
                    binfo.pieces->push_back(make_shared<Bishop>(BISHOP, j, pos, b));
                    break;
                case 3:
                    binfo.pieces->push_back(make_shared<Knight>(KNIGHT, j, pos, b));
                    break;
                default:
                    binfo.pieces->push_back(make_shared<Rook>(ROOK, j, pos, b));
                    break;
            }
        }
    }
    binfo.castle = true;
    binfo.x = 4;
    binfo.y = 8;
    return binfo;
}

std::vector<std::string> split(std::string stringToBeSplitted, std::string delimeter) {
    std::vector<std::string> splittedString;
    unsigned int startIndex = 0;
    unsigned int endIndex = 0;
    while( (endIndex = stringToBeSplitted.find(delimeter, startIndex)) < stringToBeSplitted.size() )
    {
        std::string val = stringToBeSplitted.substr(startIndex, endIndex - startIndex);
        splittedString.push_back(val);
        startIndex = endIndex + delimeter.size();
    }
    if(startIndex < stringToBeSplitted.size())
    {
        std::string val = stringToBeSplitted.substr(startIndex);
        splittedString.push_back(val);
    }
    return splittedString;
}

boardInfo FenBoardFactory::create(string s, GTLib2::domains::KriegspielState *b) {
    boardInfo binfo;
    binfo.pieces = make_shared<vector<shared_ptr<AbstractPiece>>>();
    vector<string> strs = split(s, "\t ");

    string pieceString = strs[0];
    vector<string> rows = split(pieceString, "/");

    int ySize = rows.size();
    int xSize = 0;
    for (char &c: rows[0]) {
        xSize += isdigit(c) ? c - 48 : 1;
    }

    binfo.x = xSize;
    binfo.y = ySize;

    int y = ySize;
    int x = 1;

    vector<shared_ptr<AbstractPiece>> rooks;
    shared_ptr<AbstractPiece> blackKing = nullptr;
    shared_ptr<AbstractPiece> whiteKing = nullptr;

    for (string row: rows) {
        for (char &c: row) {
            if (!isdigit(c)) {
                shared_ptr<AbstractPiece> piece = createPiece(c, x, y, b);
                blackKing = c == 'k' ? piece : blackKing;
                whiteKing = c == 'K' ? piece : whiteKing;
                if (piece->getKind() == ROOK) {
                    rooks.emplace_back(piece);
                }
                binfo.pieces->push_back(move(piece));
            } else {
                x += c - 48;
            }
        }
        y--;
        x = 1;
    }

    if (strs[1] == "b") {
        b->setPlayerOnMove(chess::BLACK);
    } else {
        b->setPlayerOnMove(chess::WHITE);
    }

    if (strs[2] == "----" || strs[2] == "--") {
        binfo.castle = false;
    } else {
        binfo.castle = true;
        for (shared_ptr<AbstractPiece> r: rooks) {
            shared_ptr<AbstractPiece> king = r->getColor() == WHITE ? whiteKing : blackKing;
            if (r->getPosition().x != 1 || r->getPosition().x != binfo.x || r->getPosition().y != 1
                || r->getPosition().y != binfo.y) {
                r->setHasMoved(true);
            } else if (r->getPosition().x > king->getPosition().x) {
                if (strs[2].find(r->getColor() == WHITE ? toupper('k') : 'k') != string::npos) {
                    r->setHasMoved(true);
                }
            } else if (r->getPosition().x < king->getPosition().x) {
                if (strs[2].find(r->getColor() == WHITE ? toupper('q') : 'q') != string::npos) {
                    r->setHasMoved(true);
                }
            }
        }
    }

    if (strs[3] != "-") {
        Square s2 = chess::stringToCoord(strs[3]);
        binfo.xEnpass = s2.x;
        binfo.yEnpass = s2.y;
    }
    return binfo;
}

shared_ptr<AbstractPiece>
FenBoardFactory::createPiece(char &c, int x, int y, GTLib2::domains::KriegspielState *b) {
    char pieceChar = tolower(c);
    Square s(x, y);
    switch (pieceChar) {
        case 'r':
            return make_shared<Rook>(static_cast<pieceName>(pieceChar),
                                     isupper(c) ? chess::WHITE : chess::BLACK,
                                     s,
                                     b);
        case 'q':
            return make_shared<Queen>(static_cast<pieceName>(pieceChar),
                                      isupper(c) ? chess::WHITE : chess::BLACK,
                                      s,
                                      b);
        case 'k':
            return make_shared<King>(static_cast<pieceName>(pieceChar),
                                     isupper(c) ? chess::WHITE : chess::BLACK,
                                     s,
                                     b);
        case 'n':
            return make_shared<Knight>(static_cast<pieceName>(pieceChar),
                                       isupper(c) ? chess::WHITE : chess::BLACK,
                                       s,
                                       b);
        case 'b':
            return make_shared<Bishop>(static_cast<pieceName>(pieceChar),
                                       isupper(c) ? chess::WHITE : chess::BLACK,
                                       s,
                                       b);
        case 'p': {
            auto p = make_shared<Pawn>(static_cast<pieceName>(pieceChar),
                                       isupper(c) ? chess::WHITE : chess::BLACK,
                                       s,
                                       b,
                                       pawnid_);
            pawnid_ += 2;
            return p;
        }
        default:
            unreachable("unrecognized option!");
    }
}

string FenBoardFactory::createFEN(const GTLib2::domains::KriegspielState *b) {
    string FEN = "";
    for (int i = b->getYSize(); i > 0; i--) {
        int freespaces = 0;
        for (int j = 1; j <= b->getXSize(); j++) {
            Square pos(j, i);
            shared_ptr<AbstractPiece> piece = b->getPieceOnCoords(pos);
            if (piece == nullptr) {
                freespaces++;
            } else {
                if (freespaces > 0) {
                    FEN += to_string(freespaces);
                }
                FEN += piece->toString();
                freespaces = 0;
            }
        }
        if (freespaces > 0) {
            FEN += to_string(freespaces);
        }

        if (i > 1) {
            FEN += "/";
        } else {
            FEN += " ";
        }
    }

    FEN += b->getPlayers()[0] == chess::WHITE ? "w " : "b ";

    if (b->canCastle()) {
        for (int i = chess::WHITE; i <= chess::BLACK; i++) {
            shared_ptr<AbstractPiece> king = b->getPiecesOfColorAndKind(i, KING)[0];
            vector<shared_ptr<AbstractPiece>> rooks = b->getPiecesOfColorAndKind(i, ROOK);

            if (rooks.size() == 2) {
                AbstractPiece *shortRook = nullptr;
                AbstractPiece *longRook = nullptr;

                for (shared_ptr<AbstractPiece> &r: rooks) {
                    shortRook = !r->hasMoved() && r->getPosition().x == 8 ? r.get() : shortRook;
                    longRook = !r->hasMoved() && r->getPosition().x == 1 ? r.get() : longRook;
                }

                if (!king->hasMoved()) {
                    if (shortRook != nullptr) {
                        FEN += i == chess::WHITE ? "K" : "k";
                    } else {
                        FEN += "-";
                    }

                    if (longRook != nullptr) {
                        FEN += i == chess::WHITE ? "Q" : "q";
                    } else {
                        FEN += "-";
                    }
                } else {
                    FEN += "--";
                }
            } else {
                if (!king->hasMoved() && !rooks[0]->hasMoved()) {
                    FEN += king->toString();
                } else {
                    FEN += "--";
                }
            }
        }
        FEN += " ";
    } else {
        FEN += "---- ";
    }

    if (!(b->getEnPassantSquare() == Square(-1, -1))) {
        FEN += chess::coordToString(b->getEnPassantSquare()) + " ";
    } else {
        FEN += "- ";
    }

    return FEN;
}
}

