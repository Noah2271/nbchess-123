#include "Chess.h"
#include <limits>
#include <cmath>
#include <string>
#include <ctype.h>
#include <cctype>
#include "Bitboard.h"

using namespace std;

// =================================================================
// global variables / Bitboards
// =================================================================

// offsets for sliding moves
int bishopOffsets[4][2] = { {1,1}, {1,-1}, {-1,1}, {-1,-1} };
int rookOffsets[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

// bitboards, initialized as empty since they are filled while processing the FEN string; otherwise the game would only be playable from the default position
BitboardElement occupancyBoard = 0;
BitboardElement bitboards[2][7] = {
    BitboardElement(),BitboardElement(),BitboardElement(),BitboardElement(),BitboardElement(),BitboardElement(), BitboardElement(),
    BitboardElement(),BitboardElement(),BitboardElement(),BitboardElement(),BitboardElement(),BitboardElement(), BitboardElement()
};

// =================================================================
// constructors, destructors
// =================================================================

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

// =================================================================
// piece creation
// draws a 8x8 grid and converts a FEN string read from left->right
// and draws pieces starting at index 0,0 on the bottom left of the
// board to the top right of the board
// * note that the state string reads from the bottom left to 
// top right
// =================================================================

char Chess::pieceNotation(int x, int y) const {
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
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
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // fen to board starts drawing from the top left now so 
    // I can have white pieces on the bottom without flipping
    // the fen string or reading it in reverse
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
    // player information grabbing
    int playerNumber = isupper(FENchar) ? 0 : 1;
    Bit* piece = PieceForPlayer(playerNumber, type);

    // bitboard updating for toggling on pieces of individual bitboards
    int bitIndex = ((row * 8 ) + col);
    bitboards[playerNumber][type] |= (1ULL << bitIndex);
    occupancyBoard |= (1ULL << bitIndex);

    // board setting
    ChessSquare* currSquare = _grid->getSquare(col, row);
    piece->setPosition(currSquare->getPosition());
    currSquare->setBit(piece);
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
    return pieceColor == currentPlayer;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &from, BitHolder &to) {
    ChessSquare* fromSquare = dynamic_cast<ChessSquare*>(&from);
    ChessSquare* toSquare   = dynamic_cast<ChessSquare*>(&to);
    if(!fromSquare || !toSquare) return false;

    char color = (bit.gameTag() < 128) ? 'W' : 'B';
    auto moves = generateMoves(color);

    int fromIndex = fromSquare->getRow() * 8 + fromSquare->getColumn();
    int toIndex   = toSquare->getRow() * 8 + toSquare->getColumn();

    for (auto &move : moves) {
        if (move.from == fromIndex && move.to == toIndex){
            return true;
        }
    }
    return false;
}

// overridden bitMovedFromTo because canBitMoveFromTo 
// since updating bitboards in canBitMoveFromTo results
// in pre-move bitboard updating which leads to all
// sorts of issues
void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    ChessSquare* fromSquare = dynamic_cast<ChessSquare*>(&src);
    ChessSquare* toSquare   = dynamic_cast<ChessSquare*>(&dst);
    if(!fromSquare || !toSquare) return;

    int fromIndex = fromSquare->getRow() * 8 + fromSquare->getColumn();
    int toIndex   = toSquare->getRow() * 8 + toSquare->getColumn();

    int playerIndex = (bit.gameTag() < 128) ? 0 : 1;
    int pieceType   = bit.gameTag() & 7;

    uint64_t playerBoard = bitboards[playerIndex][pieceType].getData();
    uint64_t occupancy    = occupancyBoard.getData();

    // toggle the bit from where the piece was from to 0
    playerBoard &= ~(1ULL << fromIndex);
    occupancy    &= ~(1ULL << fromIndex);

    // write the data back 
    bitboards[playerIndex][pieceType].setData(playerBoard);
    occupancyBoard.setData(occupancy);

    // basically the same logic but for universal capture handling
        if (toSquare->bit()) {
            int enemyIndex = (bit.gameTag() < 128) ? 1 : 0;

            // loop through all types of enemy piece bitboards
            for (int pieceType = 0; pieceType < 7; ++pieceType) {
                uint64_t enemyBoard = bitboards[enemyIndex][pieceType].getData();

                // if enemy has a piece at toIndex/destination, toggles it 
                // off on the respective bitboard
                if (enemyBoard & (1ULL << toIndex)) {
                    enemyBoard &= ~(1ULL << toIndex); 
                    bitboards[enemyIndex][pieceType].setData(enemyBoard);
                    break; 
                }
            }
        }

    // toggle the bit on where the player is going
    playerBoard |= (1ULL << toIndex);
    occupancy    |= (1ULL << toIndex);

    // write the data back again
    bitboards[playerIndex][pieceType].setData(playerBoard);
    occupancyBoard.setData(occupancy);
    printAllBoards();
    Game::bitMovedFromTo(bit, src, dst);
}

// calls all of the generateMoves functions and stores them
// to be processed in canBitMoveFromTo
vector<BitMove> Chess::generateMoves(char color) {
    vector<BitMove> moves;
    moves.reserve(40);

    int playerIndex = (color == 'W')  ? 0 : 1;
    generatePawnMoves(moves, bitboards, occupancyBoard, playerIndex);
    generateKnightMoves(moves, bitboards, occupancyBoard, playerIndex);
    generateKingMoves(moves, bitboards, occupancyBoard, playerIndex);
    generateSlidingMoves(moves, bitboards, occupancyBoard, playerIndex, rookOffsets, 4, Rook);
    generateSlidingMoves(moves, bitboards, occupancyBoard, playerIndex, bishopOffsets, 4, Bishop);
    generateSlidingMoves(moves, bitboards, occupancyBoard, playerIndex, bishopOffsets, 4, Queen);
    generateSlidingMoves(moves, bitboards, occupancyBoard, playerIndex, rookOffsets, 4, Queen);

    return moves;
}


void Chess::generatePawnMoves(vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex) {

    uint64_t pawns = bitboards[playerIndex][Pawn].getData();
    BitboardElement pawnElement(pawns);
    
    // grab enemy information to process capture moves
    int enemyIndex(playerIndex == 0 ? 1 : 0);
    uint64_t enemyPieces = 0;
    for(int pieceType = 0; pieceType < 7; ++pieceType) {
        enemyPieces |= bitboards[enemyIndex][pieceType].getData();
    }

    while (pawns) {

        int from = pawnElement.bitScanForward(pawns);
        
        // get offsets for pawns
        int to = (playerIndex == 0) ? from + 8 : from - 8;
        int fromCol = from % 8;
        int toStart = (playerIndex == 0) ? from + 16 : from - 16;

        uint64_t occupancy = occupancyBoard.getData();

        // single push
        if (from >= 8 && from <= 56 && !(occupancy & (1ULL << to))) {
            moves.emplace_back(from, to, Pawn);
        }
        // double push              
        if (((from >= 8 && from <= 15) || (from >= 48 && from <= 55)) && !(occupancy & (1ULL << (toStart))) ) {
                moves.emplace_back(from, toStart, Pawn);
        }
        // captures
        if (fromCol > 0  && (enemyPieces & (1ULL << (to - 1)))) moves.emplace_back(from, to - 1, Pawn);
        if (fromCol < 7 && (enemyPieces & (1ULL << (to + 1)))) moves.emplace_back(from, to + 1, Pawn);

        pawns &= pawns - 1;
    }
}

void Chess::generateKnightMoves(vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex) {
    
    uint64_t knights = bitboards[playerIndex][Knight].getData();
    BitboardElement knightElement(knights);
    uint64_t occupancy = occupancyBoard.getData();
    int enemyIndex(playerIndex == 0 ? 1 : 0);

    uint64_t enemyPieces = 0;
    
    for(int pieceType = 0; pieceType < 7; ++pieceType) {
        enemyPieces |= bitboards[enemyIndex][pieceType].getData();
    }

    while(knights) {

        int from = knightElement.bitScanForward(knights);
        int fromRow = from / 8;
        int fromCol = from % 8;

        if (fromRow <= 5 && fromCol <= 6 && (!(occupancy & (1ULL << (from  + 17))) || enemyPieces & (1ULL << (from + 17)))) moves.emplace_back(from, from + 17, Knight);
        if (fromRow < 6 && fromCol <= 5 && (!(occupancy & (1ULL << (from  + 10))) || enemyPieces & (1ULL << (from + 10)))) moves.emplace_back(from, from + 10, Knight);
        if (fromRow < 6 && fromCol >= 2 && (!(occupancy & (1ULL << (from  + 6))) || enemyPieces & (1ULL << (from + 6))))  moves.emplace_back(from, from + 6, Knight);
        if (fromRow <= 5 && fromCol >= 1 && (!(occupancy & (1ULL << (from  + 15))) || enemyPieces & (1ULL << (from + 15))))  moves.emplace_back(from, from + 15, Knight);
        if (fromRow >= 1 && fromCol >= 2 && (!(occupancy & (1ULL << (from  - 10))) || enemyPieces & (1ULL << (from - 10))))  moves.emplace_back(from, from - 10, Knight);
        if(fromRow  >= 2 && fromCol >= 1 && (!(occupancy & (1ULL << (from  - 17))) || enemyPieces & (1ULL << (from - 17))))  moves.emplace_back(from, from - 17, Knight);
        if(fromRow >=2 && fromCol <= 6 && (!(occupancy & (1ULL << (from  - 15))) || enemyPieces & (1ULL << (from - 15))))  moves.emplace_back(from, from - 15, Knight);
        if(fromRow >= 1 && fromCol <= 5 && (!(occupancy & (1ULL << (from  - 6))) || enemyPieces & (1ULL << (from - 6))))  moves.emplace_back(from, from - 6, Knight);
        knights &= knights - 1;
    }
}

void Chess::generateKingMoves(vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex) {
    
    uint64_t kings = bitboards[playerIndex][King].getData();
    BitboardElement kingElement(kings);
    uint64_t occupancy = occupancyBoard.getData();
    int enemyIndex(playerIndex == 0 ? 1 : 0);

    uint64_t enemyPieces = 0;
    
    for(int pieceType = 0; pieceType < 7; ++pieceType) {
        enemyPieces |= bitboards[enemyIndex][pieceType].getData();
    }

    while(kings) {

        int from = kingElement.bitScanForward(kings);
        int fromRow = from / 8;
        int fromCol = from % 8;
        
        // straight moves
        if(fromRow >= 1 && (!(occupancy & (1ULL << (from - 8))) || enemyPieces & (1ULL << (from - 8)))) moves.emplace_back(from, from - 8, King);
        if(fromRow <= 6 && (!(occupancy & (1ULL << (from  + 8))) || enemyPieces & (1ULL << (from + 8)))) moves.emplace_back(from, from + 8, King);
        if(fromCol <= 6 && (!(occupancy & (1ULL << (from  + 1))) || enemyPieces & (1ULL << (from + 1)))) moves.emplace_back(from, from + 1, King);
        if(fromCol >= 1 && (!(occupancy & (1ULL << (from  - 1))) || enemyPieces & (1ULL << (from - 1)))) moves.emplace_back(from, from - 1, King);
        // diagonals
        if(fromRow >= 1 && fromCol <= 6 && (!(occupancy & (1ULL << (from - 7))) || enemyPieces & (1ULL << (from - 7)))) moves.emplace_back(from, from - 7, King);
        if(fromRow >= 1 && fromCol >= 1 && (!(occupancy & (1ULL << (from - 9))) || enemyPieces & (1ULL << (from - 9)))) moves.emplace_back(from, from - 9, King);
        if(fromRow <= 6 && fromCol <= 6 && (!(occupancy & (1ULL << (from + 9))) || enemyPieces & (1ULL << (from + 9)))) moves.emplace_back(from, from + 9, King);
        if (fromRow <= 6 && fromCol >= 1 && (!(occupancy & (1ULL << (from + 7))) || enemyPieces & (1ULL << (from + 7)))) moves.emplace_back(from, from + 7, King);
        kings &= kings - 1;
    }
}

void Chess::generateSlidingMoves(vector<BitMove>& moves, BitboardElement bitboards[2][7], const BitboardElement& occupancyBoard, int playerIndex, int offsets[][2], int numOffsets, int pieceType) {

    // gather the bitboard information and create BitboardElement instance
    // for pieceType passed into the function
    uint64_t pieces = bitboards[playerIndex][pieceType].getData();
    BitboardElement pieceElement(pieces);
    uint64_t occupancy = occupancyBoard.getData();
    int enemyIndex = (playerIndex == 0 ? 1 : 0);

    uint64_t enemyPieces = 0;
    for (int p = 0; p < 7; ++p) {
        enemyPieces |= bitboards[enemyIndex][p].getData();
    }

    while(pieces) {

        int from = pieceElement.bitScanForward(pieces);
        int fromRow = from / 8;
        int fromCol = from % 8;

        for(int i = 0; i < numOffsets; i++) {
            int rowDirection = offsets[i][0];
            int columnDirection = offsets[i][1];

            int row = fromRow;
            int col = fromCol;

            // for as long as possible, move in the direction of the offsets
            // from start point
            while (true) {
                row += rowDirection;
                col += columnDirection;

                if( row < 0 || row > 7 || col < 0 || col > 7) break;
                int to = row * 8 + col;

                // if the player's destination is empty or has an enemy piece
                // the move is placed in the vector
                uint64_t mask = (1ULL << to);
                if (!(occupancy & mask) || (enemyPieces & mask)) {
                    moves.emplace_back(from, to, (ChessPiece)pieceType);
                }

                // if the player runs into any blocker, loop breaks
                // and moves are no longer added for specific direction
                if (occupancy & mask) break;
            }
        }
        pieces &= pieces - 1;
    }
}

// ==============================================================
// other functions
// functions that are not implemented yet, or made for the
// purpose of debugging
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

std::string Chess::stateString() {
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s) {
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

void Chess::printAllBoards() {

    cout << "================== NEW NEW NEW NEW NEW BOARDDSSSS " << endl;
    cout << "White Pawn Board" << endl;
    bitboards[0][Pawn].printBitboard();
    cout << "==================" << endl;
    cout << "White Knight Board" << endl;
    bitboards[0][Knight].printBitboard();
    cout << "==================" << endl;
    cout << "White Rook Board" << endl;
    bitboards[0][Rook].printBitboard();
    cout << "==================" << endl;
    cout << "White Bishop Board" << endl;
    bitboards[0][Bishop].printBitboard();
    cout << "==================" << endl;
    cout << "White Queen Board" << endl;
    bitboards[0][Queen].printBitboard();
    cout << "==================" << endl;
    cout << "White King Board" << endl;
    bitboards[0][King].printBitboard();

    cout << "==================" << endl;
    cout << "Black Pawn Board" << endl;
    bitboards[1][Pawn].printBitboard();
    cout << "==================" << endl;
    cout << "Black Knight Board" << endl;
    bitboards[1][Knight].printBitboard();
    cout << "==================" << endl;
    cout << "Black Rook Board" << endl;
    bitboards[1][Rook].printBitboard();
    cout << "==================" << endl;
    cout << "Black Bishop Board" << endl;
    bitboards[1][Bishop].printBitboard();
    cout << "==================" << endl;
    cout << "Black Queen Board" << endl;
    bitboards[1][Queen].printBitboard();
    cout << "==================" << endl;
    cout << "Black King Board" << endl;
    bitboards[1][King].printBitboard();
    
    cout << "==================" << endl; 
    cout << "Occupancy Board" << endl;
    occupancyBoard.printBitboard();
}