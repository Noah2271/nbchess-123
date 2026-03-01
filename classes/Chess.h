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

    std::vector<BitMove> generateMoves(char color);
    // debug testers
    void printAllBoards();
    // actual functions
    void setUpBoard() override;
    void generatePawnMoves(std::vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex);
    void generateKnightMoves(std::vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex);
    void generateKingMoves(std::vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex);
    void generateSlidingMoves(std::vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex, int offsets[][2], int numOffsets, int pieceType);
    void addMove(const char *state, std::vector<BitMove>&moves, int fromRow, int fromCol, int toRow, int toCol);
    virtual void bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst);

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

    // =================================================================
    // move calculator for all pieces
    // takes the move specified by the callable parameter getMove and the piece at the selected index.
    // if the piece is a pawn, switchcase to perform special capture and movement logic.
    // otherwise go to case for other pieces that just checks if the destination is empty or has an enemy piece before
    // adding the move to the bitMove vector
    // =================================================================
    template<typename Getter>
    void calculateMoves(const char *state, std::vector<BitMove>&moves, int row, int rowOffSet, int col, int colOffSet, int colorInt, Getter getMove)
        {
            char target = getMove();
            char piece = state[row * 8 + col];
            switch(toupper(static_cast<unsigned char>(piece))){
                case 'P': 
                    if(colOffSet == 0 && target == '0'){
                        addMove(state, moves, row, col, row + rowOffSet, col + colOffSet);
                        break;
                    }
                    if(colOffSet != 0 && target != '0'){
                        addMove(state, moves, row, col, row + rowOffSet, col + colOffSet);
                        break;
                    }
                    break;
                case 'N' : case 'K' : case 'R' : case 'Q' : case 'B' :
                    if (target == '0' || (target != '0' && ((colorInt == 1 && islower(static_cast<unsigned char>(target))) || (colorInt == -1 && isupper(static_cast<unsigned char>(target)))))){
                        addMove(state, moves, row, col, row + rowOffSet, col + colOffSet);
                    }
                    break;
            }
        }

private:
    Bit* PieceForPlayer(const int playerNumber, ChessPiece piece);
    Player* ownerAt(int x, int y) const;
    void pieceSetFEN(int col, int row, char FENchar, ChessPiece type);
    void FENtoBoard(const std::string& fen);
    char pieceNotation(int x, int y) const;
    
    Grid* _grid;

    
};