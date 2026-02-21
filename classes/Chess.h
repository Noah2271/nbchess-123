#pragma once

#include "Game.h"
#include "Grid.h"
#include "Bitboard.h"

constexpr int pieceSize = 80;
enum PieceColor { EMPTY, WHITE, BLACK };
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    // added stuff
    void generatePawnMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorInt);
    void generateKnightMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorInt);
    void generateKingMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorInt);
    void addMove(const char *state, std::vector<BitMove>&moves, int fromRow, int fromCol, int toRow, int toCol);
    PieceColor stateColor(int col, int row);
    std::vector<BitMove> generateMoves(const char*state, char color);

    void setUpBoard() override;

    bool canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    bool actionForEmptyHolder(BitHolder &holder) override;

    void stopGame() override;

    Player *checkForWinner() override;
    bool checkForDraw() override;

    std::string initialStateString() override;
    std::string stateString() override;
    void setStateString(const std::string &s) override;
    

    Grid* getGrid() override { return _grid; }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void pieceSetFEN(int col, int row, char FENchar, ChessPiece type);
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;
    
    Grid* _grid;

    
};