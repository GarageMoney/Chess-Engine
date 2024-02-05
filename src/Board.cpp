#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <omp.h>
#include "Board.h"
#include "Table.h"
typedef uint64_t U64;
using namespace std;

Board::Board() : moveIndex(-1), materialValue(0) {
    whiteTurn = true;

    //initialize occupancy boards
    whiteOccupancy = 
        whitePawn | whiteKnight | whiteBishop | 
        whiteRook | whiteQueen | whiteKing;
    blackOccupancy = 
        blackPawn | blackKnight | blackBishop | 
        blackRook | blackQueen | blackKing;

    movedKing.first = false; movedKing.second = false;
    movedRookA.first = false; movedRookA.second = false;
    movedRookH.first = false; movedRookH.second = false;
    vector<bool> moved = {false, false, false, false, false, false};

    History.push_back(Board::Move(
        whitePawn, whiteKnight, whiteBishop, whiteRook, whiteQueen, whiteKing, 
        blackPawn, blackKnight, blackBishop, blackRook, blackQueen, blackKing,
        whiteOccupancy, blackOccupancy, materialValue, moved));
    moveIndex++;
    //initialize bit masks
    for(int i = 0; i < 64; i++) {
        bitMask1[i] = 1ULL << i;
        bitMask0[i] = ~bitMask1[i];
    }

    //precompute and store attack boards
    generateAllAttacks();

}

void Board::printU64(U64 ull) { //utility
    for(int rank = 7; rank >= 0; rank--) {
        for(int file = 7; file >= 0; file--) {
            int index = rank * 8 + file;
            U64 mask = 1ULL << index;
            char piece = (ull & mask) ? '1' : '0';
            cout << piece << " ";
        }
        cout << endl;
    }
}

int Board::getPiece(int square) const {
    U64 mask = bitMask1[square];
    if(mask & whiteOccupancy) {
        if(mask & whitePawn) return 6;
        else if(mask & whiteKnight) return 5;
        else if(mask & whiteBishop) return 4;
        else if(mask & whiteRook) return 3;
        else if(mask & whiteQueen) return 2;
        return 1;
    }
    else if(mask & blackOccupancy) {
        if(mask & blackPawn) return -6;
        else if(mask & blackKnight) return -5;
        else if(mask & blackBishop) return -4;
        else if(mask & blackRook) return -3;
        else if(mask & blackQueen) return -2;
        return -1;
    }
    return 0;
}

U64 Board::getRawMoves(int square) {
    U64 Moves = 0ULL;
    if(square < 0 || square > 63) return Moves;
    int id = getPiece(square);
    if(id == 0) return Moves;
    
    U64 allOccupancy;
    U64 relevantOccupancy;
    U64 allComplement;
    int magicIndex;
    //retrieve attack board for piece at given square 
    switch(id) {
        case 1: //whiteKing
            Moves = kingAttack[square] & ~whiteOccupancy; break;
        case 2: //whiteQueen
            allOccupancy = whiteOccupancy | blackOccupancy;
            relevantOccupancy = allOccupancy & rookAttack[square][0];
            magicIndex = rookIndex[square][relevantOccupancy];
            Moves = rookAttack[square][magicIndex] & ~whiteOccupancy;
            relevantOccupancy = allOccupancy & bishopAttack[square][0];
            magicIndex = bishopIndex[square][relevantOccupancy];
            Moves |= bishopAttack[square][magicIndex] & ~whiteOccupancy;
            break;
        case 3: //whiteRook
            allOccupancy = whiteOccupancy | blackOccupancy;
            relevantOccupancy = allOccupancy & rookAttack[square][0];
            magicIndex = rookIndex[square][relevantOccupancy];
            Moves = rookAttack[square][magicIndex] & ~whiteOccupancy;
            break;
        case 4: //whiteBishop
            allOccupancy = whiteOccupancy | blackOccupancy;
            relevantOccupancy = allOccupancy & bishopAttack[square][0];
            magicIndex = bishopIndex[square][relevantOccupancy];
            Moves = bishopAttack[square][magicIndex] & ~whiteOccupancy;
            break;
        case 5: //whiteKnight
            Moves = knightAttack[square] & ~whiteOccupancy;
            break;
        case 6: //whitePawn
            allComplement = ~(whiteOccupancy | blackOccupancy);
            Moves = bitMask1[square + 8] & allComplement;
            if(8 <= square && square <= 15 && Moves)
                Moves |= bitMask1[square + 16] & allComplement;
            Moves |= whitePawnAttack[square] & blackOccupancy;
            break;
        case -1: //blackKing
            Moves = kingAttack[square] & ~blackOccupancy;
            break;
        case -2: //blackQueen
            allOccupancy = whiteOccupancy | blackOccupancy;
            relevantOccupancy = allOccupancy & rookAttack[square][0];
            magicIndex = rookIndex[square][relevantOccupancy];
            Moves = rookAttack[square][magicIndex] & ~blackOccupancy;
            relevantOccupancy = allOccupancy & bishopAttack[square][0];
            magicIndex = bishopIndex[square][relevantOccupancy];
            Moves |= bishopAttack[square][magicIndex] & ~blackOccupancy;
            break;
        case -3: //blackRook
            allOccupancy = whiteOccupancy | blackOccupancy;
            relevantOccupancy = allOccupancy & rookAttack[square][0];
            magicIndex = rookIndex[square][relevantOccupancy];
            Moves = rookAttack[square][magicIndex] & ~blackOccupancy;
            break;
        case -4: //blackBishop
            allOccupancy = whiteOccupancy | blackOccupancy;
            relevantOccupancy = allOccupancy & bishopAttack[square][0];
            magicIndex = bishopIndex[square][relevantOccupancy];
            Moves = bishopAttack[square][magicIndex] & ~blackOccupancy;
            break;
        case -5: //blackKnight
            Moves = knightAttack[square] & ~blackOccupancy;
            break;
        case -6: //blackPawn
            allComplement = ~(whiteOccupancy | blackOccupancy);
            Moves = bitMask1[square - 8] & allComplement;
            if (47 <= square && square <= 55 && Moves)
                Moves |= bitMask1[square - 16] & allComplement;
            Moves |= blackPawnAttack[square] & whiteOccupancy;
            break;
    }

    return Moves;
}

U64 Board::getSpecialMoves(int square) {
    U64 Moves = 0ULL;
    if(square < 0 || square > 63) 
        return Moves;

    int id = getPiece(square);
    switch(id) {
        case 1: //castling
            if(!movedKing.first && !isCheck(1)) {
                if(!movedRookA.first && getPiece(7) == 3) {
                    if(getPiece(6) == 0 && getPiece(5) == 0 && 
                       getPiece(4) == 0 && !isAttacked(6, 0) && 
                      !isAttacked(5, 0) && !isAttacked(4, 0))
                        Moves |= bitMask1[5];
                }
                if(!movedRookH.first && getPiece(0) == 3) {
                    if(getPiece(2) == 0 && getPiece(1) == 0 &&
                      !isAttacked(2, 0) && !isAttacked(1, 0)) 
                        Moves |= bitMask1[1];
                }
            }
            break;
        case -1: //castling
            if(!movedKing.second && !isCheck(0)) {
                if(!movedRookA.second && getPiece(63) == -3) { 
                    if(getPiece(62) == 0 && getPiece(61) == 0 && 
                       getPiece(60) == 0 && !isAttacked(62, 1) && 
                      !isAttacked(61, 1) && !isAttacked(60, 1))
                        Moves |= bitMask1[61];
                }
                if(!movedRookH.second && getPiece(56) == -3) {
                    if(getPiece(58) == 0 && getPiece(57) == 0 &&
                      !isAttacked(58, 1) && !isAttacked(57, 1)) 
                        Moves |= bitMask1[57];
                }
            }
            break;
        case 6: //passant
            if(square == 32) {
                if(blackPawn & bitMask1[33]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.bPawn & bitMask1[33]))
                        Moves |= bitMask1[41];
                }
            }
            else if(33 <= square && square <= 38) {
                if(blackPawn & bitMask1[square - 1]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.bPawn & bitMask1[square - 1]))
                        Moves |= bitMask1[(square - 1) + 8];
                }
                else if(blackPawn & bitMask1[square + 1]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.bPawn & bitMask1[square + 1]))
                        Moves |= bitMask1[(square + 1) + 8];
                }
            }
            else if(square == 39) {
                if(blackPawn & bitMask1[38]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.bPawn & bitMask1[38]))
                        Moves |= bitMask1[46];
                }
            }
            break;
        case -6: //passant
            if(square == 24) {
                if(whitePawn & bitMask1[25]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.wPawn & bitMask1[25]))
                        Moves |= bitMask1[17];
                }
            }
            else if(25 <= square && square <= 30) {
                if(whitePawn & bitMask1[square - 1]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.wPawn & bitMask1[square - 1]))
                        Moves |= bitMask1[(square - 1) - 8];
                }
                else if(whitePawn & bitMask1[square + 1]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.wPawn & bitMask1[square + 1]))
                        Moves |= bitMask1[(square + 1) - 8];
                }
            }
            else if(square == 31) {
                if(whitePawn & bitMask1[30]) {
                    Move prev = History.at(moveIndex - 1);
                    if(!(prev.wPawn & bitMask1[30]))
                        Moves |= bitMask1[22];
                }
            }
            break;
        default: break;
    }

    return Moves;
}

bool Board::isCheck(bool white) {
    if(white) {
        for(int square = 0; square < 64; square++) 
            if(bitMask1[square] & blackOccupancy) {
                U64 blackAttack = getRawMoves(square);
                if(whiteKing & blackAttack) return true;
            }
    }
    else {
        for(int square = 0; square < 64; square++) 
            if(bitMask1[square] & whiteOccupancy) {
                U64 whiteAttack = getRawMoves(square);
                if(blackKing & whiteAttack) return true;
            }
    }
    return false;
}

U64 Board::getValidMoves(int square) { 
    if(square < 0 || square > 63) return 0ULL;
    int id = getPiece(square);
    //filter legal moves based on check status
    U64 Moves = getRawMoves(square) | 
            getSpecialMoves(square);
    for(int i = 0; i < 64; i++) {
        if(bitMask1[i] & Moves) { 
            makeMove(square, i);
            if(isCheck(id > 0)) 
                Moves &= bitMask0[i];
            undoMove(); History.pop_back();
        }
    }
    return Moves;
}

bool Board::isAttacked(int square, bool byWhite) {
    if(square < 0 || square > 63) return false;
    if(byWhite) { 
        for(int i = 0; i < 64; i++) {
            if(bitMask1[i] & whiteOccupancy) {
                U64 Moves = getRawMoves(i);
                if(bitMask1[square] & Moves)
                    return true;
            }
        }
    }
    else {
        for(int i = 0; i < 64; i++) {
            if(bitMask1[i] & blackOccupancy) {
                U64 Moves = getRawMoves(i);
                if(bitMask1[square] & Moves)
                    return true;
            }
        }
    }
    return false;
}

bool Board::makeMove(int square1, int square2) {
    //check out-of-bounds
    if(square1 < 0 || square1 > 63 || 
       square2 < 0 || square2 > 63)
        return false;

    int idStart = getPiece(square1);
    int idEnd = getPiece(square2);
    if(idStart == 0)
        return false;

    int back = History.size() - 1 - moveIndex;
    for(int i = 0; i < back; i++)
        History.pop_back();

    U64 maskStart0 = bitMask0[square1];
    U64 maskEnd1 = bitMask1[square2];

    //handle if capture piece exists
    if(idEnd != 0) {
        U64 maskEnd0 = bitMask0[square2];
        U64& occupancy = (idEnd > 0) ? 
            whiteOccupancy : blackOccupancy;
        occupancy &= maskEnd0;

        switch(idEnd) {
            case 1: whiteKing &= maskEnd0; break;
            case 2: whiteQueen &= maskEnd0; break;
            case 3: whiteRook &= maskEnd0; break;
            case 4: whiteBishop &= maskEnd0; break;
            case 5: whiteKnight &= maskEnd0; break;
            case 6: whitePawn &= maskEnd0; break;
            case -1: blackKing &= maskEnd0; break;
            case -2: blackQueen &= maskEnd0; break;
            case -3: blackRook &= maskEnd0; break;
            case -4: blackBishop &= maskEnd0; break;
            case -5: blackKnight &= maskEnd0; break;
            case -6: blackPawn &= maskEnd0; break;
        }
        
        if(idEnd > 0) materialValue -= pieceValue[idEnd];
        else materialValue += pieceValue[abs(idEnd)];
    }

    U64& occupancy = (idStart > 0) ? 
        whiteOccupancy : blackOccupancy;
    occupancy &= maskStart0;
    occupancy |= maskEnd1;
    
    //handle movement
    switch(idStart) {
        case 1: 
            whiteKing &= maskStart0; 
            whiteKing |= maskEnd1; 
            movedKing.first = true;
            //handle castled rook
            if(square1 == 3) {
                if(square2 == 1) {
                    occupancy &= bitMask0[0];
                    occupancy |= bitMask1[2];
                    whiteRook &= bitMask0[0];
                    whiteRook |= bitMask1[2];
                }
                else if(square2 == 5) {
                    occupancy &= bitMask0[7];
                    occupancy |= bitMask1[4];
                    whiteRook &= bitMask0[7];
                    whiteRook |= bitMask1[4];
                }
            }
            break;
        case 2: whiteQueen &= maskStart0; 
                whiteQueen |= maskEnd1; break;
        case 3: 
            whiteRook &= maskStart0; 
            whiteRook |= maskEnd1; 
            if(square1 == 0) movedRookH.first = true;
            else if(square1 == 7) movedRookA.first = true;
            break;
        case 4: whiteBishop &= maskStart0; 
                whiteBishop |= maskEnd1; break;
        case 5: whiteKnight &= maskStart0; 
                whiteKnight |= maskEnd1; break;
        case 6: 
            whitePawn &= maskStart0;        
            //handle promotion
            if(56 <= square2 && square2 <= 63) {
                whiteQueen |= maskEnd1;
                materialValue += 800;
            } 
            else whitePawn |= maskEnd1;
            //handle passant
            if(idEnd == 0 && (square2 - square1) % 8 != 0) {
                blackOccupancy &= bitMask0[square2 - 8];
                blackPawn &= bitMask0[square2 - 8];
                materialValue += 100;
            }
            break;
        case -1: 
            blackKing &= maskStart0; 
            blackKing |= maskEnd1; 
            movedKing.second = true;
            //handle castled rook
            if(square1 == 59) {
                if(square2 == 57) {
                    occupancy &= bitMask0[56];
                    occupancy |= bitMask1[58];
                    blackRook &= bitMask0[56]; 
                    blackRook |= bitMask1[58];
                }
                else if(square2 == 61) {
                    occupancy &= bitMask0[63];
                    occupancy |= bitMask1[60];
                    blackRook &= bitMask0[63];
                    blackRook |= bitMask1[60];
                }
            }
            break;
        case -2: blackQueen &= maskStart0; 
                 blackQueen |= maskEnd1; break;
        case -3: 
            blackRook &= maskStart0; 
            blackRook |= maskEnd1; 
            if(square1 == 56) movedRookH.second = true;
            else if(square1 == 63) movedRookA.second = true;
            break;
        case -4: blackBishop &= maskStart0; 
                 blackBishop |= maskEnd1; break;
        case -5: blackKnight &= maskStart0; 
                 blackKnight |= maskEnd1; break;
        case -6: 
            blackPawn &= maskStart0;
            //handle promotion  
            if(0 <= square2 && square2 <= 7) { 
                blackQueen |= maskEnd1;
                materialValue -= 800;
            } 
            else blackPawn |= maskEnd1; 
            //handle passant
            if(idEnd == 0 && (square1 - square2) % 8 != 0) {
                whiteOccupancy &= bitMask0[square2 + 8];
                whitePawn &= bitMask0[square2 + 8];
                materialValue -= 100;
            }
            break;
    }

    // recording history
    vector<bool> moved = {
        movedKing.first, movedKing.second, 
        movedRookA.first, movedRookA.second, 
        movedRookH.first, movedRookH.second};

    History.push_back(Board::Move(
        whitePawn, whiteKnight, whiteBishop, whiteRook, whiteQueen, whiteKing, 
        blackPawn, blackKnight, blackBishop, blackRook, blackQueen, blackKing,
        whiteOccupancy, blackOccupancy, materialValue, moved));
    moveIndex++;

    whiteTurn = !whiteTurn;
    return true;
}

int Board::getTotalMoves() {return History.size();}
int Board::getMoveIndex() {return moveIndex;}

bool Board::undoMove() { //reverse of makeMove() 
    if(History.size() < 2) return false;
    if(moveIndex < 1) return false;
    moveIndex--;
    Move prev = History.at(moveIndex);

    whiteOccupancy = prev.wOccupancy;
    whitePawn = prev.wPawn;
    whiteKnight = prev.wKnight;
    whiteBishop = prev.wBishop;
    whiteRook = prev.wRook;
    whiteQueen = prev.wQueen;
    whiteKing = prev.wKing;
    blackOccupancy = prev.bOccupancy;
    blackPawn = prev.bPawn;
    blackKnight = prev.bKnight;
    blackBishop = prev.bBishop;
    blackRook = prev.bRook;
    blackQueen = prev.bQueen;
    blackKing = prev.bKing;
    
    movedKing.first = prev.moved[0]; 
    movedKing.second = prev.moved[1];
    movedRookA.first = prev.moved[2];
    movedRookA.second = prev.moved[3];
    movedRookH.first = prev.moved[4];
    movedRookH.second = prev.moved[5];

    materialValue = prev.currScore;
 
    whiteTurn = !whiteTurn;
    return true;
}

bool Board::redoMove() { //similar to makeMove()
    if(History.size() <= 1) return false;
    if(moveIndex == History.size() - 1) 
        return false;
    moveIndex++;
    Move next = History.at(moveIndex);

    materialValue = next.currScore;

    movedKing.first = next.moved[0]; 
    movedKing.second = next.moved[1];
    movedRookA.first = next.moved[2];
    movedRookA.second = next.moved[3];
    movedRookH.first = next.moved[4];
    movedRookH.second = next.moved[5];

    whiteOccupancy = next.wOccupancy;
    whitePawn = next.wPawn;
    whiteKnight = next.wKnight;
    whiteBishop = next.wBishop;
    whiteRook = next.wRook;
    whiteQueen = next.wQueen;
    whiteKing = next.wKing;
    blackOccupancy = next.bOccupancy;
    blackPawn = next.bPawn;
    blackKnight = next.bKnight;
    blackBishop = next.bBishop;
    blackRook = next.bRook;
    blackQueen = next.bQueen;
    blackKing = next.bKing;

    whiteTurn = !whiteTurn;
    return true;
}

bool Board::isCheckmate(bool white) {
    if(!isCheck(white)) return false;
    U64 occupancy = white ? 
    whiteOccupancy : blackOccupancy;
    for(int i = 0; i < 64; i++)
        if(bitMask1[i] & occupancy) {
            if(getValidMoves(i))
                return false;
        }
    return true;
}

bool Board::isStalemate() {
    if(whiteTurn && hasNoMoves(true) || 
      !whiteTurn && hasNoMoves(false))
        return true;
    U64 QueenRookPawn = 
        whiteQueen | blackQueen | 
        whiteRook | blackRook | 
        whitePawn | blackPawn;
    if(!QueenRookPawn) {
        U64 Knights = whiteKnight | blackKnight;
        U64 Bishops = whiteBishop | blackBishop;
        if(!Knights && !Bishops ||
           !Knights && __builtin_popcount(Bishops) == 1 ||
           !Bishops && __builtin_popcount(Knights) == 1) 
            return true;
    }
    return false;
}

bool Board::hasNoMoves(bool white) {
    if(isCheck(white)) return false;
    U64 occupancy = white ?
    whiteOccupancy : blackOccupancy;
    for(int i = 0; i < 64; i++) 
        if(bitMask1[i] & occupancy) {
            if(getValidMoves(i))
                return false;
        }
    return true;
}

//engine controls
vector<pair<int, int>> Board::getAllMoves(bool maxPlayer) { 
    vector<pair<int, int>> allMoves;
    U64 occupancy = maxPlayer ? 
    whiteOccupancy : blackOccupancy;
    for(int i = 0; i < 64; i++) 
        if(bitMask1[i] & occupancy) {
            U64 moves = getValidMoves(i);
            for(int j = 0; j < 64; j++) 
                if(bitMask1[j] & moves) 
                    allMoves.push_back({i, j});
        }
    return allMoves;
}

pair<int, int> Board::getRandomMove(bool maxPlayer) {
    vector<pair<int, int>> allMoves = getAllMoves(maxPlayer);
    return allMoves[rand() % allMoves.size()];
}

pair<int, int> Board::getBestMove(bool maxPlayer) {
    int bestScore = maxPlayer ? -100000 : 100000;
    pair<int, int> bestMove;

    for(const auto& move : getAllMoves(maxPlayer)) {
        makeMove(move.first, move.second);
        int score = minimax(0, -100000, 100000, !maxPlayer);
        undoMove(); History.pop_back();

        if(maxPlayer && score > bestScore || 
          !maxPlayer && score < bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    return bestMove;
}

int Board::minimax(int depth, int alpha, int beta, bool maxPlayer) {
    if(depth == 4 - 1 || 
       isCheckmate(maxPlayer) || 
       isCheckmate(!maxPlayer) || 
       isStalemate()) {
        return evaluate();
    }

    if(maxPlayer) {
        int bestScore = -100000;
        for(const auto& move : getAllMoves(maxPlayer)) {
            makeMove(move.first, move.second);
            int score = minimax(depth + 1, alpha, beta, 0);
            undoMove(); History.pop_back();

            bestScore = max(bestScore, score);
            alpha = max(alpha, bestScore);
            if(beta <= alpha) {
                return bestScore;
            }
        }
        return bestScore;
    } 
    else {
        int bestScore = 100000;
        for(const auto& move : getAllMoves(maxPlayer)) {
            makeMove(move.first, move.second);
            int score = minimax(depth + 1, alpha, beta, 1);
            undoMove(); History.pop_back();

            bestScore = min(bestScore, score);
            beta = min(beta, bestScore);
            if(beta <= alpha) {
                return bestScore;
            }
        }
        return bestScore;
    }
}

int Board::evaluate() {
    int score = 0;
    score += evaluateMaterial();
    score += evaluatePosition();
    return score;
}

int Board::evaluateMaterial() {
    return materialValue;
}

int Board::evaluatePosition() {
    int score = 0;
    for(int row = 0; row < 8; row++) {
        for(int col = 0; col < 8; col++) {
            int square = row * 8 + col;
            U64 squareMask = bitMask1[square];
            if(squareMask & ~(whiteOccupancy | blackOccupancy)) continue;
            if(squareMask & whitePawn) score += pawnTable[7 - row][col];
            else if(squareMask & whiteKnight) score += knightTable[7 - row][col];
            else if(squareMask & whiteBishop) score += bishopTable[7 - row][col];
            else if(squareMask & whiteRook) score += rookTable[7 - row][col];
            else if(squareMask & whiteQueen) score += queenTable[7 - row][col];
            else if(squareMask & whiteKing) score += kingTable[7 - row][col];
            else if(squareMask & blackPawn) score -= pawnTable[row][col];
            else if(squareMask & blackKnight) score -= knightTable[row][col];
            else if(squareMask & blackBishop) score -= bishopTable[row][col];
            else if(squareMask & blackRook) score -= rookTable[row][col];
            else if(squareMask & blackQueen) score -= queenTable[row][col];
            else if(squareMask & blackKing) score -= kingTable[row][col];
        }
    }
    return score;
}

//initializer
void Board::generateAllAttacks() {
    //whitePawnAttack:
    int whitePawn[3][2] = {
        {1, -1}, {1, 1}
    };
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            U64 curr = 0;
            for (int k = 0; k < 2; k++) {
                int eRow = i + whitePawn[k][0];
                int eCol = j + whitePawn[k][1];
                if(eRow >= 0 && eRow <= 7 && 
                   eCol >= 0 && eCol <= 7) {
                    int index = eRow * 8 + eCol;
                    curr |= bitMask1[index];
                }
            }
            whitePawnAttack[i * 8 + j] = curr;
        }
    }

    //blackPawnAttack:
    int blackPawn[3][2] = {
        {-1, -1}, {-1, 1}
    };
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            U64 curr = 0;
            for(int k = 0; k < 2; k++) {
                int eRow = i + blackPawn[k][0];
                int eCol = j + blackPawn[k][1];
                if(eRow >= 0 && eRow <= 7 && 
                   eCol >= 0 && eCol <= 7) {
                    int index = eRow * 8 + eCol;
                    curr |= bitMask1[index];
                }
            }
            blackPawnAttack[i * 8 + j] = curr;
        }
    }

    //knightAttack:
    int knight[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            U64 curr = 0;
            for(int k = 0; k < 8; k++) {
                int eRow = i + knight[k][0];
                int eCol = j + knight[k][1];            
                if(eRow >= 0 && eRow <= 7 && 
                   eCol >= 0 && eCol <= 7) {
                    int index = eRow * 8 + eCol;
                    curr |= bitMask1[index];
                }
            }
            knightAttack[i * 8 + j] = curr;
        }
    }

    //bishopAttack
    int bishop[4][2] = {
        {1, 1}, {-1, -1}, 
        {1, -1}, {-1, 1}
    };
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            U64 curr = 0;
            for(int k = 0; k < 4; k++) {
                int mult = 1;
                int eRow = i + bishop[k][0];
                int eCol = j + bishop[k][1];
                while(eRow >= 0 && eRow <= 7 && 
                      eCol >= 0 && eCol <= 7) {
                    int index = eRow * 8 + eCol;
                    curr |= bitMask1[index];
                    mult++;
                    eRow = i + mult * bishop[k][0];
                    eCol = j + mult * bishop[k][1];
                }
            }
            
            unordered_map<U64, int> map;
            int numBits = __builtin_popcountll(curr);
            int combinations = 1 << numBits;
            for(int perm = 0; perm < combinations; perm++) {
                U64 moves = 0;
                U64 occupancyBoard = setOccupancy(
                    perm, numBits, curr);
                map.insert(make_pair(occupancyBoard, perm));
                for(int k = 0; k < 4; k++) {
                    int mult = 1;
                    int eRow = i + bishop[k][0];
                    int eCol = j + bishop[k][1];
                    while(eRow >= 0 && eRow <= 7 && 
                        eCol >= 0 && eCol <= 7) {
                        int index = eRow * 8 + eCol;
                        moves |= bitMask1[index];
                        mult++;
                        eRow = i + mult * bishop[k][0];
                        eCol = j + mult * bishop[k][1];
                        if(bitMask1[index] & occupancyBoard) 
                            break;
                    }
                }

                bishopAttack[i * 8 + j][perm] = moves;
            }
            bishopIndex.push_back(map);
        }
    }

    //rookAttack
    int rook[4][2] = {
        {1, 0}, {-1, 0}, 
        {0, -1}, {0, 1}
    };
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            U64 curr = 0;
            for (int k = 0; k < 4; k++) {
                int mult = 1;
                int eRow = i + rook[k][0];
                int eCol = j + rook[k][1];
                while(eRow >= 0 && eRow <= 7 && 
                      eCol >= 0 && eCol <= 7) {
                    int index = eRow * 8 + eCol;
                    curr |= bitMask1[index];
                    mult++;
                    eRow = i + mult * rook[k][0];
                    eCol = j + mult * rook[k][1];
                }
            }
            unordered_map<U64, int> map;
            int numBits = __builtin_popcountll(curr); 
            int combinations = 1 << numBits;
            for(int perm = 0; perm < combinations; perm++) {
                U64 moves = 0;
                U64 occupancyBoard = setOccupancy(
                    perm, numBits, curr);
                map.insert(make_pair(occupancyBoard, perm));
                for(int k = 0; k < 4; k++) {
                    int mult = 1;
                    int eRow = i + rook[k][0];
                    int eCol = j + rook[k][1];
                    while(eRow >= 0 && eRow <= 7 && 
                        eCol >= 0 && eCol <= 7) {
                        int index = eRow * 8 + eCol;
                        moves |= bitMask1[index];
                        mult++;
                        eRow = i + mult * rook[k][0];
                        eCol = j + mult * rook[k][1];
                        if(bitMask1[index] & occupancyBoard) 
                            break;
                    }
                }

                rookAttack[i * 8 + j][perm] = moves;
            }
            rookIndex.push_back(map);
        }
    }

    //kingAttack
    int king[8][2] = {
        {1, 1}, {-1, -1}, {1, -1}, {-1, 1},
        {1, 0}, {-1, 0}, {0, -1}, {0, 1}
    };
    #pragma omp parallel for collapse(2)
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            U64 curr = 0;
            for (int k = 0; k < 8; k++) {
                int eRow = i + king[k][0];
                int eCol = j + king[k][1];
                if(eRow >= 0 && eRow <= 7 && 
                      eCol >= 0 && eCol <= 7) {
                    int index = eRow * 8 + eCol;
                    curr |= bitMask1[index];
                }
            }
            kingAttack[i * 8 + j] = curr;
        }
    }
}

//chess programming wiki
U64 Board::setOccupancy(int index, int numBits, U64 attackMask) {
    U64 occupancy = 0ULL;
    for(int count = 0; count < numBits; count++) {
        int square = 0;
        for(int i = 63; i >= 0; i--) {
            if(bitMask1[i] & attackMask) {
                square = i;
                break;
            }
        }
        attackMask &= bitMask0[square];
        if(index & (1 << count)) 
            occupancy |= (1ULL << square);

    }
    return occupancy;
}