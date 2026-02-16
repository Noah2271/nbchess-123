#include "Chess.h"
#include <limits>
#include <cmath>
#include <string>
#include <ctype.h>
#include <cctype>

using namespace std;

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    //numbers are empty space, lowercase is black, uppercase is white
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");



    startGame();
}
//
// FEN -> Board translator function
// draws pieces on the board based on FEN characters, starting at index 0, 0 on the top left and reading left to right until the
// fen string is processed entirely
void Chess::FENtoBoard(const std::string& fen) {
    int col = 0;
    int row = 0;
    signed int fenLength = fen.length();
    for(int i = 0; i < fenLength; i++){
        switch(fen[i]) {
            case 'r' : case 'R' :
                pieceSetFEN(col, row, fen[i], Rook);
                col++;
                break;
            case 'n' : case 'N' :
                pieceSetFEN(col, row, fen[i], Knight);
                col++;
                break;
            case 'b' : case 'B' :
                pieceSetFEN(col, row, fen[i], Bishop);
                col++;
                break;
            case 'q' : case 'Q' :
                pieceSetFEN(col, row, fen[i], Queen);
                col++;
                break;
            case 'k' : case 'K' :
                pieceSetFEN(col, row, fen[i], King);
                col++;
                break;
            case 'p' : case 'P' :
                pieceSetFEN(col, row, fen[i], Pawn);
                col++;
                break;
            case '1' : case '2' : case '3' : case '4' : case '5' : case '6' : case '7' : case '8':
                // subracting string literal 0 from numeric char converts to actual int value
                col -= (fen[i] - '0');
                break;
            case '/' :
                // new row so de-iterate row and reset column back to far right
                row++;
                col = 0;
                break;
            case ' ':
                // rest of fen string not yet implemented " w kQkq - 0 1"
                return; 
        }
    }
    return;
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
}

//
// FEN bit setter to mitigate redundant five lines of code per case
//
void Chess::pieceSetFEN(int col, int row, char FENchar, ChessPiece type){
    int playerNumber = isupper(FENchar) ? 1 : 0;
    Bit* piece = PieceForPlayer(playerNumber, type);
    ChessSquare* currSquare = _grid->getSquare(col, row);
    piece->setPosition(currSquare->getPosition());
    currSquare->setBit(piece);
}



/*
    // Enable only dark squares and place pieces
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        bool isDark = (x + y) % 2 == 1;
        _grid->setEnabled(x, y, isDark);

        if (isDark) {
            if (y < 3) {
                Bit* piece = createPiece(RED_PIECE);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            } else if (y > 4) {
                Bit* piece = createPiece(YELLOW_PIECE);
                piece->setPosition(square->getPosition());
                square->setBit(piece);
            }
        }
    });

    startGame();
}

*/

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if (pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return true;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}
