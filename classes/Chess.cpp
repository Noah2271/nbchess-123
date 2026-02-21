#include "Chess.h"
#include <limits>
#include <cmath>
#include <string>
#include <ctype.h>
#include <cctype>

using namespace std;

// ==============================================================
// constructors, destructors
// ==============================================================

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

// ==============================================================
// board setup
// draws a 8x8 grid and converts a FEN string read from left->right
// and draws pieces starting at index 0,0 on the bottom left of the
// board to the top right of the board
// * note that the state string reads from the bottom left to 
// top right
// ==============================================================

char Chess::pieceNotation(int x, int y) const {
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    // grab bit from square at x y
    Bit *bit = _grid->getSquare(x, y)-> bit();
    char notation = '0';
    if(bit){
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece) {
    const char* pieces[] =  { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();

    const char* pieceName = pieces[piece -1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber == 0 ? 1 : 0));
    bit->setSize(pieceSize, pieceSize);

    int tag = piece;
    if(playerNumber == 1) tag += 128;
    bit->setGameTag(tag);
    
    return bit;
}

void Chess::setUpBoard() {
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    // lower -> black | upper -> white
    FENtoBoard("RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr");

    startGame();
}

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
                col += (fen[i] - '0');
                break;
            case '/' :
                row++;
                col = 0;
                break;
            default :
                return; 
        }
    }
    return;
}

void Chess::pieceSetFEN(int col, int row, char FENchar, ChessPiece type ) {
    int playerNumber = isupper(FENchar) ? 0 : 1;
    Bit* piece = PieceForPlayer(playerNumber, type);
    ChessSquare* currSquare = _grid->getSquare(col, row);
    piece->setPosition(currSquare->getPosition());
    currSquare->setBit(piece);
}

// ==============================================================
// game state functions
// includes game state checks and move generation and handling
// for the player
// ==============================================================

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

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

std::string Chess::initialStateString() { return stateString(); }

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

// ==============================================================
// player action functions
// ==============================================================

bool Chess::actionForEmptyHolder(BitHolder &holder) {
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src) {
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    if(pieceColor == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    ChessSquare* srcSquare = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* dstSquare = dynamic_cast<ChessSquare*>(&dst);

    if(!srcSquare || !dstSquare) return false;

    string state = stateString();
    char color = (bit.gameTag() <  128) ? 'W' : 'B';
    auto moves = generateMoves(state.c_str(), color);

    int fromIndex = srcSquare->getRow() * 8 + srcSquare->getColumn();
    int toIndex = dstSquare->getRow() * 8 + dstSquare->getColumn();

    for(auto &move : moves){
        if(move.from == fromIndex && move.to == toIndex) return true;
    }
    return false;
}

PieceColor Chess::stateColor(int col, int row) {
    ChessSquare* square = _grid->getSquare(col, row);
    if (!square || !square->bit()) return EMPTY;
    return (square->bit()->gameTag() < 128) ? WHITE : BLACK;
}

void Chess::generatePawnMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorInt) {

    int direction = (colorInt == 1) ? 1 : -1;
    int startRow = (colorInt == 1) ? 1 : 6; 
    int nextRow = row + direction;
    char target;

    // forward moves
    if(row > 0 && row < 7){
        if (nextRow >= 0 && nextRow < 8 && state[nextRow * 8 + col] == '0') {
            addMove(state, moves, row, col, nextRow, col);

            int doubleRow = row + 2 * direction;
            if (row == startRow && state[doubleRow * 8 + col] == '0') {
                addMove(state, moves, row, col, doubleRow, col);
            }
        }

        // capture diagonals
        if (col > 0) {
            target = state[nextRow * 8 + (col - 1)];
            if (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target)))) {
                addMove(state, moves, row, col, nextRow, col - 1);
            }
        }
        if (col < 7) {
            target = state[nextRow * 8 + (col + 1)];
            if (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target)))) {
                addMove(state, moves, row, col, nextRow, col + 1);
            }
        }
    }
}

void Chess::generateKnightMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorInt) {
    
    char target;

    if (row > 0 && col < 6){
        target = state[(row - 1) * 8 + (col + 2)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row - 1, col + 2);
        }
    }

    if (row < 7 && col < 6){
        target = state[(row + 1) * 8 + (col + 2)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row + 1, col + 2);
        }
    }

    if (row < 7 && col > 1){
        target = state[(row + 1) * 8 + (col - 2)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row + 1, col - 2);
        }
    }

    if (row > 0 && col > 1){
        target = state[(row - 1) * 8 + (col - 2)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row - 1, col - 2);
        }
    }

    if (row < 6 && col > 0){
        target = state[(row + 2) * 8 + (col - 1)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row + 2, col - 1);
        }
    }
    
    if (row > 1 && col > 0){
        target = state[(row - 2) * 8 + (col - 1)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row - 2, col - 1);
        }
    }

    if (row < 6 && col < 7){
        target = state[(row + 2) * 8 + (col + 1)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row + 2, col + 1);
        }
    }
    
    if (row > 1 && col < 7){
        target = state[(row - 2) * 8 + (col + 1)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row - 2, col + 1);
        }
    }
}

void Chess::generateKingMoves(const char *state, std::vector<BitMove>& moves, int row, int col, int colorInt) {
    
    char target;
    
    // straight up and down
    if (row >= 0){
        target = state[(row + 1) * 8 + col];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row + 1, col);
        }
    }

    if (row <= 7){
        target = state[(row - 1) * 8 + col];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row - 1, col);
        }
    }

    // side and diagonal moves
    if (col > 0){
        target = state[row * 8 + (col - 1)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row , col - 1);
        }
        if(row < 7){
        target = state[(row + 1) * 8 + (col + 1)];
            if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
                addMove(state, moves, row, col, row + 1, col + 1);
            }
        }
        if(row > 0){
        target = state[(row - 1) * 8 + (col + 1)];
            if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
                addMove(state, moves, row, col, row - 1, col + 1);
            }
        }
    }

    if (col < 7){
        target = state[row * 8 + (col + 1)];
        if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
            addMove(state, moves, row, col, row , col + 1);
        }
        if(row < 7){
        target = state[(row + 1) * 8 + (col - 1)];
            if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
                addMove(state, moves, row, col, row + 1, col - 1);
            }
        }
        if(row > 0){
        target = state[(row - 1) * 8 + (col - 1)];
            if (target == '0' || (target != '0' && ((colorInt == 1 && islower(target)) || (colorInt == -1 && isupper(target))))){
                addMove(state, moves, row, col, row - 1, col - 1);
            }
        }
    }
}



void Chess::addMove(const char *state, vector<BitMove>& moves, int fromRow, int fromCol, int toRow, int toCol) {
    if(toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8) {
        int fromColor = stateColor(fromCol, fromRow);
        int toColor = stateColor(toCol, toRow);

        if(fromColor != toColor) {
            moves.emplace_back(fromRow * 8 + fromCol, toRow * 8 + toCol, Knight);
        }
    }
}

vector<BitMove> Chess::generateMoves(const char* state, char color) {
    vector<BitMove> moves;
    moves.reserve(40);

    int colorInt = (color == 'W')  ? 1 : -1;

    for(int i = 0; i < 64; i++) {
        int row = i / 8;
        int col = i % 8;
        char piece = state[i];
        int pieceColor = (piece == '0') ? 0 : (piece < 'a') ? 1 : -1;
        if(pieceColor == colorInt){
            if(toupper(piece) == 'P') generatePawnMoves(state, moves, row, col, colorInt);
            if(toupper(piece) == 'N') generateKnightMoves(state, moves, row, col, colorInt);
            if(toupper(piece) == 'K') generateKingMoves(state, moves, row, col, colorInt);
        }
    }
    return moves;
}
