## Noah Billedo CMPM 123 - Windows PC

## Chess Implementation

### Chess.cpp
- Contains implementation of the 8x8 game board and basic movement without legality checks. This version of the chess implementation uses offsets and bitboards. The board indexing is organized from the bottom left (0, 0) to the top right (7, 7). Movement is implement via generateMove() functions that utilize boundary checks and bitboard operations which then emplace the moves into the moves vector generated in canBitMoveFromTo() by calling the generateMoves() function. All bitboards are empty on declaration and the bits are toggled to represent the game state based on the FEN string on setup. The engine currently utilizes a bitboard for each piece of each color, as well as an occupancy bitBoard and an enemyPiece bitboards that stays within the scope of the function it is created in.

### Chess.h
- Contains function definitions for the implementation. This implementation does not use a template function like my iterative one.

### Most Recent Requested Screenshots
## Movement Vector Screenshot
![Vector Movement Screenshot](VectorScreenshotMovementOne.png)
## Board Screenshot
![Most Recent Board Screenshot](BoardScreenshotMovementOne.png)
