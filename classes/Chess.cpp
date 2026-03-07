#include "Chess.h"
#include <limits>
#include <cmath>
#include <string>
#include <ctype.h>
#include <cctype>
#include "Bitboard.h"

using namespace std;

// ==============================================================
// global variables
// ==============================================================

int rowOffset = 0;
int colOffset = 0;
int bishopOffsets[4][2] = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };
int rookOffsets[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
static int pieceValue[128] = {0};

void initPieceValues() {
    pieceValue['P'] = 100; pieceValue['N'] = 320; pieceValue['B'] = 320; pieceValue['R'] = 500; pieceValue['Q'] = 900; pieceValue['K'] = 20000;
    pieceValue['p'] = -100; pieceValue['n'] = -320; pieceValue['b'] = -320; pieceValue['r'] = -500; pieceValue['q'] = -900; pieceValue['k'] = -20000;
};

// ==============================================================
// constructors, destructors
// ==============================================================

Chess::Chess()
{
    initPieceValues();
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
    string spritePath = string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
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
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    //FENtoBoard("8/8/8/3rrrrr/8/4K3/8/8");
    startGame();
}

void Chess::FENtoBoard(const string& fen) {
    // fen to board starts drawing from the top left now so I can have white pieces on the bottom without flipping
    // the fen string or reading it in reverse.
    int col = 0;
    int row = 7; 
    for (char piece : fen) {
        switch (piece) {
            case 'r': case 'R':
                pieceSetFEN(col, row, piece, Rook);
                col++;
                break;
            case 'n': case 'N':
                pieceSetFEN(col, row, piece, Knight);
                col++;
                break;
            case 'b': case 'B':
                pieceSetFEN(col, row, piece, Bishop);
                col++;
                break;
            case 'q': case 'Q':
                pieceSetFEN(col, row, piece, Queen);
                col++;
                break;
            case 'k': case 'K':
                pieceSetFEN(col, row, piece, King);
                col++;
                break;
            case 'p': case 'P':
                pieceSetFEN(col, row, piece, Pawn);
                col++;
                break;
            case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8':
                col += (piece - '0');
                break;
            case '/':
                row--;
                col = 0;
                break;

            default:
                return;
        }
    }
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

string Chess::initialStateString() { return stateString(); }

string Chess::stateString() {
    string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const string &s) {
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

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &from, BitHolder &to) {
    ChessSquare* fromSquare = dynamic_cast<ChessSquare*>(&from);
    ChessSquare* toSquare = dynamic_cast<ChessSquare*>(&to);

    if(!fromSquare || !toSquare) return false;
 
    string state = stateString();
    char color = (bit.gameTag() <  128) ? 'W' : 'B';
    auto moves = generateMoves(state.c_str(), color);

    int fromIndex = fromSquare->getRow() * 8 + fromSquare->getColumn();
    int toIndex = toSquare->getRow() * 8 + toSquare->getColumn();

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

// all move generation functionality basically operates the same, checking for a valid initial position before passing piece-specific offsets into
// the calculate moves function template that utilizes callable parameter to apply the offsets
void Chess::generatePawnMoves(const char *state, vector<BitMove>& moves, int row, int col, int colorInt) {
    rowOffset = (colorInt == 1) ? 1 : -1; colOffset = 0;
    int startRow = (colorInt == 1) ? 1 : 6; 
    // forward moves
    if(row > 0 && row < 7){
            calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset) * 8 + (col + colOffset)]; });
        if(row == startRow){
            rowOffset = rowOffset * 2;
            calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset) * 8 + (col + colOffset)]; });
        }
    }
    // captures
    if (col > 0) {
        rowOffset = (colorInt == 1 ) ? 1 : -1; colOffset = -1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset) * 8 + (col + colOffset)]; });
    }
    if (col < 7) {
        rowOffset = (colorInt == 1 ) ? 1 : -1; colOffset = 1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset) * 8 + (col + colOffset)]; });
        }
    }

void Chess::generateKnightMoves(const char *state, vector<BitMove>& moves, int row, int col, int colorInt) {
    if (row > 0 && col < 6){
        rowOffset = -1; colOffset = 2;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row < 7 && col < 6){
        rowOffset = 1; colOffset = 2;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row < 7 && col > 1){
        rowOffset = 1; colOffset = -2;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row > 0 && col > 1){
        rowOffset = -1; colOffset = -2;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row < 6 && col > 0){
        rowOffset = 2; colOffset = -1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row > 1 && col > 0){
        rowOffset = -2; colOffset = -1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row < 6 && col < 7){
        rowOffset = 2; colOffset = 1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row > 1 && col < 7){
        rowOffset = -2; colOffset = 1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
}

void Chess::generateKingMoves(const char *state, vector<BitMove>& moves, int row, int col, int colorInt) {
    // for my own reference, a row offset of 1 is a single space up towards the board. A col offset of 1 is one move to the right
    if (row >= 0){
        rowOffset = 1; colOffset = 0;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row <= 7){
        rowOffset = -1; colOffset = 0;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (col > 0){
        rowOffset = 0; colOffset = -1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (col < 7){
        rowOffset = 0; colOffset = 1;
        calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    // diagonal moves
    if (row >= 0 && row < 7 && col < 7){ 
            rowOffset = 1; colOffset = 1;
            calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row > 0 && row <= 7 && col < 7){ 
            rowOffset = -1; colOffset = 1;
            calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row >= 0 && row < 7 && col > 0){ 
            rowOffset = 1; colOffset = -1;
            calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
    if (row > 0 && row <= 7 && col > 0){ 
            rowOffset = -1; colOffset = -1;
            calculateMoves(state, moves, row, rowOffset, col, colOffset, colorInt, [&]{ return state[(row + rowOffset ) * 8 + (col + colOffset)]; });
    }
}

void Chess::generateBishopAndRookMoves(const char* state, vector<BitMove>& moves, int row, int col, int colorInt, int offsets[][2], int numOffsets) {
    for(int i = 0; i < numOffsets; i++) {
        rowOffset = offsets[i][0];
        colOffset = offsets[i][1];
        int depth = 1;
        while(true) {
            
            int rowIncrementValue = rowOffset * depth;
            int colIncrementValue = colOffset * depth;
            if(row + rowIncrementValue < 0 || row + rowIncrementValue  > 7 || col + colIncrementValue < 0 || col + colIncrementValue > 7) break;
            calculateMoves(state, moves, row, rowIncrementValue, col, colIncrementValue, colorInt, [&]{ return state[ (row + rowIncrementValue) * 8 + (col + colIncrementValue)];});

            char target = state[(row + rowIncrementValue) * 8 + (col + colIncrementValue)];
            if (target != '0') break;
            depth++;
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
            if(toupper(static_cast<unsigned char>(piece)) == 'P') generatePawnMoves(state, moves, row, col, colorInt);
            if(toupper(static_cast<unsigned char>(piece)) == 'N') generateKnightMoves(state, moves, row, col, colorInt);
            if(toupper(static_cast<unsigned char>(piece)) == 'K') generateKingMoves(state, moves, row, col, colorInt);
            if(toupper(static_cast<unsigned char>(piece)) == 'R') {
                generateBishopAndRookMoves(state, moves, row, col, colorInt, rookOffsets, 4);
            }
            if(toupper(static_cast<unsigned char>(piece)) == 'B') {
                generateBishopAndRookMoves(state, moves, row, col, colorInt, bishopOffsets, 4);
            }
            if(toupper(static_cast<unsigned char>(piece)) == 'Q') {
                generateBishopAndRookMoves(state, moves, row, col, colorInt, rookOffsets, 4);
                generateBishopAndRookMoves(state, moves, row, col, colorInt, bishopOffsets, 4);
            }
        }
    }
    return moves;
}

// ==================================================
// AI Functions
// ==================================================

void Chess::tryMove(string &state, int from, int to) {
    state[to] = state[from];
    state[from] = '0';
}

void Chess::undoMove(string &state, int from, int to, char capturedPiece) {
    state[from] = state[to];
    state[to] = capturedPiece;
}

int Chess::aiBoardEval(const string &state) {
    // iterate through the state string and add score based on values designated to each piece in pieceValue look up table
    // created in the constructor
    int score = 0;
    for (char piece : state) {
        score += pieceValue[(int)piece];
    }
    return score;
}


bool Chess::aiTestForTerminal(const string &state) {
    bool whiteKing=false, blackKing=false;
    for (char c : state) {
        if(c=='K') whiteKing = true;
        if(c=='k') blackKing=true;
    }
    return !whiteKing || !blackKing;
}

int Chess::negamax(string &state, int depth, int alpha, int beta, int playerColor) {
    if(depth == 0 || aiTestForTerminal(state)) return aiBoardEval(state) * playerColor;

    int bestVal = -99999;

    char colorChar = (playerColor == 1) ? 'W' : 'B';
    auto moves = generateMoves(state.c_str(), colorChar);

    if(moves.empty())
        return aiBoardEval(state) * playerColor;

    // iterate through the moves, try each and recursively call negamax. undo moves and determine best move
    // if alpha beta threshold met, discard
    for(auto &move : moves) {
        char capturedPiece = state[move.to];
        tryMove(state, move.from, move.to);

        int val = -negamax(state, depth-1, -beta, -alpha, -playerColor);

        undoMove(state, move.from, move.to, capturedPiece);

        bestVal = max(bestVal, val);
        alpha = max(alpha, val);

        if(alpha >= beta)
            break;
    }
    return bestVal;
}

void Chess::updateAI() {

    // generate all possible moves to be scored by negamax
    string baseState = stateString();
    auto moves = generateMoves(baseState.c_str(), 'B');
    if(moves.empty()) return;

    int bestScore = -99999;
    BitMove bestMove = moves[0];

    for(auto &move : moves) {
        
        // test moves on the state, perform negamax base call, replace bestMove based on scores
        string state = baseState;

        tryMove(state, move.from, move.to);
        // negamax depth 5. Any higher and it takes forever to think/crashes on this implementation.
        int score = -negamax(state, 5-1, -99999, 99999, 1);

        if(score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    // convert move indices to grid, take the move, end the turn
    ChessSquare* fromSquare = _grid->getSquare(bestMove.from % 8, bestMove.from / 8);
    ChessSquare* toSquare   = _grid->getSquare(bestMove.to % 8, bestMove.to / 8);

    Bit* activePiece = fromSquare->bit();
    toSquare->setBit(activePiece);
    fromSquare->setBit(nullptr);
    activePiece->moveTo(toSquare->getPosition());
    endTurn();

}