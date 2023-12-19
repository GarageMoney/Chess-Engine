#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <unordered_map>
typedef uint64_t U64;
using namespace std;

class Board {
public:
    Board();
    void printU64(U64 ull); //utility
    int getPiece(int square) const; //get piece at given square
    U64 getRawMoves(int square); //get unfiltered moves of piece at given square
    U64 getSpecialMoves(int square); //get special moves of piece at given square
    bool isCheck(bool white); //determine whether given side is in check
    U64 getValidMoves(int square); //get filtered moves of piece at given square
    bool isAttacked(int square, bool byWhite); //determine whether given square is attacked by given color
    bool makeMove(int square1, int square2); //make move from square to square and record
        int getTotalMoves(); 
        int getMoveIndex();
        bool undoMove(); //undo move if moves exist
        bool redoMove(); //redo moves if moves exist
    bool isCheckmate(bool white); //determine whether given side is in checkmate
    bool isStalemate(); //determine whether current state is a stalemate
        bool hasNoMoves(bool white); //determine whether given side has no moves

    //engine controls
    vector<pair<int, int>> getAllMoves(bool maxPlayer); //find all moves available for given player
    pair<int, int> getRandomMove(bool maxPlayer); //get random move in all available moves
    pair<int, int> getBestMove(bool maxPlayer); //get best move in all available moves
    int minimax(int depth, int alpha, int beta, bool maxPlayer); //algorithm
    int evaluate(); //evaluate the current state
        int evaluateMaterial();
        int evaluatePosition();

    //initializer
    void generateAllAttacks(); //precompute attack boards
        U64 setOccupancy( //simulate all possible occupancies
            int index, 
            int numBits, 
            U64 attackMask);

private:
    bool whiteTurn; 

    //single masks
    U64 bitMask1[64]; //Single bit 1
    U64 bitMask0[64]; //Single bit 0

    //color & type-specific occupancy
    //initialized to starting chess positions
    U64 whitePawn = 0xFF00;
    U64 whiteKnight = 0x42;
    U64 whiteBishop = 0x24;
    U64 whiteRook = 0x81;
    U64 whiteQueen = 0x10;
    U64 whiteKing = 0x08;
    U64 blackPawn = 0x00FF000000000000;
    U64 blackKnight = 0x4200000000000000;
    U64 blackBishop = 0x2400000000000000;
    U64 blackRook = 0x8100000000000000;
    U64 blackQueen = 0x1000000000000000;
    U64 blackKing = 0x0800000000000000;

    //color-specific occupancy
    U64 whiteOccupancy;
    U64 blackOccupancy;

    //precomputed attack boards
    U64 whitePawnAttack[64];
    U64 blackPawnAttack[64];
    U64 knightAttack[64];
    U64 rookAttack[64][16384];
    U64 bishopAttack[64][8192];
    U64 kingAttack[64];

    //occupancy permutation to retrieval index maps
    vector<unordered_map<U64, int>> rookIndex; 
    vector<unordered_map<U64, int>> bishopIndex;

    //control flags
    pair<bool, bool> movedKing;
    pair<bool, bool> movedRookA;
    pair<bool, bool> movedRookH;

    //For undo-redo functionality:
    class Move {
    public:
        U64 wPawn, wKnight, wBishop, wRook, wQueen, wKing,
            bPawn, bKnight, bBishop, bRook, bQueen, bKing;
        U64 wOccupancy, bOccupancy;
        int currScore; vector<bool> moved;
        Move(U64 whPawn, U64 whKnight, U64 whBishop, U64 whRook, U64 whQueen, U64 whKing,
            U64 blPawn, U64 blKnight, U64 blBishop, U64 blRook, U64 blQueen, U64 blKing,
            U64 whOccupancy, U64 blOccupancy, int curr, vector<bool> Moved) : 
            wPawn(whPawn), wKnight(whKnight), wBishop(whBishop), wRook(whRook),
            wQueen(whQueen), wKing(whKing), bPawn(blPawn), bKnight(blKnight),
            bBishop(blBishop), bRook(blRook), bQueen(blQueen), bKing(blKing),
            wOccupancy(whOccupancy), bOccupancy(blOccupancy), 
            currScore(curr), moved(Moved) {}
    };
    vector<Move> History;
    int moveIndex;

    //engine members
    int materialValue;
    int pieceValue[6] = {
        20000, 900, 500, 330, 330, 100
    };
};

#endif //BOARD_H